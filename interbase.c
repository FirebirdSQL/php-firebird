/*
   +----------------------------------------------------------------------+
   | PHP Version 7, 8                                                     |
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jouni Ahto <jouni.ahto@exdec.fi>                            |
   |          Andrew Avdeev <andy@simgts.mv.ru>                           |
   |          Ard Biesheuvel <a.k.biesheuvel@its.tudelft.nl>              |
   |          Martin Koeditz <martin.koeditz@it-syn.de>                   |
   |          Martins Lazdans <marrtins@dqdp.net>                         |
   |          others                                                      |
   +----------------------------------------------------------------------+
   | You'll find history on Github                                        |
   | https://github.com/FirebirdSQL/php-firebird/commits/master           |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

#if HAVE_IBASE

#include "php_ini.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/md5.h"
#include "php_interbase.h"
#include "php_ibase_includes.h"
#include "SAPI.h"
#include <stdbool.h>
#include <time.h>
#include "firebird_utils.h"

#define ROLLBACK    0
#define COMMIT      1
#define RETAIN      2

#define CHECK_LINK(link) { if (link==NULL) { php_error_docref(NULL, E_WARNING, "A link to the server could not be established"); RETURN_FALSE; } }

ZEND_DECLARE_MODULE_GLOBALS(ibase)
static PHP_GINIT_FUNCTION(ibase);
static PHP_GSHUTDOWN_FUNCTION(ibase);

/* Generated from interbase.stub.php by build/gen_stub.php */
#if PHP_VERSION_ID >= 80000
# include "interbase_arginfo.h"
#else
# include "interbase_legacy_arginfo.h"
#endif

/* {{{ extension definition structures */
zend_module_entry ibase_module_entry = {
	STANDARD_MODULE_HEADER,
	"interbase",
	ext_functions,
	PHP_MINIT(ibase),
	PHP_MSHUTDOWN(ibase),
	NULL,
	PHP_RSHUTDOWN(ibase),
	PHP_MINFO(ibase),
	PHP_INTERBASE_VER_STR,
	PHP_MODULE_GLOBALS(ibase),
	PHP_GINIT(ibase),
	PHP_GSHUTDOWN(ibase),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_INTERBASE
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(ibase)
#endif

/* True globals, no need for thread safety */
int le_link, le_plink, le_trans;

/* }}} */

/* error handling ---------------------------- */

/* {{{ proto string ibase_errmsg(void)
   Return error message */
PHP_FUNCTION(ibase_errmsg)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (IBG(sql_code) != 0) {
		RETURN_STRING(IBG(errmsg));
	}

	RETURN_FALSE;
}
/* }}} */

/* The runtime client on the same scale FB_API_VER uses: major * 10 + minor. A client with
 * no OO API cannot report itself and leaves both at -1, which comes out negative here --
 * below every real version, so comparisons still answer correctly without guessing at
 * which old version it actually is. */
static zend_long fbp_client_api_ver(void)
{
	return (zend_long)IBG(client_major_version) * 10 + IBG(client_minor_version);
}

/* {{{ proto int ibase_get_client_version(void)
   Return client version on the FB_API_VER scale: 2.5 is 25, 5.0 is 50 */
PHP_FUNCTION(ibase_get_client_version)
{
	RETURN_LONG(fbp_client_api_ver());
}
/* }}} */

/* {{{ proto int ibase_get_client_major_version(void)
   Return client major version */
PHP_FUNCTION(ibase_get_client_major_version)
{
	RETURN_LONG(IBG(client_major_version));
}
/* }}} */

/* {{{ proto int ibase_get_client_minor_version(void)
   Return client minor version */
PHP_FUNCTION(ibase_get_client_minor_version)
{
	RETURN_LONG(IBG(client_minor_version));
}
/* }}} */

/* {{{ proto int ibase_errcode(void)
   Return error code */
PHP_FUNCTION(ibase_errcode)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (IBG(sql_code) != 0) {
		RETURN_LONG(IBG(sql_code));
	}
	RETURN_FALSE;
}
/* }}} */

/* print interbase error and save it for ibase_errmsg() */
void _php_ibase_error(void) /* {{{ */
{
	char *s = IBG(errmsg);
	const ISC_STATUS *statusp = IB_STATUS;

	IBG(sql_code) = isc_sqlcode(IB_STATUS);

	while ((s - IBG(errmsg)) < MAX_ERRMSG && fb_interpret(s, (unsigned int)(MAX_ERRMSG - strlen(IBG(errmsg)) - 1), &statusp)) {
		strcat(IBG(errmsg), " ");
		s = IBG(errmsg) + strlen(IBG(errmsg));
	}

	php_error_docref(NULL, E_WARNING, "%s", IBG(errmsg));
}
/* }}} */

/* print php interbase module error and save it for ibase_errmsg() */
void _php_ibase_module_error(const char *msg, ...) /* {{{ */
{
	va_list ap;

	va_start(ap, msg);

	/* vsnprintf NUL terminates the buf and writes at most n-1 chars+NUL */
	vsnprintf(IBG(errmsg), MAX_ERRMSG, msg, ap);
	va_end(ap);

	IBG(sql_code) = -999; /* no SQL error */

	php_error_docref(NULL, E_WARNING, "%s", IBG(errmsg));
}
/* }}} */

/* {{{ internal macros, functions and structures */
typedef struct {
	isc_db_handle *db_ptr;
	zend_long tpb_len;
	char *tpb_ptr;
} ISC_TEB;

/* }}} */

/* Fill ib_link and trans with the correct database link and transaction. */
void _php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAMETERS, /* {{{ */
	zval *link_id, ibase_db_link **ib_link, ibase_trans **trans)
{
	IBDEBUG("Transaction or database link?");
	if (Z_RES_P(link_id)->type == le_trans) {
		/* Transaction resource: make sure it refers to one link only, then
		   fetch it; database link is stored in ib_trans->db_link[]. */
		IBDEBUG("Type is le_trans");
		*trans = (ibase_trans *)zend_fetch_resource_ex(link_id, LE_TRANS, le_trans);
		if ((*trans)->link_cnt > 1) {
			_php_ibase_module_error("Link id is ambiguous: transaction spans multiple connections."
				);
			return;
		}
		*ib_link = (*trans)->db_link[0];
		return;
	}
	IBDEBUG("Type is le_[p]link or id not found");
	/* Database link resource, use default transaction. */
	*trans = NULL;
	*ib_link = (ibase_db_link *)zend_fetch_resource2_ex(link_id, LE_LINK, le_link, le_plink);
}
/* }}} */

/* destructors ---------------------- */

static void _php_ibase_commit_link(ibase_db_link *link) /* {{{ */
{
	unsigned short i = 0, j;
	ibase_tr_list *l;
	ibase_event *e;
	IBDEBUG("Checking transactions to close...");

	for (l = link->tr_list; l != NULL; ++i) {
		ibase_tr_list *p = l;
		if (p->trans != 0) {
			if (i == 0) {
				if (p->trans->handle != 0) {
					IBDEBUG("Committing default transaction...");
					if (isc_commit_transaction(IB_STATUS, &p->trans->handle)) {
						_php_ibase_error();
					}
				}
				efree(p->trans); /* default transaction is not a registered resource: clean up */
			} else {
				if (p->trans->handle != 0) {
					/* non-default trans might have been rolled back by other call of this dtor */
					IBDEBUG("Rolling back other transactions...");
					if (isc_rollback_transaction(IB_STATUS, &p->trans->handle)) {
						_php_ibase_error();
					}
				}
				/* set this link pointer to NULL in the transaction */
				for (j = 0; j < p->trans->link_cnt; ++j) {
					if (p->trans->db_link[j] == link) {
						p->trans->db_link[j] = NULL;
						break;
					}
				}
			}
		}
		l = l->next;
		efree(p);
	}
	link->tr_list = NULL;

	for (e = link->event_head; e; e = e->event_next) {
		_php_ibase_free_event(e);
		e->link = NULL;
	}
}

/* }}} */

static void php_ibase_commit_link_rsrc(zend_resource *rsrc) /* {{{ */
{
	ibase_db_link *link = (ibase_db_link *) rsrc->ptr;

	_php_ibase_commit_link(link);
}
/* }}} */

static void _php_ibase_close_link(zend_resource *rsrc) /* {{{ */
{
	ibase_db_link *link = (ibase_db_link *) rsrc->ptr;

	_php_ibase_commit_link(link);
	if (link->handle != 0) {
		IBDEBUG("Closing normal link...");
		isc_detach_database(IB_STATUS, &link->handle);
	}
	IBG(num_links)--;
	efree(link);
}
/* }}} */

static void _php_ibase_close_plink(zend_resource *rsrc) /* {{{ */
{
	ibase_db_link *link = (ibase_db_link *) rsrc->ptr;

	_php_ibase_commit_link(link);
	IBDEBUG("Closing permanent link...");
	if (link->handle != 0) {
		isc_detach_database(IB_STATUS, &link->handle);
	}
	IBG(num_persistent)--;
	IBG(num_links)--;
	free(link);
}
/* }}} */

static void _php_ibase_free_trans(zend_resource *rsrc) /* {{{ */
{
	ibase_trans *trans = (ibase_trans *)rsrc->ptr;
	unsigned short i;

	IBDEBUG("Cleaning up transaction resource...");
	if (trans->handle != 0) {
		IBDEBUG("Rolling back unhandled transaction...");
		if (isc_rollback_transaction(IB_STATUS, &trans->handle)) {
			_php_ibase_error();
		}
	}

	/* now remove this transaction from all the connection-transaction lists */
	for (i = 0; i < trans->link_cnt; ++i) {
		if (trans->db_link[i] != NULL) {
			ibase_tr_list **l;
			for (l = &trans->db_link[i]->tr_list; *l != NULL; l = &(*l)->next) {
				if ( (*l)->trans == trans) {
					ibase_tr_list *p = *l;
					*l = p->next;
					efree(p);
					break;
				}
			}
		}
	}
	efree(trans);
}
/* }}} */

/* TODO this function should be part of either Zend or PHP API */
static PHP_INI_DISP(php_ibase_password_displayer_cb)
{

	if ((type == PHP_INI_DISPLAY_ORIG && ini_entry->orig_value)
			|| (type == PHP_INI_DISPLAY_ACTIVE && ini_entry->value)) {
		PUTS("********");
	} else if (!sapi_module.phpinfo_as_text) {
		PUTS("<i>no value</i>");
	} else {
		PUTS("no value");
	}
}

#define PUTS_TP(str) do {       \
	if(has_puts) {              \
		PUTS(" | ");            \
	}                           \
	PUTS(str);                  \
	has_puts = 1;               \
} while (0)

static PHP_INI_DISP(php_ibase_trans_displayer)
{
	int has_puts = 0;
	char *value;

	if (type == ZEND_INI_DISPLAY_ORIG && ini_entry->modified) {
		value = ZSTR_VAL(ini_entry->orig_value);
	} else if (ini_entry->value) {
		value = ZSTR_VAL(ini_entry->value);
	} else {
		value = NULL;
	}

	if (value) {
		zend_long trans_argl = atol(value);

		if (trans_argl != PHP_IBASE_DEFAULT) {
			/* access mode */
			if (PHP_IBASE_READ == (trans_argl & PHP_IBASE_READ)) {
				PUTS_TP("IBASE_READ");
			} else if (PHP_IBASE_WRITE == (trans_argl & PHP_IBASE_WRITE)) {
				PUTS_TP("IBASE_WRITE");
			}

			/* isolation level */
			if (PHP_IBASE_COMMITTED == (trans_argl & PHP_IBASE_COMMITTED)) {
				PUTS_TP("IBASE_COMMITTED");
				if (PHP_IBASE_REC_VERSION == (trans_argl & PHP_IBASE_REC_VERSION)) {
					PUTS_TP("IBASE_REC_VERSION");
				} else if (PHP_IBASE_REC_NO_VERSION == (trans_argl & PHP_IBASE_REC_NO_VERSION)) {
					PUTS_TP("IBASE_REC_NO_VERSION");
				}
			} else if (PHP_IBASE_CONSISTENCY == (trans_argl & PHP_IBASE_CONSISTENCY)) {
				PUTS_TP("IBASE_CONSISTENCY");
			} else if (PHP_IBASE_CONCURRENCY == (trans_argl & PHP_IBASE_CONCURRENCY)) {
				PUTS_TP("IBASE_CONCURRENCY");
			}

			/* lock resolution */
			if (PHP_IBASE_NOWAIT == (trans_argl & PHP_IBASE_NOWAIT)) {
				PUTS_TP("IBASE_NOWAIT");
			} else if (PHP_IBASE_WAIT == (trans_argl & PHP_IBASE_WAIT)) {
				PUTS_TP("IBASE_WAIT");
				if (PHP_IBASE_LOCK_TIMEOUT == (trans_argl & PHP_IBASE_LOCK_TIMEOUT)) {
					PUTS_TP("IBASE_LOCK_TIMEOUT");
				}
			}
		} else {
			PUTS_TP("IBASE_DEFAULT");
		}
	}
}

/* {{{ startup, shutdown and info functions */
PHP_INI_BEGIN()
	PHP_INI_ENTRY_EX("ibase.allow_persistent", "1", PHP_INI_SYSTEM, NULL, zend_ini_boolean_displayer_cb)
	/* On: ibase_connect() with arguments it has seen before returns the existing link
	 * instead of attaching again, as it always has. Off: every call gets its own. */
	PHP_INI_ENTRY_EX("ibase.reuse_connections", "1", PHP_INI_ALL, NULL, zend_ini_boolean_displayer_cb)
	PHP_INI_ENTRY_EX("ibase.max_persistent", "-1", PHP_INI_SYSTEM, NULL, display_link_numbers)
	PHP_INI_ENTRY_EX("ibase.max_links", "-1", PHP_INI_SYSTEM, NULL, display_link_numbers)
	PHP_INI_ENTRY("ibase.default_db", NULL, PHP_INI_SYSTEM, NULL)
	PHP_INI_ENTRY("ibase.default_user", NULL, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY_EX("ibase.default_password", NULL, PHP_INI_ALL, NULL, php_ibase_password_displayer_cb)
	PHP_INI_ENTRY("ibase.default_charset", NULL, PHP_INI_ALL, NULL)
	STD_PHP_INI_ENTRY("ibase.timestampformat", IB_DEF_DATE_FMT " " IB_DEF_TIME_FMT, PHP_INI_ALL, OnUpdateString, timestampformat, zend_ibase_globals, ibase_globals)
	STD_PHP_INI_ENTRY("ibase.timestampformat_tz", IB_DEF_DATE_FMT " " IB_DEF_TIME_FMT " e", PHP_INI_ALL, OnUpdateString, timestampformat_tz, zend_ibase_globals, ibase_globals)
	STD_PHP_INI_ENTRY("ibase.dateformat", IB_DEF_DATE_FMT, PHP_INI_ALL, OnUpdateString, dateformat, zend_ibase_globals, ibase_globals)
	STD_PHP_INI_ENTRY("ibase.timeformat", IB_DEF_TIME_FMT, PHP_INI_ALL, OnUpdateString, timeformat, zend_ibase_globals, ibase_globals)
	STD_PHP_INI_ENTRY("ibase.timeformat_tz", IB_DEF_TIME_FMT " e", PHP_INI_ALL, OnUpdateString, timeformat_tz, zend_ibase_globals, ibase_globals)
	STD_PHP_INI_ENTRY_EX("ibase.default_trans_params", "0", PHP_INI_ALL, OnUpdateLongGEZero, default_trans_params, zend_ibase_globals, ibase_globals, php_ibase_trans_displayer)
	STD_PHP_INI_ENTRY_EX("ibase.default_lock_timeout", "0", PHP_INI_ALL, OnUpdateLongGEZero, default_lock_timeout, zend_ibase_globals, ibase_globals, display_link_numbers)
	/* -1 = every legacy behaviour, including any added later; 0 = none. See php_ibase_includes.h */
	STD_PHP_INI_ENTRY("ibase.legacy_flags", "-1", PHP_INI_ALL, OnUpdateLong, legacy_flags, zend_ibase_globals, ibase_globals)
PHP_INI_END()

#ifdef __GNUC__
void* _php_ibase_get_fbclient_symbol(const char* sym)
{
	return dlsym(RTLD_DEFAULT, sym);
}
#elif defined(PHP_WIN32)
void* _php_ibase_get_fbclient_symbol(const char* sym)
{
	HMODULE l = GetModuleHandle("fbclient");

	if (!l && !(l = GetModuleHandle("gds32"))) {
		return NULL;
	}

	return GetProcAddress(l, sym);
}
#else
	static_assert(false, "TODO: implement dynamic symbol name lookup for your platform");
#endif

static PHP_GINIT_FUNCTION(ibase)
{
#if defined(COMPILE_DL_INTERBASE) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	ibase_globals->num_persistent = ibase_globals->num_links = 0;
	ibase_globals->sql_code = *ibase_globals->errmsg = 0;
	ibase_globals->default_link = NULL;
	ibase_globals->get_master_interface = _php_ibase_get_fbclient_symbol("fb_get_master_interface");
	ibase_globals->get_statement_interface = _php_ibase_get_fbclient_symbol("fb_get_statement_interface");

#if FB_API_VER >= 30
	if (ibase_globals->get_master_interface) {
		ibase_globals->master_instance = ((fb_get_master_interface_t)(ibase_globals->get_master_interface))();
		ibase_globals->client_version = fbu_get_client_version(ibase_globals->master_instance);
		ibase_globals->client_major_version = (ibase_globals->client_version >> 8) & 0xFF;
		ibase_globals->client_minor_version = ibase_globals->client_version & 0xFF;
		ibase_globals->status_instance = fbu_new_status(ibase_globals->master_instance);
	} else {
#endif
		ibase_globals->master_instance = NULL;
		ibase_globals->status_instance = NULL;
		ibase_globals->client_version = -1;
		ibase_globals->client_major_version = -1;
		ibase_globals->client_minor_version = -1;
#if FB_API_VER >= 30
	}
#endif
}

static PHP_GSHUTDOWN_FUNCTION(ibase)
{
#if FB_API_VER >= 30
	if (ibase_globals->status_instance) {
		fbu_dispose_status(ibase_globals->status_instance);
		ibase_globals->status_instance = NULL;
	}
#endif
}

PHP_MINIT_FUNCTION(ibase)
{
	REGISTER_INI_ENTRIES();

	le_link = zend_register_list_destructors_ex(_php_ibase_close_link, NULL, LE_LINK, module_number);
	le_plink = zend_register_list_destructors_ex(php_ibase_commit_link_rsrc, _php_ibase_close_plink, LE_PLINK, module_number);
	le_trans = zend_register_list_destructors_ex(_php_ibase_free_trans, NULL, LE_TRANS, module_number);

	register_interbase_symbols(module_number);

	/* The lower of FB_API_VER and the loaded client's version; only known at load time */
	REGISTER_LONG_CONSTANT("IBASE_EFFECTIVE_VER",
		FB_API_VER < fbp_client_api_ver() ? FB_API_VER : fbp_client_api_ver(), CONST_PERSISTENT);

	php_ibase_query_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_ibase_blobs_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_ibase_events_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_ibase_service_minit(INIT_FUNC_ARGS_PASSTHRU);

#ifdef ZEND_SIGNALS
	// firebird replaces some signals at runtime, suppress warnings.
	SIGG(check) = 0;
#endif

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(ibase)
{
#ifndef PHP_WIN32
	/**
	 * When the Interbase client API library libgds.so is first loaded, it registers a call to
	 * gds__cleanup() with atexit(), in order to clean up after itself when the process exits.
	 * This means that the library is called at process shutdown, and cannot be unloaded beforehand.
	 * PHP tries to unload modules after every request [dl()'ed modules], and right before the
	 * process shuts down [modules loaded from php.ini]. This results in a segfault for this module.
	 * By NULLing the dlopen() handle in the module entry, Zend omits the call to dlclose(),
	 * ensuring that the module will remain present until the process exits. However, the functions
	 * and classes exported by the module will not be available until the module is 'reloaded'.
	 * When reloaded, dlopen() will return the handle of the already loaded module. The module will
	 * be unloaded automatically when the process exits.
	 */
	zend_module_entry *ibase_entry;
	if ((ibase_entry = zend_hash_str_find_ptr(&module_registry, ibase_module_entry.name,
			strlen(ibase_module_entry.name))) != NULL) {
		ibase_entry->handle = 0;
	}
#endif
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(ibase)
{
	IBG(num_links) = IBG(num_persistent);
	IBG(default_link)= NULL;

	RESET_ERRMSG;

	return SUCCESS;
}

PHP_MINFO_FUNCTION(ibase)
{
	char tmp[64], *s;

	php_info_print_table_start();
	php_info_print_table_row(2, "Firebird/InterBase Support",
#ifdef COMPILE_DL_INTERBASE
		"dynamic");
#else
		"static");
#endif

	php_info_print_table_row(2, "Interbase extension version", PHP_INTERBASE_VER_STR);

#ifdef FB_API_VER
	snprintf( (s = tmp), sizeof(tmp), "Firebird API version %d", FB_API_VER);
#elif (SQLDA_CURRENT_VERSION > 1)
	s =  "Interbase 7.0 and up";
#endif
	php_info_print_table_row(2, "Compile-time Client Library Version", s);

#if defined(__GNUC__) || defined(PHP_WIN32)
	do {
		info_func_t info_func = NULL;
#ifdef __GNUC__
		info_func = (info_func_t)dlsym(RTLD_DEFAULT, "isc_get_client_version");
#else
		HMODULE l = GetModuleHandle("fbclient");

		if (!l && !(l = GetModuleHandle("gds32"))) {
			break;
		}
		info_func = (info_func_t)GetProcAddress(l, "isc_get_client_version");
#endif
		if (info_func) {
			info_func(s = tmp);
		}
		php_info_print_table_row(2, "Run-time Client Library Version", s);
	} while (0);
#endif
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

}
/* }}} */

enum connect_args { DB = 0, USER = 1, PASS = 2, CSET = 3, ROLE = 4, BUF = 0, DLECT = 1, SYNC = 2 };

static char const dpb_args[] = {
	0, isc_dpb_user_name, isc_dpb_password, isc_dpb_lc_ctype, isc_dpb_sql_role_name, 0
};

int _php_ibase_attach_db(char **args, size_t *len, zend_long *largs, isc_db_handle *db) /* {{{ */
{
	short i, dpb_len, buf_len = 257-2;  /* version byte at the front, and a null at the end */
	char dpb_buffer[257] = { isc_dpb_version1, 0 }, *dpb;

	dpb = dpb_buffer + 1;

	for (i = 0; i < sizeof(dpb_args); ++i) {
		if (dpb_args[i] && args[i] && len[i] && buf_len > 0) {
			dpb_len = snprintf(dpb, buf_len, "%c%c%s", dpb_args[i],(unsigned char)len[i],args[i]);
			dpb += dpb_len;
			buf_len -= dpb_len;
		}
	}
	if (largs[BUF] && buf_len > 0) {
		dpb_len = snprintf(dpb, buf_len, "%c\2%c%c", isc_dpb_num_buffers,
			(char)(largs[BUF] >> 8), (char)(largs[BUF] & 0xff));
		dpb += dpb_len;
		buf_len -= dpb_len;
	}
	if (largs[SYNC] && buf_len > 0) {
		dpb_len = snprintf(dpb, buf_len, "%c\1%c", isc_dpb_force_write, largs[SYNC] == isc_spb_prp_wm_sync);
		dpb += dpb_len;
		buf_len -= dpb_len;
	}

#if FB_API_VER >= 40
	if (buf_len > 0) {
		// ibase_query(): Data type unknown
		// If fbclient >= 4 then convert to VARCHAR at server only INT128 and DECFLOAT
		// If we have older client, convert also timezone types
		if(IBG(client_major_version) >= 4) {
			const char compat[] = "INT128 TO VARCHAR;DECFLOAT TO VARCHAR";
			dpb_len = snprintf(dpb, buf_len, "%c%c%s", isc_dpb_set_bind, (unsigned char)(sizeof(compat) - 1), compat);
		} else {
			const char compat[] = "INT128 TO VARCHAR;DECFLOAT TO VARCHAR;TIME ZONE TO LEGACY";
			dpb_len = snprintf(dpb, buf_len, "%c%c%s", isc_dpb_set_bind, (unsigned char)(sizeof(compat) - 1), compat);
		}

		dpb += dpb_len;
		buf_len -= dpb_len;
	}
#endif

	if (buf_len < 0) {
		fbp_fatal("ibase_connect(): DPB buffer overflow (connection parameters exceed internal buffer size)");
	}

	if (isc_attach_database(IB_STATUS, (short)len[DB], args[DB], db, (short)(dpb-dpb_buffer), dpb_buffer)) {
		_php_ibase_error();
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static void _php_ibase_connect(INTERNAL_FUNCTION_PARAMETERS, zend_long persistent) /* {{{ */
{
	char *c, hash[16], *args[] = { NULL, NULL, NULL, NULL, NULL };
	int i;
	size_t len[] = { 0, 0, 0, 0, 0 };
	zend_long largs[] = { 0, 0, 0 };
	int reuse;
	PHP_MD5_CTX hash_context;
	zend_resource new_index_ptr, *le;
	isc_db_handle db_handle = 0;
	ibase_db_link *ib_link;

	RESET_ERRMSG;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|ssssllsl",
			&args[DB], &len[DB], &args[USER], &len[USER], &args[PASS], &len[PASS],
			&args[CSET], &len[CSET], &largs[BUF], &largs[DLECT], &args[ROLE], &len[ROLE],
			&largs[SYNC])) {
		RETURN_FALSE;
	}

	/* restrict to the server/db in the .ini if in safe mode */
	if (!len[DB] && (c = INI_STR("ibase.default_db"))) {
		args[DB] = c;
		len[DB] = strlen(c);
	}
	if (!len[USER] && (c = INI_STR("ibase.default_user"))) {
		args[USER] = c;
		len[USER] = strlen(c);
	}
	if (!len[PASS] && (c = INI_STR("ibase.default_password"))) {
		args[PASS] = c;
		len[PASS] = strlen(c);
	}
	if (!len[CSET] && (c = INI_STR("ibase.default_charset"))) {
		args[CSET] = c;
		len[CSET] = strlen(c);
	}

	/* don't want usernames and passwords floating around */
	PHP_MD5Init(&hash_context);
	for (i = 0; i < sizeof(args)/sizeof(char*); ++i) {
		PHP_MD5Update(&hash_context,args[i],len[i]);
	}
	for (i = 0; i < sizeof(largs)/sizeof(zend_long); ++i) {
		PHP_MD5Update(&hash_context,(char*)&largs[i],sizeof(zend_long));
	}
	PHP_MD5Final((unsigned char*)hash, &hash_context);

	reuse = INI_BOOL("ibase.reuse_connections");

	/*
	 * A pooled connection is a shared one by definition, so there is nothing to hand out
	 * privately: fall back to a normal link, as the max_persistent path below already does.
	 */
	if (!reuse) {
		persistent = 0;
	}

	/* try to reuse a connection */
	if (reuse && (le = zend_hash_str_find_ptr(&EG(regular_list), hash, sizeof(hash)-1)) != NULL) {
		zend_resource *xlink;

		if (le->type != le_index_ptr) {
			RETURN_FALSE;
		}

		xlink = (zend_resource*) le->ptr;
		if ((!persistent && xlink->type == le_link) || xlink->type == le_plink) {
			if (IBG(default_link) != xlink) {
				GC_ADDREF(xlink);
				if (IBG(default_link)) {
					zend_list_delete(IBG(default_link));
				}
				IBG(default_link) = xlink;
			}
			GC_ADDREF(xlink);
			RETURN_RES(xlink);
		} else {
			zend_hash_str_del(&EG(regular_list), hash, sizeof(hash)-1);
		}
	}

	/* ... or a persistent one */
	do {
		zend_long l;
		static char info[] = { isc_info_base_level, isc_info_end };
		char result[8];
		ISC_STATUS status[20];

		if (reuse && (le = zend_hash_str_find_ptr(&EG(persistent_list), hash, sizeof(hash)-1)) != NULL) {
			if (le->type != le_plink) {
				RETURN_FALSE;
			}
			/* check if connection has timed out */
			ib_link = (ibase_db_link *) le->ptr;
			if (!isc_database_info(status, &ib_link->handle, sizeof(info), info, sizeof(result), result)) {
				RETVAL_RES(zend_register_resource(ib_link, le_plink));
				break;
			}
			zend_hash_str_del(&EG(persistent_list), hash, sizeof(hash)-1);
		}

		/* no link found, so we have to open one */

		if ((l = INI_INT("ibase.max_links")) != -1 && IBG(num_links) >= l) {
			_php_ibase_module_error("Too many open links (%ld)", IBG(num_links));
			RETURN_FALSE;
		}

		/* create the ib_link */
		if (FAILURE == _php_ibase_attach_db(args, len, largs, &db_handle)) {
			RETURN_FALSE;
		}

		/* use non-persistent if allowed number of persistent links is exceeded */
		if (!persistent || ((l = INI_INT("ibase.max_persistent")) != -1 && IBG(num_persistent) >= l)) {
			ib_link = (ibase_db_link *) emalloc(sizeof(ibase_db_link));
			RETVAL_RES(zend_register_resource(ib_link, le_link));
		} else {
			ib_link = (ibase_db_link *) malloc(sizeof(ibase_db_link));
			if (!ib_link) {
				RETURN_FALSE;
			}

			/* hash it up */
			if (zend_register_persistent_resource(hash, sizeof(hash)-1, ib_link, le_plink) == NULL) {
				free(ib_link);
				RETURN_FALSE;
			}
			RETVAL_RES(zend_register_resource(ib_link, le_plink));
			++IBG(num_persistent);
		}
		ib_link->handle = db_handle;
		ib_link->dialect = largs[DLECT] ? (unsigned short)largs[DLECT] : SQL_DIALECT_CURRENT;
		ib_link->tr_list = NULL;
		ib_link->event_head = NULL;

		++IBG(num_links);
	} while (0);

	/* add it to the hash, so a later call with the same arguments finds it */
	if (reuse) {
		new_index_ptr.ptr = (void *) Z_RES_P(return_value);
		new_index_ptr.type = le_index_ptr;
		zend_hash_str_update_mem(&EG(regular_list), hash, sizeof(hash)-1,
				(void *) &new_index_ptr, sizeof(zend_resource));
	}
	if (IBG(default_link)) {
		zend_list_delete(IBG(default_link));
	}
	IBG(default_link) = Z_RES_P(return_value);
	Z_TRY_ADDREF_P(return_value);
	Z_TRY_ADDREF_P(return_value);
}
/* }}} */

/* {{{ proto resource ibase_connect([string database [, string username [, string password [, string charset [, int buffers [, int dialect [, string role]]]]]]])
   Open a connection to an InterBase database */
PHP_FUNCTION(ibase_connect)
{
	_php_ibase_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto resource ibase_pconnect([string database [, string username [, string password [, string charset [, int buffers [, int dialect [, string role]]]]]]])
   Open a persistent connection to an InterBase database */
PHP_FUNCTION(ibase_pconnect)
{
	_php_ibase_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INI_INT("ibase.allow_persistent"));
}
/* }}} */

/* {{{ proto bool ibase_close([resource link_identifier])
   Close an InterBase connection */
PHP_FUNCTION(ibase_close)
{
	zval *link_arg = NULL;
	zend_resource *link_res;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r!", &link_arg) == FAILURE) {
		return;
	}

	if (link_arg == NULL) {
		link_res = IBG(default_link);
		CHECK_LINK(link_res);
		IBG(default_link) = NULL;
	} else {
		link_res = Z_RES_P(link_arg);
	}

	if (!zend_fetch_resource2(link_res, LE_LINK, le_link, le_plink)) {
		RETURN_FALSE;
	}

	/* we have at least 3 additional references to this resource ??? */
	// Keep this code for now. In case we decide to put it under a some kind of
	// legacy flag
	// if (GC_REFCOUNT(link_res) < 4) {
	// 	zend_list_close(link_res);
	// } else {
	// 	zend_list_delete(link_res);
	// }

	zend_list_close(link_res);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ibase_drop_db([resource link_identifier])
   Drop an InterBase database */
PHP_FUNCTION(ibase_drop_db)
{
	zval *link_arg = NULL;
	ibase_db_link *ib_link;
	ibase_tr_list *l;
	zend_resource *link_res;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r!", &link_arg) == FAILURE) {
		return;
	}

	if (link_arg == NULL) {
		link_res = IBG(default_link);
		CHECK_LINK(link_res);
		IBG(default_link) = NULL;
	} else {
		link_res = Z_RES_P(link_arg);
	}

	ib_link = (ibase_db_link *)zend_fetch_resource2(link_res, LE_LINK, le_link, le_plink);

	if (!ib_link) {
		RETURN_FALSE;
	}

	if (isc_drop_database(IB_STATUS, &ib_link->handle)) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	/* isc_drop_database() doesn't invalidate the transaction handles */
	for (l = ib_link->tr_list; l != NULL; l = l->next) {
		if (l->trans != NULL) l->trans->handle = 0;
	}

	zend_list_delete(link_res);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource ibase_trans([int trans_args [, resource link_identifier [, ... ], int trans_args [, resource link_identifier [, ... ]] [, ...]]])
   Start a transaction over one or several databases */

#define TPB_MAX_SIZE (8*sizeof(char))

void _php_ibase_populate_trans(zend_long trans_argl, zend_long trans_timeout, char *last_tpb, unsigned short *len) /* {{{ */
{
	unsigned short tpb_len = 0;
	if (trans_argl != PHP_IBASE_DEFAULT) {
		last_tpb[tpb_len++] = isc_tpb_version3;

		/* access mode */
		if (PHP_IBASE_READ == (trans_argl & PHP_IBASE_READ)) {
			last_tpb[tpb_len++] = isc_tpb_read;
		} else if (PHP_IBASE_WRITE == (trans_argl & PHP_IBASE_WRITE)) {
			last_tpb[tpb_len++] = isc_tpb_write;
		}

		/* isolation level */
		if (PHP_IBASE_COMMITTED == (trans_argl & PHP_IBASE_COMMITTED)) {
			last_tpb[tpb_len++] = isc_tpb_read_committed;
			if (PHP_IBASE_REC_VERSION == (trans_argl & PHP_IBASE_REC_VERSION)) {
				last_tpb[tpb_len++] = isc_tpb_rec_version;
			} else if (PHP_IBASE_REC_NO_VERSION == (trans_argl & PHP_IBASE_REC_NO_VERSION)) {
				last_tpb[tpb_len++] = isc_tpb_no_rec_version;
			}
		} else if (PHP_IBASE_CONSISTENCY == (trans_argl & PHP_IBASE_CONSISTENCY)) {
			last_tpb[tpb_len++] = isc_tpb_consistency;
		} else if (PHP_IBASE_CONCURRENCY == (trans_argl & PHP_IBASE_CONCURRENCY)) {
			last_tpb[tpb_len++] = isc_tpb_concurrency;
		}

		/* lock resolution */
		if (PHP_IBASE_NOWAIT == (trans_argl & PHP_IBASE_NOWAIT)) {
			last_tpb[tpb_len++] = isc_tpb_nowait;
		} else if (PHP_IBASE_WAIT == (trans_argl & PHP_IBASE_WAIT)) {
			last_tpb[tpb_len++] = isc_tpb_wait;
			if (PHP_IBASE_LOCK_TIMEOUT == (trans_argl & PHP_IBASE_LOCK_TIMEOUT)) {
				if (trans_timeout <= 0 || trans_timeout > 0x7FFF) {
					php_error_docref(NULL, E_WARNING, "Invalid timeout parameter");
				} else {
					last_tpb[tpb_len++] = isc_tpb_lock_timeout;
					last_tpb[tpb_len++] = sizeof(ISC_SHORT);
					last_tpb[tpb_len++] = (char)trans_timeout;
					last_tpb[tpb_len++] = (char)(trans_timeout >> 8);
				}
			}
		}
	}
	*len = tpb_len;
}
/* }}} */

PHP_FUNCTION(ibase_trans)
{
	unsigned short i, link_cnt = 0, tpb_len = 0;
	int argn = ZEND_NUM_ARGS();
	char last_tpb[TPB_MAX_SIZE];
	ibase_db_link **ib_link = NULL;
	ibase_trans *ib_trans;
	isc_tr_handle tr_handle = 0;
	ISC_STATUS result;

	RESET_ERRMSG;

	/* (1+argn) is an upper bound for the number of links this trans connects to */
	ib_link = (ibase_db_link **) safe_emalloc(sizeof(ibase_db_link *),1+argn,0);

	if (argn > 0) {
		zend_long trans_argl = 0;
		zend_long trans_timeout = 0;
		char *tpb;
		ISC_TEB *teb;
		zval *args = NULL;

		if (zend_parse_parameters(argn, "+", &args, &argn) == FAILURE) {
			efree(ib_link);
			RETURN_FALSE;
		}

		teb = (ISC_TEB *) safe_emalloc(sizeof(ISC_TEB),argn,0);
		tpb = (char *) safe_emalloc(TPB_MAX_SIZE,argn,0);

		/* enumerate all the arguments: assume every non-resource argument
		   specifies modifiers for the link ids that follow it */
		for (i = 0; i < argn; ++i) {

			if (Z_TYPE(args[i]) == IS_RESOURCE) {

				if ((ib_link[link_cnt] = (ibase_db_link *)zend_fetch_resource2_ex(&args[i], LE_LINK, le_link, le_plink)) == NULL) {
					efree(teb);
					efree(tpb);
					efree(ib_link);
					RETURN_FALSE;
				}

				/* copy the most recent modifier string into tbp[] */
				memcpy(&tpb[TPB_MAX_SIZE * link_cnt], last_tpb, TPB_MAX_SIZE);

				/* add a database handle to the TEB with the most recently specified set of modifiers */
				teb[link_cnt].db_ptr = &ib_link[link_cnt]->handle;
				teb[link_cnt].tpb_len = tpb_len;
				teb[link_cnt].tpb_ptr = &tpb[TPB_MAX_SIZE * link_cnt];

				++link_cnt;

			} else {

				tpb_len = 0;

				convert_to_long_ex(&args[i]);
				trans_argl = Z_LVAL(args[i]);
				if (trans_argl != PHP_IBASE_DEFAULT) {
					// Skip conflicting parameters
					if (PHP_IBASE_NOWAIT != (trans_argl & PHP_IBASE_NOWAIT) && PHP_IBASE_WAIT == (trans_argl & PHP_IBASE_WAIT)) {
						if (PHP_IBASE_LOCK_TIMEOUT == (trans_argl & PHP_IBASE_LOCK_TIMEOUT)) {
							if((i + 1 < argn) && (Z_TYPE(args[i + 1]) == IS_LONG)){
								i++;
								convert_to_long_ex(&args[i]);
								trans_timeout = Z_LVAL(args[i]);
							} else {
								php_error_docref(NULL, E_WARNING, "IBASE_LOCK_TIMEOUT expects next argument to be timeout value");
							}
						}
					}
					_php_ibase_populate_trans(trans_argl, trans_timeout, last_tpb, &tpb_len);
				}
			}
		}

		if (link_cnt > 0) {
			result = isc_start_multiple(IB_STATUS, &tr_handle, link_cnt, teb);
		}

		efree(tpb);
		efree(teb);
	}

	if (link_cnt == 0) {
		link_cnt = 1;
		if ((ib_link[0] = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink)) == NULL) {
			efree(ib_link);
			RETURN_FALSE;
		}
		result = isc_start_transaction(IB_STATUS, &tr_handle, 1, &ib_link[0]->handle, tpb_len, last_tpb);
	}

	/* start the transaction */
	if (result) {
		_php_ibase_error();
		efree(ib_link);
		RETURN_FALSE;
	}

	/* register the transaction in our own data structures */
	ib_trans = (ibase_trans *) safe_emalloc(link_cnt-1, sizeof(ibase_db_link *), sizeof(ibase_trans));
	ib_trans->handle = tr_handle;
	ib_trans->link_cnt = link_cnt;
	ib_trans->affected_rows = 0;
	for (i = 0; i < link_cnt; ++i) {
		ibase_tr_list **l;
		ib_trans->db_link[i] = ib_link[i];

		/* the first item in the connection-transaction list is reserved for the default transaction */
		if (ib_link[i]->tr_list == NULL) {
			ib_link[i]->tr_list = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
			ib_link[i]->tr_list->trans = NULL;
			ib_link[i]->tr_list->next = NULL;
		}

		/* link the transaction into the connection-transaction list */
		for (l = &ib_link[i]->tr_list; *l != NULL; l = &(*l)->next);
		*l = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
		(*l)->trans = ib_trans;
		(*l)->next = NULL;
	}
	efree(ib_link);
	RETVAL_RES(zend_register_resource(ib_trans, le_trans));
	Z_TRY_ADDREF_P(return_value);
}
/* }}} */

int _php_ibase_def_trans(ibase_db_link *ib_link, ibase_trans **trans) /* {{{ */
{
	if (ib_link == NULL) {
		php_error_docref(NULL, E_WARNING, "Invalid database link");
		return FAILURE;
	}

	/* the first item in the connection-transaction list is reserved for the default transaction */
	if (ib_link->tr_list == NULL) {
		ib_link->tr_list = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
		ib_link->tr_list->trans = NULL;
		ib_link->tr_list->next = NULL;
	}

	if (*trans == NULL) {
		ibase_trans *tr = ib_link->tr_list->trans;

		if (tr == NULL) {
			tr = (ibase_trans *) emalloc(sizeof(ibase_trans));
			tr->handle = 0;
			tr->link_cnt = 1;
			tr->affected_rows = 0;
			tr->db_link[0] = ib_link;
			ib_link->tr_list->trans = tr;
		}
		if (tr->handle == 0) {
			ISC_STATUS result;
			zend_long trans_argl = IBG(default_trans_params);

			if(trans_argl == PHP_IBASE_DEFAULT){
				result = isc_start_transaction(IB_STATUS, &tr->handle, 1, &ib_link->handle, 0, NULL);
			} else {
				zend_long trans_timeout = IBG(default_lock_timeout);
				char last_tpb[TPB_MAX_SIZE];
				unsigned short tpb_len = 0;
				_php_ibase_populate_trans(trans_argl, trans_timeout, last_tpb, &tpb_len);
				result = isc_start_transaction(IB_STATUS, &tr->handle, 1, &ib_link->handle, tpb_len, last_tpb);
			}

			if (result) {
				_php_ibase_error();
				return FAILURE;
			}
		}
		*trans = tr;
	}
	return SUCCESS;
}
/* }}} */

static void _php_ibase_trans_end(INTERNAL_FUNCTION_PARAMETERS, int commit) /* {{{ */
{
	ibase_trans *trans = NULL;
	int res_id = 0;
	ISC_STATUS result;
	ibase_db_link *ib_link;
	zval *arg = NULL;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r!", &arg) == FAILURE) {
		return;
	}

	if (arg == NULL) {
		ib_link = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink);
		if (ib_link == NULL) {
			RETURN_FALSE;
		}
		if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
			/* this link doesn't have a default transaction */
			_php_ibase_module_error("Default link has no default transaction");
			RETURN_FALSE;
		}
		trans = ib_link->tr_list->trans;
	} else {
		/* one id was passed, could be db or trans id */
		if (Z_RES_P(arg)->type == le_trans) {
			trans = (ibase_trans *)zend_fetch_resource_ex(arg, LE_TRANS, le_trans);
			res_id = Z_RES_P(arg)->handle;
		} else {
			ib_link = (ibase_db_link *)zend_fetch_resource2_ex(arg, LE_LINK, le_link, le_plink);
			if (ib_link == NULL) {
				RETURN_FALSE;
			}

			if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
				/* this link doesn't have a default transaction */
				_php_ibase_module_error("Link has no default transaction");
				RETURN_FALSE;
			}
			trans = ib_link->tr_list->trans;
		}
	}

	switch (commit) {
		default: /* == case ROLLBACK: */
			result = isc_rollback_transaction(IB_STATUS, &trans->handle);
			break;
		case COMMIT:
			result = isc_commit_transaction(IB_STATUS, &trans->handle);
			break;
		case (ROLLBACK | RETAIN):
			result = isc_rollback_retaining(IB_STATUS, &trans->handle);
			break;
		case (COMMIT | RETAIN):
			result = isc_commit_retaining(IB_STATUS, &trans->handle);
			break;
	}

	if (result) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	/* Don't try to destroy implicitly opened transaction from list... */
	if ((commit & RETAIN) == 0 && res_id != 0) {
		zend_list_delete(Z_RES_P(arg));
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ibase_commit( resource link_identifier )
   Commit transaction */
PHP_FUNCTION(ibase_commit)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT);
}
/* }}} */

/* {{{ proto bool ibase_rollback( resource link_identifier )
   Rollback transaction */
PHP_FUNCTION(ibase_rollback)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK);
}
/* }}} */

/* {{{ proto bool ibase_commit_ret( resource link_identifier )
   Commit transaction and retain the transaction context */
PHP_FUNCTION(ibase_commit_ret)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT | RETAIN);
}
/* }}} */

/* {{{ proto bool ibase_rollback_ret( resource link_identifier )
   Rollback transaction and retain the transaction context */
PHP_FUNCTION(ibase_rollback_ret)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK | RETAIN);
}
/* }}} */

/* {{{ proto int ibase_gen_id(string generator [, int increment [, resource link_identifier ]])
   Increments the named generator and returns its new value */
PHP_FUNCTION(ibase_gen_id)
{
	zval *link = NULL;
	char query[128], *generator;
	size_t gen_len;
	zend_long inc = 1;
	ibase_db_link *ib_link;
	ibase_trans *trans = NULL;
	XSQLDA out_sqlda;
	ISC_INT64 result;

	RESET_ERRMSG;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s|lr!", &generator, &gen_len,
			&inc, &link)) {
		RETURN_FALSE;
	}

	if (gen_len > 31) {
		php_error_docref(NULL, E_WARNING, "Invalid generator name");
		RETURN_FALSE;
	}

	PHP_IBASE_LINK_TRANS(link, ib_link, trans);

	snprintf(query, sizeof(query), "SELECT GEN_ID(%s,%ld) FROM rdb$database", generator, inc);

	/* allocate a minimal descriptor area */
	out_sqlda.sqln = out_sqlda.sqld = 1;
	out_sqlda.version = SQLDA_CURRENT_VERSION;

	/* allocate the field for the result */
	out_sqlda.sqlvar[0].sqltype = SQL_INT64;
	out_sqlda.sqlvar[0].sqlscale = 0;
	out_sqlda.sqlvar[0].sqllen = sizeof(result);
	out_sqlda.sqlvar[0].sqldata = (void*) &result;

	/* execute the query */
	if (isc_dsql_exec_immed2(IB_STATUS, &ib_link->handle, &trans->handle, 0, query,
			SQL_DIALECT_CURRENT, NULL, &out_sqlda)) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	/* don't return the generator value as a string unless it doesn't fit in a long */
#if SIZEOF_ZEND_LONG < 8
	if (result < ZEND_LONG_MIN || result > ZEND_LONG_MAX) {
		char *res;
		int l;

		l = spprintf(&res, 0, "%" LL_MASK "d", result);
		RETURN_STRINGL(res, l);
	}
#endif
	RETURN_LONG((zend_long)result);
}

#if PHP_DEBUG
void fbp_dump_buffer(int len, const unsigned char *buffer)
{
	int i;
	for (i = 0; i < len; i++) {
		if(buffer[i] < 32 || buffer[i] > 126)
			php_printf("0x%02x ", buffer[i]);
		else
			php_printf(" [%c] ", buffer[i]);
		if(i % 16 == 15)php_printf("\n");
	}
	if(i > 0)php_printf("\n");
}

void fbp_dump_buffer_raw(int len, const unsigned char *buffer)
{
	int i;
	for (i = 0; i < len; i++) {
		php_printf("%c", buffer[i]);
	}
}
#endif

void fbp_error_ex(long level, const char *msg, ...)
{
	va_list ap;
	char buf[1024] = {0};

	va_start(ap, msg);

	/* vsnprintf NUL terminates the buf and writes at most n-1 chars+NUL */
	vsnprintf(buf, sizeof(buf), msg, ap);
	va_end(ap);

	// IBG(sql_code) = -999; /* no SQL error */

	php_error(level, "%s", buf);
}

/* }}} */

#endif /* HAVE_IBASE */
