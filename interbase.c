/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2001 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.02 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_02.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jouni Ahto <jouni.ahto@exdec.fi>                            |
   |          Andrew Avdeev <andy@rsc.mv.ru>                              |
   +----------------------------------------------------------------------+
 */

/* $Id$ */


/* TODO: Arrays, roles?
A lot... */

/*
	Changes:
		1999-09-21:	Ivo Panacek <ivop@regionet.cz>
			- added COMPILE_DL section
			- more verbose php_info_ibase function
			  mostly stolen from pgsql.c for now
		1999-10-05:	Ivo Panacek <ivop@regionet.cz>
			- safe rinit/rfinish: check for NULL so
			  rfinish could be called repeatedly
			  emalloc & co. replaced with malloc & co.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_standard.h"
#include "php_globals.h"
#include "php_interbase.h"

#if HAVE_IBASE
#include <ibase.h>
#include <time.h>
#include "ext/standard/fsock.h"
#include "ext/standard/info.h"

#ifdef SQL_INT64
#include <math.h>
#endif

#ifndef SQL_DIALECT_CURRENT
#define SQL_DIALECT_CURRENT 1
#endif

/*
#define IBDEBUG(a) php_printf("::: %s (%d)\n", a, __LINE__);
*/
#define IBDEBUG(a)

#define SAFE_STRING(s) ((s)?(s):"")

/* {{{ extension definition structures */
function_entry ibase_functions[] = {
	PHP_FE(ibase_connect, NULL)
	PHP_FE(ibase_pconnect, NULL)
	PHP_FE(ibase_close, NULL)
	PHP_FE(ibase_query, NULL)
	PHP_FE(ibase_fetch_row, NULL)
	PHP_FE(ibase_fetch_object, NULL)
	PHP_FE(ibase_free_result, NULL)
	PHP_FE(ibase_prepare, NULL)
	PHP_FE(ibase_execute, NULL)
	PHP_FE(ibase_free_query, NULL)
	PHP_FE(ibase_timefmt, NULL)

	PHP_FE(ibase_num_fields, NULL)
	PHP_FE(ibase_field_info, NULL)

	PHP_FE(ibase_trans, NULL)
	PHP_FE(ibase_commit, NULL)
	PHP_FE(ibase_rollback, NULL)

	PHP_FE(ibase_blob_info, NULL)
	PHP_FE(ibase_blob_create, NULL)
	PHP_FE(ibase_blob_add, NULL)
	PHP_FE(ibase_blob_cancel, NULL)
	PHP_FE(ibase_blob_close, NULL)
	PHP_FE(ibase_blob_open, NULL)
	PHP_FE(ibase_blob_get, NULL)
	PHP_FE(ibase_blob_echo, NULL)
	PHP_FE(ibase_blob_import, NULL)
	PHP_FE(ibase_errmsg, NULL)
	{NULL, NULL, NULL}
};

zend_module_entry ibase_module_entry =
{
	"interbase",
	ibase_functions,
	PHP_MINIT(ibase),
	PHP_MSHUTDOWN(ibase),
	PHP_RINIT(ibase),
	PHP_RSHUTDOWN(ibase),
	PHP_MINFO(ibase),
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_INTERBASE
ZEND_GET_MODULE(ibase)

#define DL_MALLOC(size) malloc(size)
#define DL_STRDUP(str) strdup(str)
#define DL_FREE(ptr) free(ptr)

#else

#define DL_MALLOC(size) emalloc(size)
#define DL_STRDUP(str) estrdup(str)
#define DL_FREE(ptr) efree(ptr)

#endif

/* True globals, no need for thread safety */
static int le_blob, le_link, le_plink, le_result, le_query, le_trans;

ZEND_DECLARE_MODULE_GLOBALS(ibase)

/* }}} */


/* {{{ internal macros, functions and structures */

#define IB_STATUS (IBG(status))

/* Fill ib_link and trans_n with the correct database link and transaction slot number. */

static void get_link_trans(INTERNAL_FUNCTION_PARAMETERS, zval **link_id, ibase_db_link **ib_link, int *trans_n, int *trans_id) {
	int type;
	void *resource = NULL;
	ibase_tr_link *ib_trans;
	IBLS_FETCH();

	IBDEBUG("Transaction or database link?");
	if ((resource = zend_list_find(Z_LVAL_PP(link_id), &type))) {
		IBDEBUG("Found in list");
		if (type == le_trans) {
			/* Transaction resource. Fetch it, database link resource
			   is stored in ib_trans->link_rsrc. */
			IBDEBUG("Type is le_trans");
			*trans_id = (Z_LVAL_PP(link_id));
			ZEND_FETCH_RESOURCE(ib_trans, ibase_tr_link *, link_id, -1, "InterBase transaction", le_trans);
			*trans_n = ib_trans->trans_num;
			ZEND_FETCH_RESOURCE2(resource, ibase_db_link *, link_id, ib_trans->link_rsrc, "InterBase link", le_link, le_plink);
		} else {
			IBDEBUG("Type is le_[p]link");
			/* Database link resource, use default transaction (=0). */
			*trans_n = 0;
			ZEND_FETCH_RESOURCE2(resource, ibase_db_link *, link_id, -1, "InterBase link", le_link, le_plink);
		}
	}
	*ib_link = resource;
}
	
#define RESET_ERRMSG { IBG(errmsg)[0] = '\0';}

#define TEST_ERRMSG ( IBG(errmsg)[0] != '\0')

/* sql variables union
   used for convert and binding input variables
*/
typedef struct {
	union {
		short sval;
		float fval;
		ISC_QUAD qval;
#ifdef ISC_TIMESTAMP
		ISC_TIMESTAMP tsval;
		ISC_DATE dtval;
		ISC_TIME tmval;
#endif
	} val;
	short sqlind;
} BIND_BUF;


/* get blob identifier from argument
   on empty unset argument ib_blob set to NULL
*/
#define GET_BLOB_ID_ARG(blob_arg, ib_blob)\
{\
	if (blob_arg->type == IS_STRING && blob_arg->value.str.len == 0) {\
		ib_blob = NULL;\
	} else if (blob_arg->type != IS_STRING\
			 || blob_arg->value.str.len != sizeof(ibase_blob_handle)\
			 || ((ibase_blob_handle *)(blob_arg->value.str.val))->bl_handle != 0){\
		_php_ibase_module_error("invalid blob id");\
		RETURN_FALSE;\
	} else {\
		ib_blob = (ibase_blob_handle *)blob_arg->value.str.val;\
	}\
}


/* get blob handle from argument
   note: blob already open when handle active
*/
#define GET_BLOB_HANDLE_ARG(blob_arg, blob_ptr) \
{ \
	int type; \
	convert_to_long(blob_arg); \
	blob_ptr = (ibase_blob_handle *) zend_list_find(blob_arg->value.lval, &type); \
	if (type!=le_blob) { \
		_php_ibase_module_error("%d is not blob handle",blob_arg->value.lval); \
		RETURN_FALSE; \
	} \
}

/* blob information struct */
typedef struct {
	ISC_LONG  max_segment;		/* Length of longest segment */
	ISC_LONG  num_segments;		/* Total number of segments */
	ISC_LONG  total_length;		/* Total length of blob */
	int		  bl_stream;		/* blob is stream ? */
} IBASE_BLOBINFO;

/* }}} */

/* error handling ---------------------------- */

/* {{{ proto string ibase_errmsg(void) 
   Return error message */
PHP_FUNCTION(ibase_errmsg)
{
	IBLS_FETCH();
	
	if (IBG(errmsg[0])) {
		RETURN_STRING(IBG(errmsg),1);
	}
	RETURN_FALSE;
}
/* }}} */


/* {{{ _php_ibase_error() 
    print interbase error and save it for ibase_errmsg() */
static void _php_ibase_error(void)
{
	char *s;
	ISC_STATUS *statusp;
	IBLS_FETCH();

	s = IBG(errmsg);
	statusp = IB_STATUS;
	while ((s - IBG(errmsg)) < MAX_ERRMSG - (IBASE_MSGSIZE + 2) && isc_interprete(s, &statusp)) {
		strcat(IBG(errmsg), " ");
		s = IBG(errmsg) + strlen(IBG(errmsg));
	}
	php_error(E_WARNING, "InterBase: %s", IBG(errmsg));
}
/* }}} */


/* {{{ _php_ibase_module_error()
	print php interbase module error and save it for ibase_errmsg() */
static void _php_ibase_module_error(char *msg, ...)
{
	va_list ap;
	int len;
	IBLS_FETCH();

	va_start(ap, msg);
	len = vsnprintf(IBG(errmsg), MAX_ERRMSG - 1, msg, ap);
	va_end(ap);
	IBG(errmsg[len]) = '\0';
	
	php_error(E_WARNING, "InterBase module: %s", IBG(errmsg));
}
/* }}} */


/* destructors ---------------------- */


/* {{{ _php_ibase_free_xsqlda() */
/* not actual destructor ... */
static void _php_ibase_free_xsqlda(XSQLDA *sqlda)
{
	int i;
	XSQLVAR *var;

	IBDEBUG("Free XSQLDA?");
	if (sqlda) {
		IBDEBUG("Freeing XSQLDA...");
		var = sqlda->sqlvar;
		for (i = 0; i < sqlda->sqld; i++, var++) {
			efree(var->sqldata);
			if(var->sqlind)
				efree(var->sqlind);
		}
		efree(sqlda);
	}
}
/* }}} */


/* {{{ _php_ibase_commit_link() */
static void _php_ibase_commit_link(ibase_db_link *link)
{
	int i;
	IBLS_FETCH();

	IBDEBUG("Checking transactions to close...");
	if (link->trans[0] != NULL) { /* commit default */
		IBDEBUG("Committing default transaction...");
		if (isc_commit_transaction(IB_STATUS, &link->trans[0])) {
			_php_ibase_error();
		}
		link->trans[0] = NULL;
	}
	for (i = 1; i < IBASE_TRANS_ON_LINK; i++) {
		if (link->trans[i] != NULL) {
			IBDEBUG("Rolling back other transactions...");
			if (isc_rollback_transaction(IB_STATUS, &link->trans[i])) {
				_php_ibase_error();
			}
			link->trans[i] = NULL;
		}
	}
}
/* }}} */

static void php_ibase_commit_link_rsrc(zend_rsrc_list_entry *rsrc)
{
	ibase_db_link *link = (ibase_db_link *)rsrc->ptr;
	_php_ibase_commit_link(link);
}

/* {{{ _php_ibase_close_link() */
static void _php_ibase_close_link(zend_rsrc_list_entry *rsrc)
{
	ibase_db_link *link = (ibase_db_link *)rsrc->ptr;
	IBLS_FETCH();

	_php_ibase_commit_link(link);
	IBDEBUG("Closing normal link...");
	isc_detach_database(IB_STATUS, &link->link);
	IBG(num_links)--;
	efree(link);
}
/* }}} */


/* {{{ _php_ibase_close_plink() */
static void _php_ibase_close_plink(zend_rsrc_list_entry *rsrc)
{
	ibase_db_link *link = (ibase_db_link *)rsrc->ptr;
	IBLS_FETCH();

	_php_ibase_commit_link(link);
	IBDEBUG("Closing permanent link...");
	isc_detach_database(IB_STATUS, &link->link);
	IBG(num_persistent)--;
	IBG(num_links)--;
	free(link);
}
/* }}} */


/* {{{ _php_ibase_free_result() */
static void _php_ibase_free_result(zend_rsrc_list_entry *rsrc)
{
	ibase_result *ib_result = (ibase_result *)rsrc->ptr;
	IBLS_FETCH();

	IBDEBUG("Freeing result...");
	if (ib_result){
		_php_ibase_free_xsqlda(ib_result->out_sqlda);
		if (ib_result->drop_stmt && ib_result->stmt) {
			IBDEBUG("Dropping statement handle (free_result)...");
			if (isc_dsql_free_statement(IB_STATUS, &ib_result->stmt, DSQL_drop)) {
				_php_ibase_error();
			}
		} else {
			/* Shouldn't be here unless query was select and had parameter
			   placeholders, in which case ibase_execute handles this???
			   (Testing seems to confirm the decision was a right one.)
			*/
			IBDEBUG("Closing statement handle...");
			/*
			if (isc_dsql_free_statement(IB_STATUS, &ib_result->stmt, DSQL_close)) {
				_php_ibase_error();
			}
			*/
		}
		if (ib_result->out_array) {
			efree(ib_result->out_array);
		}
		efree(ib_result);
	}
}
/* }}} */


/* {{{ _php_ibase_free_query() */
static void _php_ibase_free_query(ibase_query *ib_query)
{
	IBLS_FETCH();

	IBDEBUG("Freeing query...");
	if (ib_query) {
		if (ib_query->in_sqlda) {
			efree(ib_query->in_sqlda);
		}
		if (ib_query->out_sqlda) {
			efree(ib_query->out_sqlda);
		}
		if (ib_query->stmt) {
			IBDEBUG("Dropping statement handle (free_query)...");
			if (isc_dsql_free_statement(IB_STATUS, &ib_query->stmt, DSQL_drop)){
				_php_ibase_error();
			}
		}
		if (ib_query->in_array) {
			efree(ib_query->in_array);
		}
		if (ib_query->out_array) {
			efree(ib_query->out_array);
		}
		efree(ib_query);
	}
}
/* }}} */

static void php_ibase_free_query_rsrc(zend_rsrc_list_entry *rsrc)
{
	ibase_query *query = (ibase_query *)rsrc->ptr;
	_php_ibase_free_query(query);
}

/* {{{ _php_ibase_free_blob()	*/
static void _php_ibase_free_blob(zend_rsrc_list_entry *rsrc)
{
	ibase_blob_handle *ib_blob = (ibase_blob_handle *)rsrc->ptr;
	IBLS_FETCH();

	if (ib_blob->bl_handle != NULL) { /* blob open*/
		if (isc_cancel_blob(IB_STATUS, &ib_blob->bl_handle)) {
			_php_ibase_error();
		}
	}
	efree(ib_blob);
}
/* }}} */


/* {{{ _php_ibase_free_trans()	*/
static void _php_ibase_free_trans(zend_rsrc_list_entry *rsrc)
{
	ibase_tr_link *ib_trans = (ibase_tr_link *)rsrc->ptr;
	ibase_db_link *ib_link;
	IBLS_FETCH();

	ib_link = (ibase_db_link *) zend_fetch_resource(NULL, ib_trans->link_rsrc, "InterBase link", NULL, 2, le_link, le_plink);
	
	if (ib_link) {
		if (ib_link->trans[ib_trans->trans_num] != NULL) {
			IBDEBUG("Rolling back unhandled transaction...");
			if (isc_rollback_transaction(IB_STATUS, &ib_link->trans[ib_trans->trans_num])) {
				_php_ibase_error();
			}
			ib_link->trans[ib_trans->trans_num] = NULL;
		}
	}
	
	efree(ib_trans);
}
/* }}} */


/* {{{ startup, shutdown and info functions */
PHP_INI_BEGIN()
	 STD_PHP_INI_BOOLEAN("ibase.allow_persistent", "1", PHP_INI_SYSTEM, OnUpdateInt, allow_persistent, zend_ibase_globals, ibase_globals)
	 STD_PHP_INI_ENTRY_EX("ibase.max_persistent", "-1", PHP_INI_SYSTEM, OnUpdateInt, max_persistent, zend_ibase_globals, ibase_globals, display_link_numbers)
	 STD_PHP_INI_ENTRY_EX("ibase.max_links", "-1", PHP_INI_SYSTEM, OnUpdateInt, max_links, zend_ibase_globals, ibase_globals, display_link_numbers)
	 STD_PHP_INI_ENTRY("ibase.default_user", NULL, PHP_INI_ALL, OnUpdateString, default_user, zend_ibase_globals, ibase_globals)
	 STD_PHP_INI_ENTRY("ibase.default_password", NULL, PHP_INI_ALL, OnUpdateString, default_password, zend_ibase_globals, ibase_globals)
	 STD_PHP_INI_ENTRY("ibase.timestampformat", "%m/%d/%Y %H:%M:%S", PHP_INI_ALL, OnUpdateString, cfg_timestampformat, zend_ibase_globals, ibase_globals)
	 STD_PHP_INI_ENTRY("ibase.dateformat", "%m/%d/%Y", PHP_INI_ALL, OnUpdateString, cfg_dateformat, zend_ibase_globals, ibase_globals)
	 STD_PHP_INI_ENTRY("ibase.timeformat", "%H:%M:%S", PHP_INI_ALL, OnUpdateString, cfg_timeformat, zend_ibase_globals, ibase_globals)
PHP_INI_END()

static void php_ibase_init_globals(IBLS_D)
{
	IBG(timestampformat) = NULL;
	IBG(dateformat) = NULL;
	IBG(timeformat) = NULL;
	IBG(errmsg) = NULL;
	IBG(num_persistent) = 0;
}

PHP_MINIT_FUNCTION(ibase)
{
	IBLS_FETCH();

	ZEND_INIT_MODULE_GLOBALS(ibase, php_ibase_init_globals, NULL);

	REGISTER_INI_ENTRIES();

	le_result = zend_register_list_destructors_ex(_php_ibase_free_result, NULL, "interbase result", module_number);
	le_query = zend_register_list_destructors_ex(php_ibase_free_query_rsrc, NULL, "interbase query", module_number);
	le_blob = zend_register_list_destructors_ex(_php_ibase_free_blob, NULL, "interbase blob", module_number);
	le_link = zend_register_list_destructors_ex(_php_ibase_close_link, NULL, "interbase link", module_number);
	le_plink = zend_register_list_destructors_ex(php_ibase_commit_link_rsrc, _php_ibase_close_plink, "interbase link persistent", module_number);
	le_trans = zend_register_list_destructors_ex(_php_ibase_free_trans, NULL, "interbase transaction", module_number);

	REGISTER_LONG_CONSTANT("IBASE_DEFAULT", PHP_IBASE_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_TEXT", PHP_IBASE_TEXT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_UNIXTIME", PHP_IBASE_UNIXTIME, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_READ", PHP_IBASE_READ, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_COMMITTED", PHP_IBASE_COMMITTED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_CONSISTENCY", PHP_IBASE_CONSISTENCY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_NOWAIT", PHP_IBASE_NOWAIT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_TIMESTAMP", PHP_IBASE_TIMESTAMP, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_DATE", PHP_IBASE_DATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("IBASE_TIME", PHP_IBASE_TIME, CONST_PERSISTENT);
	
	return SUCCESS;
}


PHP_RINIT_FUNCTION(ibase)
{
	IBLS_FETCH();

	IBG(default_link)= -1;
	IBG(num_links) = IBG(num_persistent);

	if (IBG(timestampformat))
        DL_FREE(IBG(timestampformat));
	IBG(timestampformat) = DL_STRDUP(IBG(cfg_timestampformat));

	if (IBG(dateformat))
        DL_FREE(IBG(dateformat));
	IBG(dateformat) = DL_STRDUP(IBG(cfg_dateformat));

	if (IBG(timeformat))
        DL_FREE(IBG(timeformat));
	IBG(timeformat) = DL_STRDUP(IBG(cfg_timeformat));

	if (IBG(errmsg))
	    DL_FREE(IBG(errmsg));
	IBG(errmsg) = DL_MALLOC(sizeof(char)*MAX_ERRMSG+1);

	return SUCCESS;
}


PHP_MSHUTDOWN_FUNCTION(ibase)
{
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}


PHP_RSHUTDOWN_FUNCTION(ibase)
{
    IBLS_FETCH();

	if (IBG(timestampformat))
	    DL_FREE(IBG(timestampformat));
	IBG(timestampformat) = NULL;

	if (IBG(dateformat))
	    DL_FREE(IBG(dateformat));
	IBG(dateformat) = NULL;

	if (IBG(timeformat))
	    DL_FREE(IBG(timeformat));
	IBG(timeformat) = NULL;
    
	if (IBG(errmsg))
	    DL_FREE(IBG(errmsg));
	IBG(errmsg) = NULL;

    return SUCCESS;
} 

 
PHP_MINFO_FUNCTION(ibase)
{
	char tmp[32];
	IBLS_FETCH();

	php_info_print_table_start();
	php_info_print_table_row(2, "Interbase Support", "enabled");    
	php_info_print_table_row(2, "Revision", "$Revision$");
#ifdef COMPILE_DL_INTERBASE
	php_info_print_table_row(2, "Dynamic Module", "yes");
#endif
	php_info_print_table_row(2, "Allow Persistent Links", (IBG(allow_persistent)?"Yes":"No") );

	if (IBG(max_persistent) == -1) {
		snprintf(tmp, 31, "%ld/unlimited", IBG(num_persistent));
	} else {
		snprintf(tmp, 31, "%ld/%ld", IBG(num_persistent), IBG(max_persistent));
	}
	tmp[31]=0;
	php_info_print_table_row(2, "Persistent Links", tmp );

	if (IBG(max_links) == -1) {
		snprintf(tmp, 31, "%ld/unlimited", IBG(num_links));
	} else {
		snprintf(tmp, 31, "%ld/%ld", IBG(num_links), IBG(max_links));
	}
	tmp[31]=0;
	php_info_print_table_row(2, "Total Links", tmp );

	php_info_print_table_row(2, "Timestamp Format", IBG(timestampformat) );
	php_info_print_table_row(2, "Date Format", IBG(dateformat) );
	php_info_print_table_row(2, "Time Format", IBG(timeformat) );

	php_info_print_table_end();
}
/* }}} */


/* {{{ _php_ibase_attach_db() */
static int _php_ibase_attach_db(char *server, char *uname, char *passwd, char *charset, int buffers, char *role, isc_db_handle *db)
{
	char dpb_buffer[256], *dpb, *p;
	int dpb_length, len;
	IBLS_FETCH();

	dpb = dpb_buffer;

	*dpb++ = isc_dpb_version1;

	if (uname != NULL && (len = strlen(uname))) {
		*dpb++ = isc_dpb_user_name;
		*dpb++ = len;
		for (p = uname; *p;) {
			*dpb++ = *p++;
		}
	}

	if (passwd != NULL && (len = strlen(passwd))) {
		*dpb++ = isc_dpb_password;
		*dpb++ = strlen(passwd);
		for (p = passwd; *p;) {
			*dpb++ = *p++;
		}
	}

	if (charset != NULL && (len = strlen(charset))) {
		*dpb++ = isc_dpb_lc_ctype;
		*dpb++ = strlen(charset);
		for (p = charset; *p;) {
			*dpb++ = *p++;
		}
	}

	if (buffers) {
		*dpb++ = isc_dpb_num_buffers;
		*dpb++ = 1;
		*dpb++ = buffers;
	}

#ifdef isc_dpb_sql_role_name
	if (role != NULL && (len = strlen(role))) {
		*dpb++ = isc_dpb_sql_role_name;
		*dpb++ = strlen(role);
		for (p = role; *p;) {
			*dpb++ = *p++;
		}
	}
#endif

	dpb_length = dpb - dpb_buffer;

	if (isc_attach_database(IB_STATUS, (short) strlen(server), server, db, (short) dpb_length, dpb_buffer)) {
		_php_ibase_error();
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */


/* {{{ _php_ibase_connect() */
static void _php_ibase_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	pval ***args;
	char *ib_server = NULL, *ib_uname, *ib_passwd, *ib_charset = NULL, *ib_buffers = NULL, *ib_dialect = NULL, *ib_role = NULL;
	int i, ib_uname_len, ib_passwd_len;
	isc_db_handle db_handle = NULL;
	char *hashed_details;
	int hashed_details_length = 0;
	ibase_db_link *ib_link = NULL;
	IBLS_FETCH();
	PLS_FETCH();
	
	RESET_ERRMSG;
		
	ib_uname = IBG(default_user);
	ib_passwd = IBG(default_password);
	ib_uname_len = ib_uname ? strlen(ib_uname) : 0;
	ib_passwd_len = ib_passwd ? strlen(ib_passwd) : 0;
	
	if(ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 7){
		WRONG_PARAM_COUNT;
	}
	
	args = (pval ***) emalloc(sizeof(pval **)*ZEND_NUM_ARGS());
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}

	switch(ZEND_NUM_ARGS()) {
		case 7:
			convert_to_string_ex(args[6]);
			ib_role = (*args[6])->value.str.val;
			hashed_details_length += (*args[6])->value.str.len;
			/* fallout */
		case 6:
			convert_to_string_ex(args[5]);
			ib_dialect = (*args[5])->value.str.val;
			hashed_details_length += (*args[5])->value.str.len;
			/* fallout */
		case 5:
			convert_to_string_ex(args[4]);
			ib_buffers = (*args[4])->value.str.val;
			hashed_details_length += (*args[4])->value.str.len;
			/* fallout */
		case 4:
			convert_to_string_ex(args[3]);
			ib_charset = (*args[3])->value.str.val;
			hashed_details_length += (*args[3])->value.str.len;
			/* fallout */
		case 3:
			convert_to_string_ex(args[2]);
			ib_passwd = (*args[2])->value.str.val;
			hashed_details_length += (*args[2])->value.str.len;
			/* fallout */
		case 2:
			convert_to_string_ex(args[1]);
			ib_uname = (*args[1])->value.str.val;
			hashed_details_length += (*args[1])->value.str.len;
			/* fallout */
		case 1:
			convert_to_string_ex(args[0]);
			ib_server = (*args[0])->value.str.val;
			hashed_details_length += (*args[0])->value.str.len;
	} /* case */
	
	efree(args);
	
	hashed_details = (char *) emalloc(hashed_details_length+strlen("ibase_%s_%s_%s_%s_%s_%s_%s")+1);
	sprintf(hashed_details, "ibase_%s_%s_%s_%s_%s_%s_%s", SAFE_STRING(ib_server), SAFE_STRING(ib_uname), SAFE_STRING(ib_passwd), SAFE_STRING(ib_charset), SAFE_STRING(ib_buffers), SAFE_STRING(ib_dialect), SAFE_STRING(ib_role));

	if (persistent) {
		list_entry *le;
		int open_new_connection = 1;
		
		if (zend_hash_find(&EG(persistent_list), hashed_details, hashed_details_length+1, (void **) &le) != FAILURE) {
			char tmp_1[] = {isc_info_base_level, isc_info_end};
			char tmp_2[8]; /* Enough? Hope so... */ 

			if (le->type != le_plink) {
				RETURN_FALSE;
			}
			/* Check if connection has timed out */
			ib_link = (ibase_db_link *) le->ptr;
			if (!isc_database_info(IB_STATUS, &ib_link->link, sizeof(tmp_1), tmp_1, sizeof(tmp_2), tmp_2)) {
				open_new_connection = 0;
			}
		}
		
		/* There was no previous connection to use or it has timed out */
		if (open_new_connection) {
			list_entry new_le;
			
			if ((IBG(max_links) != -1) && (IBG(num_links) >= IBG(max_links))) {
				_php_ibase_module_error("Too many open links (%d)", IBG(num_links));
				efree(hashed_details);
				RETURN_FALSE;
			}
			if ((IBG(max_persistent) != -1) && (IBG(num_persistent) >= IBG(max_persistent))) {
				_php_ibase_module_error("Too many open persistent links (%d)", IBG(num_persistent));
				efree(hashed_details);
				RETURN_FALSE;
			}

			/* create the ib_link */

			if (_php_ibase_attach_db(ib_server, ib_uname, ib_passwd, ib_charset,
									 (ib_buffers ? strtoul(ib_buffers, NULL, 0) : 0),
									 ib_role, &db_handle) == FAILURE) {
				efree(hashed_details);
				RETURN_FALSE;
			}

			ib_link = (ibase_db_link *) malloc(sizeof(ibase_db_link));
			ib_link->link = db_handle;
			ib_link->dialect = (ib_dialect ? (unsigned short) strtoul(ib_dialect, NULL, 10) : SQL_DIALECT_CURRENT);
			for (i = 0; i < IBASE_TRANS_ON_LINK; i++)
				ib_link->trans[i] = NULL;
			
			/* hash it up */
			new_le.type = le_plink;
			new_le.ptr = ib_link;
			if (zend_hash_update(&EG(persistent_list), hashed_details, hashed_details_length+1, (void *) &new_le, sizeof(list_entry), NULL)==FAILURE) {
				efree(hashed_details);
				free(ib_link);
				RETURN_FALSE;
			}
			IBG(num_links)++;
			IBG(num_persistent)++;
		}

		ZEND_REGISTER_RESOURCE(return_value, ib_link, le_plink);

	} else {
		list_entry *index_ptr, new_index_ptr;
		
		/* first we check the hash for the hashed_details key.	if it exists,
		 * it should point us to the right offset where the actual ib_link sits.
		 * if it doesn't, open a new ib_link, add it to the resource list,
		 * and add a pointer to it with hashed_details as the key.
		 */
		if (zend_hash_find(&EG(regular_list), hashed_details, hashed_details_length+1, (void **) &index_ptr)==SUCCESS) {
			int type,xlink;
			void *ptr;
			if (index_ptr->type != le_index_ptr) {
				RETURN_FALSE;
			}
			xlink = (int) index_ptr->ptr;
			ptr = zend_list_find(xlink, &type);	 /* check if the xlink is still there */
			if (ptr && (type==le_link || type==le_plink)) {
				zend_list_addref(xlink);
				return_value->value.lval = xlink;
				return_value->type = IS_RESOURCE;
				IBG(default_link) = return_value->value.lval;
				efree(hashed_details);
				return;
			} else {
				zend_hash_del(&EG(regular_list), hashed_details, hashed_details_length+1);
			}
		}
		if ((IBG(max_links) != -1) && (IBG(num_links) >= IBG(max_links))) {
			_php_ibase_module_error("Too many open links (%d)", IBG(num_links));
			efree(hashed_details);
			RETURN_FALSE;
		}
		/* create the ib_link */

		if (_php_ibase_attach_db(ib_server, ib_uname, ib_passwd, ib_charset,
								 (ib_buffers ? strtoul(ib_buffers, NULL, 0) : 0),
								 ib_role, &db_handle) == FAILURE) {
			efree(hashed_details);
			RETURN_FALSE;
		}

		ib_link = (ibase_db_link *) emalloc(sizeof(ibase_db_link));
		ib_link->link = db_handle;
		ib_link->dialect = (ib_dialect ? (unsigned short) strtoul(ib_dialect, NULL, 10) : SQL_DIALECT_CURRENT);
		for (i = 0; i < IBASE_TRANS_ON_LINK; i++)
			ib_link->trans[i] = NULL;

		ZEND_REGISTER_RESOURCE(return_value, ib_link, le_link);

		/* add it to the hash */
		new_index_ptr.ptr = (void *) return_value->value.lval;
		new_index_ptr.type = le_index_ptr;
		if (zend_hash_update(&EG(regular_list), hashed_details, hashed_details_length+1, (void *) &new_index_ptr, sizeof(list_entry), NULL)==FAILURE) {
			efree(hashed_details);
			RETURN_FALSE;
		}
		IBG(num_links)++;
	}
	efree(hashed_details);
	zend_list_addref(return_value->value.lval);
	IBG(default_link) = return_value->value.lval;
}
/* }}} */


/* {{{ proto int ibase_connect(string database [, string username] [, string password] [, string charset] [, int buffers] [, int dialect] [, string role])
   Open a connection to an InterBase database */
PHP_FUNCTION(ibase_connect)
{
	_php_ibase_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */


/* {{{ proto int ibase_pconnect(string database [, string username] [, string password] [, string charset] [, int buffers] [, int dialect] [, string role])
   Open a persistent connection to an InterBase database */
PHP_FUNCTION(ibase_pconnect)
{
	_php_ibase_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */


/* {{{ proto int ibase_close([int link_identifier])
   Close an InterBase connection */
PHP_FUNCTION(ibase_close)
{
	zval **link_arg;
	ibase_db_link *ib_link;
	int link_id;
	IBLS_FETCH();
	
	RESET_ERRMSG;
	
	switch (ZEND_NUM_ARGS()) {
		case 0:
			link_id = IBG(default_link);
			break;
		case 1:
			if (zend_get_parameters_ex(1, &link_arg) == FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(link_arg);
			link_id = (*link_arg)->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, link_arg, link_id, "InterBase link", le_link, le_plink);
	zend_list_delete(link_id);
	RETURN_TRUE;
}
/* }}} */

/* {{{ _php_ibase_alloc_array() */
static int _php_ibase_alloc_array(ibase_array **ib_arrayp, int *array_cntp,
								  XSQLDA *sqlda, isc_db_handle link, isc_tr_handle trans)
{
#define IB_ARRAY (*ib_arrayp)
	
	int i, dim, ar_cnt, ar_length;
	XSQLVAR *var;
	IBLS_FETCH();	

	IB_ARRAY = NULL;
	
	ar_cnt = 0; /* find arrays */
	var = sqlda->sqlvar;
	for (i = 0; i < sqlda->sqld; i++, var++) {
		if ((var->sqltype & ~1) == SQL_ARRAY)
			ar_cnt++;
	}

	if (ar_cnt) {	  /* have  arrays ? */

		*array_cntp = ar_cnt;
		IB_ARRAY = emalloc(sizeof(ibase_array)*ar_cnt);
		ar_cnt = 0;
		var = sqlda->sqlvar;
		for (i = 0; i < sqlda->sqld; i++, var++) {
			if ((var->sqltype & ~1) == SQL_ARRAY) {
				
				ISC_ARRAY_DESC *ar_desc = &IB_ARRAY[ar_cnt].ar_desc;
				
				if (isc_array_lookup_bounds(IB_STATUS, &link, &trans,
											var->relname, var->sqlname, ar_desc)) {
					_php_ibase_error();
					efree(IB_ARRAY);
					IB_ARRAY = NULL;
					return FAILURE;
				}

				switch (ar_desc->array_desc_dtype) {
					case blr_text:
					case blr_text2:
						IB_ARRAY[ar_cnt].el_type = SQL_TEXT;
						IB_ARRAY[ar_cnt].el_size = ar_desc->array_desc_length+1;
						break;
					case blr_short:
						IB_ARRAY[ar_cnt].el_type = SQL_SHORT;
						IB_ARRAY[ar_cnt].el_size = sizeof(short);
						break;
					case blr_long:
						IB_ARRAY[ar_cnt].el_type = SQL_LONG;
						IB_ARRAY[ar_cnt].el_size = sizeof(long);
						break;
					case blr_float:
						IB_ARRAY[ar_cnt].el_type = SQL_FLOAT;
						IB_ARRAY[ar_cnt].el_size = sizeof(float);
						break;
					case blr_double:
						IB_ARRAY[ar_cnt].el_type = SQL_DOUBLE;
						IB_ARRAY[ar_cnt].el_size = sizeof(double);
						break;
					case blr_date:
						IB_ARRAY[ar_cnt].el_type = SQL_DATE;
						IB_ARRAY[ar_cnt].el_size = sizeof(ISC_QUAD);
						break;
					case blr_varying:
					case blr_varying2:	/* changed to SQL_TEXT ? */
						/* sql_type = SQL_VARYING; Why? FIXME: ??? */
						IB_ARRAY[ar_cnt].el_type = SQL_TEXT;
						IB_ARRAY[ar_cnt].el_size = ar_desc->array_desc_length+sizeof(short);
						break;
					default:
						_php_ibase_module_error("unexpected array type %d in relation '%s' column '%s')",
												ar_desc->array_desc_dtype,var->relname, var->sqlname);
						efree(IB_ARRAY);
						IB_ARRAY = NULL;
						return FAILURE;
				} /* switch array_desc_type */
				
				ar_length = 0; /* calculate elements count */
				for (dim = 0; dim < ar_desc->array_desc_dimensions; dim++) {
					ar_length += 1 + ar_desc->array_desc_bounds[dim].array_bound_upper
						- ar_desc->array_desc_bounds[dim].array_bound_lower;
				}
				IB_ARRAY[ar_cnt].ar_size = IB_ARRAY[ar_cnt].el_size * ar_length;
				
				ar_cnt++;
				
			} /* if SQL_ARRAY */
		} /* for column */
	} /* if array_cnt */
	
	return SUCCESS;
#undef IB_ARRAY
}
/* }}} */


/* {{{ _php_ibase_alloc_query() */
/* allocate and prepare query */
static int _php_ibase_alloc_query(ibase_query **ib_queryp, isc_db_handle link, isc_tr_handle trans, char *query, unsigned short dialect)
{
#define IB_QUERY (*ib_queryp)
	IBLS_FETCH();
	
	IB_QUERY = emalloc(sizeof(ibase_query));
	IB_QUERY->link = link;
	IB_QUERY->trans = trans;
	IB_QUERY->stmt = NULL;
	IB_QUERY->out_sqlda = NULL;
	IB_QUERY->in_sqlda = NULL;
	IB_QUERY->in_array = NULL;
	IB_QUERY->in_array_cnt = 0;
	IB_QUERY->out_array = NULL;
	IB_QUERY->out_array_cnt = 0;
	IB_QUERY->dialect = dialect;
	
	if (isc_dsql_allocate_statement(IB_STATUS, &link, &IB_QUERY->stmt)) {
		_php_ibase_error();
		goto _php_ibase_alloc_query_error;
	}

	IB_QUERY->out_sqlda = (XSQLDA *) emalloc(XSQLDA_LENGTH(0));
	IB_QUERY->out_sqlda->sqln = 0;
	IB_QUERY->out_sqlda->version = SQLDA_VERSION1;

	if (isc_dsql_prepare(IB_STATUS, &IB_QUERY->trans, &IB_QUERY->stmt, 0, query, dialect, IB_QUERY->out_sqlda)) {
		_php_ibase_error();
		goto _php_ibase_alloc_query_error;
	}
	
	/* not enough output variables ? */
	if (IB_QUERY->out_sqlda->sqld > IB_QUERY->out_sqlda->sqln) {
		IB_QUERY->out_sqlda = erealloc(IB_QUERY->out_sqlda,XSQLDA_LENGTH(IB_QUERY->out_sqlda->sqld));
		IB_QUERY->out_sqlda->sqln = IB_QUERY->out_sqlda->sqld;
		IB_QUERY->out_sqlda->version = SQLDA_VERSION1;
		if (isc_dsql_describe(IB_STATUS, &IB_QUERY->stmt, SQLDA_VERSION1, IB_QUERY->out_sqlda)) {
			_php_ibase_error();
			goto _php_ibase_alloc_query_error;
		}
	}

	/* maybe have input placeholders? */
	IB_QUERY->in_sqlda = emalloc(XSQLDA_LENGTH(0));
	IB_QUERY->in_sqlda->sqln = 0;
	IB_QUERY->in_sqlda->version = SQLDA_VERSION1;
	if (isc_dsql_describe_bind(IB_STATUS, &IB_QUERY->stmt, SQLDA_VERSION1, IB_QUERY->in_sqlda)) {
		_php_ibase_error();
		goto _php_ibase_alloc_query_error;
	}
	
	/* not enough input variables ? */
	if (IB_QUERY->in_sqlda->sqln < IB_QUERY->in_sqlda->sqld) {
		IB_QUERY->in_sqlda = erealloc(IB_QUERY->in_sqlda,XSQLDA_LENGTH(IB_QUERY->in_sqlda->sqld));
		IB_QUERY->in_sqlda->sqln = IB_QUERY->in_sqlda->sqld;
		IB_QUERY->in_sqlda->version = SQLDA_VERSION1;
		if (isc_dsql_describe_bind(IB_STATUS, &IB_QUERY->stmt, SQLDA_VERSION1, IB_QUERY->in_sqlda)) {
			_php_ibase_error();
			goto _php_ibase_alloc_query_error;
		}
	}
	
	/* allocate arrays... */
	if (_php_ibase_alloc_array(&IB_QUERY->in_array, &IB_QUERY->in_array_cnt,
							   IB_QUERY->in_sqlda, link, trans) == FAILURE) {
		goto _php_ibase_alloc_query_error; /* error report already done */
	}
	
	if (_php_ibase_alloc_array(&IB_QUERY->out_array, &IB_QUERY->out_array_cnt,
							   IB_QUERY->out_sqlda, link, trans) == FAILURE) {
		goto _php_ibase_alloc_query_error;
	}

	/* no, haven't placeholders at all */
	if (IB_QUERY->in_sqlda->sqld == 0) {
		efree(IB_QUERY->in_sqlda);
		IB_QUERY->in_sqlda = NULL;
	}

	if (IB_QUERY->out_sqlda->sqld == 0) {
		efree(IB_QUERY->out_sqlda);
		IB_QUERY->out_sqlda = NULL;
	}

	return SUCCESS;
	
_php_ibase_alloc_query_error:
	
	if (IB_QUERY->out_sqlda)
		efree(IB_QUERY->out_sqlda);
	if (IB_QUERY->in_sqlda)
		efree(IB_QUERY->in_sqlda);
	if (IB_QUERY->out_array)
		efree(IB_QUERY->out_array);
	efree(IB_QUERY);
	IB_QUERY = NULL;
	
	return FAILURE;
#undef IB_QUERY
}
/* }}} */


/* {{{ _php_ibase_bind()
   Bind parameter placeholders in a previously prepared query */
static int _php_ibase_bind(XSQLDA *sqlda, pval **b_vars, BIND_BUF *buf)
{
	XSQLVAR *var;
	pval *b_var;
	int i;

	var = sqlda->sqlvar;
	for (i = 0; i < sqlda->sqld; var++, i++) { /* binded vars */
		
		buf[i].sqlind = 0;
		var->sqlind	 = &buf[i].sqlind;
		b_var = b_vars[i];
		
		switch(var->sqltype & ~1) {
			case SQL_TEXT:			   /* direct to variable */
			case SQL_VARYING:
				convert_to_string(b_var);
				var->sqldata = (void ISC_FAR *)b_var->value.str.val;
				var->sqllen	 = b_var->value.str.len;
				var->sqltype = SQL_TEXT + (var->sqltype & 1);
				break;
			case SQL_SHORT:
				convert_to_long(b_var);
				if (b_var->value.lval > SHRT_MAX || b_var->value.lval < SHRT_MIN) {
					_php_ibase_module_error("field %*s overflow", var->aliasname_length, var->aliasname);
					return FAILURE;
				}
				buf[i].val.sval = (short)b_var->value.lval;
				var->sqldata = (void ISC_FAR *)(&buf[i].val.sval);
				break;
			case SQL_LONG:
				if (var->sqlscale < 0) {
					/*
					  DECIMAL or NUMERIC field stored internally as scaled integer.
					  Coerce it to string and let InterBase's internal routines
					  handle it.
					*/
					convert_to_string(b_var);
					var->sqldata = (void ISC_FAR *)b_var->value.str.val;
					var->sqllen	 = b_var->value.str.len;
					var->sqltype = SQL_TEXT;
				} else {
					convert_to_long(b_var);
					var->sqldata = (void ISC_FAR *)(&b_var->value.lval);
				}
				break;
			case SQL_FLOAT:
				convert_to_double(b_var);
				buf[i].val.fval = (float)b_var->value.dval;
				var->sqldata = (void ISC_FAR *)(&buf[i].val.fval);
				break;
			case SQL_DOUBLE:  /* direct to variable */
				convert_to_double(b_var);
				var->sqldata = (void ISC_FAR *)(&b_var->value.dval);
				break;
#ifdef SQL_INT64
			case SQL_INT64:
				/*
				  Just let InterBase's internal routines handle it.
				  Besides, it might even have originally been a string
				  to avoid rounding errors...
				*/
				convert_to_string(b_var);
				var->sqldata = (void ISC_FAR *)b_var->value.str.val;
				var->sqllen	 = b_var->value.str.len;
				var->sqltype = SQL_TEXT;
				break;
#endif
#ifndef SQL_TIMESTAMP
			case SQL_DATE:
#else
			case SQL_TIMESTAMP:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
#endif
#ifndef HAVE_STRPTIME
#ifndef SQL_TIMESTAMP
				/* Parsing doesn't seem to happen with older versions... */
				{
					struct tm t;
					int n;
					
					t.tm_year = t.tm_mon = t.tm_mday = t.tm_hour =
					t.tm_min = t.tm_sec = 0;
					
					convert_to_string(b_var);
					
					n = sscanf(b_var->value.str.val,"%d%*[/]%d%*[/]%d %d%*[:]%d%*[:]%d",
						   &t.tm_mon, &t.tm_mday, &t.tm_year,  &t.tm_hour, &t.tm_min, &t.tm_sec);
					if(n != 3 && n != 6){
						_php_ibase_module_error("invalid date/time format");
						return FAILURE;
					}
					t.tm_year -= 1900;
					t.tm_mon--;
					isc_encode_date(&t, &buf[i].val.qval);
					var->sqldata = (void ISC_FAR *)(&buf[i].val.qval);
				}
#else
				/*
				  Once again, InterBase's internal parsing routines
				  seems to be a good solution... Might change this on
				  platforms that have strptime()? Code is there and works,
				  but the functions existence is not yet tested...
				  ask Sascha?
				*/
				convert_to_string(b_var);
				var->sqldata = (void ISC_FAR *)b_var->value.str.val;
				var->sqllen = b_var->value.str.len;
				var->sqltype = SQL_TEXT;
#endif
#else
				{
					struct tm t;

					convert_to_string(b_var);
#ifndef SQL_TIMESTAMP
					strptime(b_var->value.str.val, IBG(timestampformat), &t);
					isc_encode_date(&t, &buf[i].val.qval);
					var->sqldata = (void ISC_FAR *)(&buf[i].val.qval);
#else
					switch (var->sqltype & ~1) {
						case SQL_TIMESTAMP:
							strptime(b_var->value.str.val, IBG(timestampformat), &t);
							isc_encode_timestamp(&t, &buf[i].val.tsval);
							var->sqldata = (void ISC_FAR *)(&buf[i].val.tsval);
							break;
						case SQL_TYPE_DATE:
							strptime(b_var->value.str.val, IBG(dateformat), &t);
							isc_encode_sql_date(&t, &buf[i].val.dtval);
							var->sqldata = (void ISC_FAR *)(&buf[i].val.dtval);
							break;
						case SQL_TYPE_TIME:
							strptime(b_var->value.str.val, IBG(timeformat), &t);
							isc_encode_sql_time(&t, &buf[i].val.tmval);
							var->sqldata = (void ISC_FAR *)(&buf[i].val.tmval);
							break;
					}
#endif
				}
#endif
				break;
			case SQL_BLOB:
				{
					ibase_blob_handle *ib_blob_id;
					if (b_var->type != IS_STRING
						|| b_var->value.str.len != sizeof(ibase_blob_handle)
						|| ((ibase_blob_handle *)(b_var->value.str.val))->bl_handle != 0) {
						_php_ibase_module_error("invalid blob id string");
						return FAILURE;
					}
					ib_blob_id = (ibase_blob_handle *)b_var->value.str.val;
					
					var->sqldata = (void ISC_FAR *)&ib_blob_id->bl_qd;
				}
			break;
			case SQL_ARRAY:
				_php_ibase_module_error("binding arrays not supported yet");
				return FAILURE;
			break;
		} /*switch*/
	} /*for*/
	return SUCCESS;
}
/* }}} */


/* {{{ _php_ibase_alloc_xsqlda() */
static void _php_ibase_alloc_xsqlda(XSQLDA *sqlda)
{
	int i;
	XSQLVAR *var = sqlda->sqlvar;
	
	
	for (i = 0; i < sqlda->sqld; i++, var++) {
		switch(var->sqltype & ~1) {
			case SQL_TEXT:
				var->sqldata = emalloc(sizeof(char)*(var->sqllen));
				break;
			case SQL_VARYING:
				var->sqldata = emalloc(sizeof(char)*(var->sqllen+sizeof(short)));
				break;
			case SQL_SHORT:
				var->sqldata = emalloc(sizeof(short));
				break;
			case SQL_LONG:
				var->sqldata = emalloc(sizeof(long));
				break;
			case SQL_FLOAT:
				var->sqldata = emalloc(sizeof(float));
					break;
			case SQL_DOUBLE:
				var->sqldata = emalloc(sizeof(double));
				break;
#ifdef SQL_INT64
			case SQL_INT64:
				var->sqldata = emalloc(sizeof(ISC_INT64));
				break;
#endif
#ifdef SQL_TIMESTAMP
			case SQL_TIMESTAMP:
				var->sqldata = emalloc(sizeof(ISC_TIMESTAMP));
				break;
			case SQL_TYPE_DATE:
				var->sqldata = emalloc(sizeof(ISC_DATE));
				break;
			case SQL_TYPE_TIME:
				var->sqldata = emalloc(sizeof(ISC_TIME));
				break;
#else
			case SQL_DATE:
#endif
			case SQL_BLOB:
			case SQL_ARRAY:
				var->sqldata = emalloc(sizeof(ISC_QUAD));
				break;
		} /*switch*/
		if (var->sqltype & 1) /* sql NULL flag */
			var->sqlind = emalloc(sizeof(short));
		else
			var->sqlind = NULL;
	} /* for*/
}
/* }}} */


/* {{{ _php_ibase_exec() */
static int _php_ibase_exec(ibase_result **ib_resultp, ibase_query *ib_query, int argc, pval **args)
{
#define IB_RESULT (*ib_resultp)
	XSQLDA *in_sqlda = NULL, *out_sqlda = NULL;
	BIND_BUF *bind_buf = NULL;
	int rv = FAILURE;
	IBLS_FETCH();
	
	IB_RESULT = NULL;
	
	/* allocate sqlda and output buffers */
	if (ib_query->out_sqlda) { /* output variables in select, select for update */
		IBDEBUG("Query wants XSQLDA for output");
		IB_RESULT = emalloc(sizeof(ibase_result));
		IB_RESULT->link = ib_query->link;
		IB_RESULT->trans = ib_query->trans;
		IB_RESULT->stmt = ib_query->stmt; 
		IB_RESULT->drop_stmt = 0; /* when free result close but not drop!*/

		out_sqlda = IB_RESULT->out_sqlda = emalloc(XSQLDA_LENGTH(ib_query->out_sqlda->sqld));
		memcpy(out_sqlda,ib_query->out_sqlda,XSQLDA_LENGTH(ib_query->out_sqlda->sqld));
		_php_ibase_alloc_xsqlda(out_sqlda);

		if (ib_query->out_array) {
			IB_RESULT->out_array = emalloc(sizeof(ibase_array)*ib_query->out_array_cnt);
			memcpy(IB_RESULT->out_array, ib_query->out_array, sizeof(ibase_array)*ib_query->out_array_cnt);
		} else {
			IB_RESULT->out_array = NULL;
		}
	}

	if (ib_query->in_sqlda) { /* has placeholders */
		IBDEBUG("Query wants XSQLDA for input");
		if (ib_query->in_sqlda->sqld != argc) {
			_php_ibase_module_error("placeholders (%d) and variables (%d) mismatch", ib_query->in_sqlda->sqld, argc);
			goto _php_ibase_exec_error;  /* yes mommy, goto! */
		}
		in_sqlda = emalloc(XSQLDA_LENGTH(ib_query->in_sqlda->sqld));
		memcpy(in_sqlda, ib_query->in_sqlda, XSQLDA_LENGTH(ib_query->in_sqlda->sqld));
		bind_buf = emalloc(sizeof(BIND_BUF) * ib_query->in_sqlda->sqld);
		if (_php_ibase_bind(in_sqlda, args, bind_buf) == FAILURE) {
			IBDEBUG("Could not bind input XSQLDA... (_php_ibase_exec)");
			goto _php_ibase_exec_error;
		}
	}

	if (isc_dsql_execute(IB_STATUS, &ib_query->trans, &ib_query->stmt, ib_query->dialect, in_sqlda)) {
		IBDEBUG("Could not execute query... (_php_ibase_exec)");
		_php_ibase_error();
		goto _php_ibase_exec_error;
	}

	rv = SUCCESS;
	
_php_ibase_exec_error:		 /* I'm a bad boy... */
	
	if (in_sqlda){
		efree(in_sqlda);
	}
	if (bind_buf)
		efree(bind_buf);

	if (rv == FAILURE) {
		if (IB_RESULT) {
			efree(IB_RESULT);
			IB_RESULT = NULL;
		}
		if (out_sqlda)
			_php_ibase_free_xsqlda(out_sqlda);
	}
	
	return rv;
#undef IB_RESULT
}
/* }}} */


/* {{{ proto int ibase_trans([int trans_args [, int link_identifier]])
   Start transaction */
PHP_FUNCTION(ibase_trans)
{
	pval ***args;
	char tpb[20], *tpbp = NULL;
	long trans_argl = 0;
	int tpb_len = 0, argn, link_id, trans_n = 0, i;
	ibase_db_link *ib_link;
	ibase_tr_link *ib_trans;
	IBLS_FETCH();
	
	RESET_ERRMSG;

	link_id = IBG(default_link);

	/* TODO: multi-databases trans */
	argn = ZEND_NUM_ARGS();
	if (argn < 0 || argn > 20) {
		WRONG_PARAM_COUNT;
	}

	if (argn) {
		args = (pval ***) emalloc(sizeof(pval **)*argn);
		if (zend_get_parameters_array_ex(argn, args) == FAILURE) {
			efree(args);
			RETURN_FALSE;
		}

		/* Handle all database links, although we don't support multibase
		   transactions yet, so only the last one is will be used. */
		for (i = argn-1; i > 0 && Z_TYPE_PP(args[i]) == IS_RESOURCE; i--) {
			ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, args[i], -1, "InterBase link", le_link, le_plink);
			link_id = Z_LVAL_PP(args[i]);
		}

		/* First argument is transaction parameters */
		convert_to_long_ex(args[0]);
		trans_argl = (*args[0])->value.lval;

		efree(args);
	}

	if (argn < 2) {
		ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, NULL, link_id, "InterBase link", le_link, le_plink);
	}

	if (trans_argl ) {
		tpb[tpb_len++] = isc_tpb_version3;
		tpbp = tpb;
		/* access mode */
		if (trans_argl & PHP_IBASE_READ) /* READ ONLY TRANSACTION */
			tpb[tpb_len++] = isc_tpb_read;
		else
			tpb[tpb_len++] = isc_tpb_write;
		/* isolation level */
		if (trans_argl & PHP_IBASE_COMMITTED) {
			tpb[tpb_len++] = isc_tpb_read_committed;
		} else if (trans_argl & PHP_IBASE_CONSISTENCY)
			tpb[tpb_len++] = isc_tpb_consistency;
		else 
			tpb[tpb_len++] = isc_tpb_concurrency;
		/* lock resolution */
		if (trans_argl & PHP_IBASE_NOWAIT)
			tpb[tpb_len++] = isc_tpb_nowait;
		else 
			tpb[tpb_len++] = isc_tpb_wait;

	}

	/* find empty transaction slot */
	for (trans_n = 0; trans_n < IBASE_TRANS_ON_LINK
		&& ib_link->trans[trans_n]; trans_n++)
		;
	if (trans_n == IBASE_TRANS_ON_LINK) {
		_php_ibase_module_error("too many transactions on link");
		RETURN_FALSE;
	}
	
	if (isc_start_transaction(IB_STATUS, &ib_link->trans[trans_n], 1, &ib_link->link, tpb_len, tpbp)) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	ib_trans = (ibase_tr_link *) emalloc(sizeof(ibase_tr_link));
	ib_trans->trans_num = trans_n;
	ib_trans->link_rsrc = link_id;
	ZEND_REGISTER_RESOURCE(return_value, ib_trans, le_trans);
}
/* }}} */


/* {{{ _php_ibase_def_trans() */
/* open default transaction */
static int _php_ibase_def_trans(ibase_db_link * ib_link, int trans_n)
{
	IBLS_FETCH();
	
	if (trans_n == 0 && ib_link->trans[0] == NULL) { 
		if (isc_start_transaction(IB_STATUS, &ib_link->trans[0], 1, &ib_link->link, 0, NULL)) {
			_php_ibase_error();
			return FAILURE;
		}
	}
	return SUCCESS;
}
/*}}}*/


/* {{{ _php_ibase_trans_end() */
#define COMMIT 1
#define ROLLBACK 0
static void _php_ibase_trans_end(INTERNAL_FUNCTION_PARAMETERS, int commit)
{
	pval **link_trans_arg = NULL;
	int link_id = 0, trans_n = 0, trans_id = 0;
	ibase_db_link *ib_link;
	IBLS_FETCH();

	RESET_ERRMSG;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			link_id = IBG(default_link);
			ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, link_trans_arg, link_id, "InterBase link", le_link, le_plink);
			break;
		case 1: 
			if (zend_get_parameters_ex(1, &link_trans_arg) == FAILURE) {
				RETURN_FALSE;
			}
			get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, link_trans_arg, &ib_link, &trans_n, &trans_id);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	if (ib_link->trans[trans_n] == NULL) {
		php_error(E_WARNING, "Trying to commit or rollback an already handled transaction");
		RETURN_FALSE;
	}

	if (commit) {
		if (isc_commit_transaction(IB_STATUS, &ib_link->trans[trans_n])) {
			_php_ibase_error();
			RETURN_FALSE;
		}
	} else {
		if (isc_rollback_transaction(IB_STATUS, &ib_link->trans[trans_n])) {
			_php_ibase_error();
			RETURN_FALSE;
		}
	}
	ib_link->trans[trans_n] = NULL;

	/* Don't try to destroy imnplicitly opened transaction from list... */
	if (trans_id) {
		zend_list_delete(trans_id);
	}
	
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int ibase_commit([int link_identifier,] int trans_number)
   Commit transaction */
PHP_FUNCTION(ibase_commit)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT);
}
/* }}} */


/* {{{ proto int ibase_rollback([int link_identifier,] int trans_number)
   Roolback transaction */
PHP_FUNCTION(ibase_rollback)
{
	_php_ibase_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK);
}
/* }}} */


/* {{{ proto int ibase_query([int link_identifier,] string query [, int bind_args])
   Execute a query */
PHP_FUNCTION(ibase_query)
{
	pval ***args, **bind_args = NULL, **dummy = NULL;
	int i, link_id = 0, trans_n = 0, bind_n = 0, trans_id = 0;
	char *query;
	ibase_db_link *ib_link;
	ibase_query *ib_query;
	ibase_result *ib_result;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS() < 1) {
		WRONG_PARAM_COUNT;
	}

	args = (pval ***) emalloc(sizeof(pval **)*ZEND_NUM_ARGS());
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}

	i = 0;
	if ((*args[i])->type == IS_RESOURCE) { /* link or transaction argument */
		get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, args[i], &ib_link, &trans_n, &trans_id);
		i++; /* next arg */
	} else {
		link_id = IBG(default_link);
		ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, dummy, link_id, "InterBase link", le_link, le_plink);
	}

	if ((*args[i])->type == IS_STRING) { /* query argument */
		convert_to_string_ex(args[i]);
		query = (*args[i])->value.str.val;
		i++; /* next arg */
	} else {
		_php_ibase_module_error("query argument missed");
		efree(args);
		RETURN_FALSE;
	}

	if (ZEND_NUM_ARGS() > i) { /* have variables to bind */
		/* XXX Remove or fix??? Variable placeholders and binding makes
		   absolutely no sense if not using a prepared SQL statement.
		*/
		bind_n = ZEND_NUM_ARGS() - i;
		bind_args = args[i];
	}
	
	/* open default transaction */
	if (_php_ibase_def_trans(ib_link, trans_n) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}

	if (_php_ibase_alloc_query(&ib_query, ib_link->link, ib_link->trans[trans_n], query, ib_link->dialect) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}

	if (_php_ibase_exec(&ib_result, ib_query, bind_n, bind_args) == FAILURE) {
		_php_ibase_free_query(ib_query);
		efree(args);
		RETURN_FALSE;
	}
	
	efree(args);
	
	if (ib_result) { /* select statement */
		ib_result->drop_stmt = 1; /* drop stmt when free result */
		ib_query->stmt = NULL; /* keep stmt when free query */
		_php_ibase_free_query(ib_query);
		ZEND_REGISTER_RESOURCE(return_value, ib_result, le_result);
	} else {
		_php_ibase_free_query(ib_query);
		RETURN_TRUE;
	}
}
/* }}} */


/* {{{ _php_ibase_var_pval() */
static int _php_ibase_var_pval(pval *val, void *data, int type, int len, int scale, int flag)
{
	char string_data[255];
	IBLS_FETCH();
	PLS_FETCH();
	
	switch(type & ~1) {
		case SQL_VARYING:
			len = ((IBASE_VCHAR *) data)->var_len;
			data = ((IBASE_VCHAR *) data)->var_str;
			/* fallout */
		case SQL_TEXT:
			val->value.str.val = (char *)emalloc(sizeof(char)*(len+1));
			memcpy(val->value.str.val, data, len);
			val->value.str.val[len] = '\0';
			if (PG(magic_quotes_runtime)) {
				val->value.str.val = php_addslashes(val->value.str.val, len, &len, 1);
            }
			val->type = IS_STRING;
			val->value.str.len = len;
			break;
		case SQL_LONG:
			if (scale) {
				int j, f = 1;
				float n = (float) *(long *)(data);
				
				for (j = 0; j < -scale; j++) {
					f *= 10;
				}
				val->type = IS_STRING;
				val->value.str.len = sprintf(string_data, "%.*f", -scale, n / f);
				val->value.str.val = estrdup(string_data);
			} else {
				val->type = IS_LONG;
				val->value.lval = *(long *)(data);
			}
			break;
		case SQL_SHORT:
			val->type = IS_LONG;
			val->value.lval = *(short *)(data);
			break;
		case SQL_FLOAT:
			val->type = IS_DOUBLE;
			val->value.dval = *(float *)(data);
			break;
		case SQL_DOUBLE:
			if (scale) {
				val->type = IS_STRING;
				val->value.str.len = sprintf(string_data, "%.*f", -scale, *(double *)data);
				val->value.str.val = estrdup(string_data);
				/*
				val->value.str.val = string_data;
				*/
			} else {
				val->type = IS_DOUBLE;
				val->value.dval = *(double *)data;
			}
			break;
#ifdef SQL_INT64
		case SQL_INT64:
			val->type = IS_STRING;
			val->value.str.len = sprintf(string_data, "%Ld.%0*Ld",
										 (ISC_INT64) (*((ISC_INT64 *)data) / (int) pow(10.0, (double) -scale)), -scale,
										 (ISC_INT64) abs((int) (*((ISC_INT64 *)data) % (int) pow(10.0, (double) -scale))));
			val->value.str.val = estrdup(string_data);
			break;
#endif
#ifndef SQL_TIMESTAMP
		case SQL_DATE:
#else
		case SQL_TIMESTAMP:
		case SQL_TYPE_DATE:
		case SQL_TYPE_TIME:
#endif
		{
			struct tm t;
			char *format = NULL;
			long timestamp = -1;

#ifndef SQL_TIMESTAMP
			isc_decode_date((ISC_QUAD *) data, &t);
			format = IBG(timestampformat);
#else
			switch (type & ~1) {
				case SQL_TIMESTAMP:
					isc_decode_timestamp((ISC_TIMESTAMP *) data, &t);
					format = IBG(timestampformat);
					break;
				case SQL_TYPE_DATE:
					isc_decode_sql_date((ISC_DATE *) data, &t);
					format = IBG(dateformat);
					break;
				case SQL_TYPE_TIME:
					isc_decode_sql_time((ISC_TIME *) data, &t);
					format = IBG(timeformat);
					break;
			}
#endif
			/*
			  XXX - Might have to remove this later - seems that isc_decode_date()
			   always sets tm_isdst to 0, sometimes incorrectly (InterBase 6 bug?)
			*/
			t.tm_isdst = -1;
			timestamp = mktime(&t);
#if HAVE_TM_ZONE
			t.tm_zone = tzname[0];
#endif
			if (flag & PHP_IBASE_UNIXTIME) {
				val->type = IS_LONG;
				val->value.lval = timestamp;
			} else {
				val->type = IS_STRING;
#if HAVE_STRFTIME
				val->value.str.len = strftime(string_data, sizeof(string_data), format, &t);
#else
				/* FIXME */
				if (!t.tm_hour && !t.tm_min && !t.tm_sec)
					val->value.str.len = sprintf(string_data, "%02d/%02d/%4d", t.tm_mon+1, t.tm_mday, t.tm_year+1900);
				else
					val->value.str.len = sprintf(string_data, "%02d/%02d/%4d %02d:%02d:%02d",
									 t.tm_mon+1, t.tm_mday, t.tm_year+1900, t.tm_hour, t.tm_min, t.tm_sec);
#endif
				val->value.str.val = estrdup(string_data);
				/*
				val->value.str.val = string_data;
				*/
				break;
			}
		}
		default:
			return FAILURE;
	} /* switch (type) */
	return SUCCESS;
}
/* }}}	*/

/* {{{ _php_ibase_arr_pval() */
/* create multidimension array - resursion function
 (*datap) argument changed */
static int _php_ibase_arr_pval(pval *ar_pval, char **datap, ibase_array *ib_array, int dim, int flag)
{
	pval tmp;
	int i, dim_len, l_bound, u_bound;
	

	if (dim > 16) { /* InterBase limit */
		_php_ibase_module_error("php module internal error in %s %d",__FILE__,__LINE__);
		return FAILURE;
	}

	u_bound = ib_array->ar_desc.array_desc_bounds[dim].array_bound_upper;
	l_bound = ib_array->ar_desc.array_desc_bounds[dim].array_bound_lower;
	dim_len = 1 + u_bound - l_bound;
		
	if (dim < ib_array->ar_desc.array_desc_dimensions - 1) { /* array again */
		for (i = 0; i < dim_len; i++) {
			/* recursion here */
			if (_php_ibase_arr_pval(ar_pval, datap, ib_array, dim+1, flag) == FAILURE) {
				return FAILURE;
			}
		}
	} else { /* data at last */
		
		array_init(ar_pval);

		for (i = 0; i < dim_len; i++) {
			if (_php_ibase_var_pval(&tmp, *datap, ib_array->el_type,
									ib_array->ar_desc.array_desc_length,
									ib_array->ar_desc.array_desc_scale,
									flag) == FAILURE){
				return FAILURE;
			}
			/* FIXME ??? */
			zend_hash_index_update(ar_pval->value.ht,
								   l_bound + i,
								   (void *) &tmp, sizeof(pval),NULL);
			*datap += ib_array->el_size;
		}
	}
	return SUCCESS;
}


/* {{{ _php_ibase_fetch_hash() */

#define FETCH_ARRAY 1
#define FETCH_OBJECT 2

static void _php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAMETERS, int fetch_type)
{
	zval **result_arg, **flag_arg;
	long flag = 0;
	int i, arr_cnt;
	ibase_result *ib_result;
	XSQLVAR *var;
	IBLS_FETCH();
	
	RESET_ERRMSG;
	
	switch(ZEND_NUM_ARGS()) {
		case 1:
			if (ZEND_NUM_ARGS()==1 && zend_get_parameters_ex(1, &result_arg)==FAILURE) {
				RETURN_FALSE;
			}
			break;
		case 2:
			if (ZEND_NUM_ARGS()==2 && zend_get_parameters_ex(2, &result_arg, &flag_arg)==FAILURE) {
				RETURN_FALSE;
			}
			convert_to_long_ex(flag_arg);
			flag = (*flag_arg)->value.lval;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}

	ZEND_FETCH_RESOURCE(ib_result, ibase_result *, result_arg, -1, "InterBase result", le_result);

	if (ib_result->out_sqlda == NULL) {
		_php_ibase_module_error("trying to fetch results from a non-select query");
		RETURN_FALSE;
	}
	
	if (isc_dsql_fetch(IB_STATUS, &ib_result->stmt, 1, ib_result->out_sqlda) == 100L) {
		RETURN_FALSE;  /* end of cursor */
	}
	
	if (IB_STATUS[0] && IB_STATUS[1]) { /* error in fetch */
		_php_ibase_error();
		RETURN_FALSE;
	}
	
	if (fetch_type == FETCH_ARRAY) {
		if (array_init(return_value)==FAILURE) {
			RETURN_FALSE;
		}
	} else if (fetch_type == FETCH_OBJECT) {
		if (object_init(return_value)==FAILURE) {
			RETURN_FALSE;
		}
	}
	
	arr_cnt = 0;
	var = ib_result->out_sqlda->sqlvar;
	for (i = 0; i < ib_result->out_sqlda->sqld; i++, var++) {
		if (((var->sqltype & 1) == 0) || *var->sqlind != -1) {
			pval *tmp;
			tmp = emalloc(sizeof(pval));
			switch(var->sqltype & ~1) {
				case SQL_VARYING:
				case SQL_TEXT:
				case SQL_SHORT:
				case SQL_LONG:
				case SQL_FLOAT:
				case SQL_DOUBLE:
#ifdef SQL_INT64
				case SQL_INT64:
#endif
#ifndef SQL_TIMESTAMP
				case SQL_DATE:
#else
				case SQL_TIMESTAMP:
				case SQL_TYPE_DATE:
				case SQL_TYPE_TIME:
#endif
					_php_ibase_var_pval(tmp, var->sqldata, var->sqltype, var->sqllen, var->sqlscale, flag);
					break;
				case SQL_BLOB:
					if (flag & PHP_IBASE_TEXT) { /* text ? */
						int stat;
						isc_blob_handle bl_handle = NULL;
						ISC_LONG max_len = 0, cur_len = 0;
						char bl_items[1], *bl_data, bl_info[20], *p;
						
						if (isc_open_blob(IB_STATUS, &ib_result->link, &ib_result->trans, &bl_handle, (ISC_QUAD ISC_FAR *) var->sqldata)) {
							_php_ibase_error();
							RETURN_FALSE;
						}
						
						bl_items[0] = isc_info_blob_total_length;
						if (isc_blob_info(IB_STATUS,&bl_handle,sizeof(bl_items),bl_items,sizeof(bl_info),bl_info)) {
							_php_ibase_error();
							RETURN_FALSE;
						}
						
						/* find total length of blob's data */
						for (p = bl_info; *p != isc_info_end && p < bl_info+sizeof(bl_info);) {
							unsigned short item_len, item = *p++;

							item_len = (short) isc_vax_integer(p, 2);
							p += 2;
							if (item == isc_info_blob_total_length)
								max_len = isc_vax_integer(p, item_len);
							p += item_len;
						}
						
						bl_data = emalloc(max_len+1);
						
						for (cur_len = stat = 0; stat == 0 && cur_len < max_len; ) {
							unsigned short seg_len;
							unsigned short max_seg = (unsigned short) (max_len - cur_len > USHRT_MAX ? USHRT_MAX : max_len-cur_len);
							stat = isc_get_segment(IB_STATUS, &bl_handle, &seg_len, max_seg, &bl_data[cur_len]);
							cur_len += seg_len;
							if (cur_len > max_len) { /* never!*/
								efree(bl_data);
								_php_ibase_module_error("php module internal error in %s %d",__FILE__,__LINE__);
								RETURN_FALSE;
							}
						}
						
						if (IB_STATUS[0] && IB_STATUS[1] && (IB_STATUS[1] != isc_segstr_eof)) {
							efree(bl_data);
							_php_ibase_error();
							RETURN_FALSE;
						}
						bl_data[cur_len] = '\0';
						if (isc_close_blob(IB_STATUS, &bl_handle)) {
							efree(bl_data);
							_php_ibase_error();
							RETURN_FALSE;
						}
						tmp->type = IS_STRING;
						tmp->value.str.len = cur_len;
						tmp->value.str.val = estrndup(bl_data, cur_len);
						efree(bl_data);
					} else { /* blob id only */
						ISC_QUAD *bl_qd = (ISC_QUAD ISC_FAR *) var->sqldata;
						ibase_blob_handle *ib_blob_id;
						
						ib_blob_id = (ibase_blob_handle *) emalloc(sizeof(ibase_blob_handle));
						
						ib_blob_id->link = ib_result->link;
						ib_blob_id->trans_handle = ib_result->trans;
						ib_blob_id->bl_qd.gds_quad_high = bl_qd->gds_quad_high;
						ib_blob_id->bl_qd.gds_quad_low = bl_qd->gds_quad_low;
						ib_blob_id->bl_handle = NULL;
						
						tmp->type = IS_STRING;
						tmp->value.str.len = sizeof(ibase_blob_handle);
						tmp->value.str.val = estrndup((char *)ib_blob_id, sizeof(ibase_blob_handle));
						efree(ib_blob_id);
					}
					break;
				case SQL_ARRAY:{
					ISC_QUAD ar_qd = *(ISC_QUAD ISC_FAR *) var->sqldata;
					ibase_array *ib_array = &ib_result->out_array[arr_cnt];
					void *ar_data;
					char *tmp_ptr;
					
					ar_data = emalloc(ib_array->ar_size);
					
					if (isc_array_get_slice(IB_STATUS, &ib_result->link, &ib_result->trans,
											&ar_qd, &ib_array->ar_desc, ar_data, &ib_array->ar_size)) {
						_php_ibase_error();
						RETURN_FALSE;
					}
					
					tmp_ptr = ar_data; /* avoid changes in _arr_pval */
					if (_php_ibase_arr_pval(tmp, &tmp_ptr, ib_array, 0, flag) == FAILURE) {
						RETURN_FALSE;
					}
					efree(ar_data);
				}
				break;
				default:
					break;
			} /*switch*/
			if (fetch_type & FETCH_ARRAY) {
				switch (tmp->type) {
				case IS_STRING:
					add_index_stringl(return_value, i, tmp->value.str.val, tmp->value.str.len, 0);
					break;
				case IS_LONG:
					add_index_long(return_value, i, tmp->value.lval);
					break;
				case IS_DOUBLE:
					add_index_double(return_value, i, tmp->value.dval);
					break;
				}
			} else {
				switch (tmp->type) {
				case IS_STRING:
					add_property_stringl(return_value, var->aliasname, tmp->value.str.val, tmp->value.str.len, 0);
break;
				case IS_LONG:
					add_property_long(return_value, var->aliasname, tmp->value.lval);
					break;
				case IS_DOUBLE:
					add_property_double(return_value, var->aliasname, tmp->value.dval);
					break;
				}
			}
			efree(tmp);
		} /* if not null */
		if ((var->sqltype & ~1) == SQL_ARRAY) {
			arr_cnt++;
		}
	} /*for field*/
}
/* }}} */


/* {{{ proto array ibase_fetch_row(int result [, int blob_flag])
   Fetch a row  from the results of a query */
PHP_FUNCTION(ibase_fetch_row)
{
	_php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_ARRAY);
}
/* }}} */


/* {{{ proto object ibase_fetch_object(int result [, int blob_flag])
   Fetch a object from the results of a query */
PHP_FUNCTION(ibase_fetch_object)
{
	_php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_OBJECT);
}
/* }}} */


/* {{{ proto int ibase_free_result(int result)
   Free the memory used by a result */
PHP_FUNCTION(ibase_free_result)
{
	zval **result_arg;
	ibase_result *ib_result;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=1 || zend_get_parameters_ex(1, &result_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ZEND_FETCH_RESOURCE(ib_result, ibase_result *, result_arg, -1, "InterBase result", le_result);
	zend_list_delete((*result_arg)->value.lval);
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int ibase_prepare([int link_identifier,] string query)
   Prepare a query for later execution */
PHP_FUNCTION(ibase_prepare)
{
	zval **link_arg, **query_arg;
	int link_id, trans_n = 0, trans_id = 0;
	ibase_db_link *ib_link;
	ibase_query *ib_query;
	char *query;
	IBLS_FETCH();

	RESET_ERRMSG;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_get_parameters_ex(1, &query_arg) == FAILURE) {
				RETURN_FALSE;
			}
			link_id = IBG(default_link);
			ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, NULL, link_id, "InterBase link", le_link, le_plink);
			break;
		case 2:
			if (zend_get_parameters_ex(2, &link_arg, &query_arg) == FAILURE) {
				RETURN_FALSE;
			}
			get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, link_arg, &ib_link, &trans_n, &trans_id);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	convert_to_string_ex(query_arg);
	query = (*query_arg)->value.str.val;

	/* open default transaction */
	if (_php_ibase_def_trans(ib_link, trans_n) == FAILURE) {
		RETURN_FALSE;
	}
	
	if (_php_ibase_alloc_query(&ib_query, ib_link->link, ib_link->trans[trans_n],  query, ib_link->dialect) == FAILURE) {
		RETURN_FALSE;
	}
	ib_query->cursor_open = 0;

	zend_list_addref(link_id);

	ZEND_REGISTER_RESOURCE(return_value, ib_query, le_query);

}
/* }}} */


/* {{{ proto int ibase_execute(int query [, int bind_args [, int ...]])
   Execute a previously prepared query */
PHP_FUNCTION(ibase_execute)
{
	pval ***args, **bind_args = NULL;
	ibase_query *ib_query;
	ibase_result *ib_result;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS() < 1) {
		WRONG_PARAM_COUNT;
	}

	args = (pval ***)emalloc(ZEND_NUM_ARGS() * sizeof(pval **));
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(ib_query, ibase_query *, args[0], -1, "InterBase query", le_query);

	if (ZEND_NUM_ARGS() > 1) { /* have variables to bind */
		bind_args = args[1];
	}
	
	/* Have we used this cursor before and it's still open? */
	if (ib_query->cursor_open) {
		IBDEBUG("Implicitly closing a cursor");
		if (isc_dsql_free_statement(IB_STATUS, &ib_query->stmt, DSQL_close)){
			efree(args);
			_php_ibase_error();
		}
	}

	if ( _php_ibase_exec(&ib_result, ib_query, ZEND_NUM_ARGS()-1, bind_args) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}
	
	efree(args);

	if (ib_result) { /* select statement */
		ib_query->cursor_open = 1;
		ZEND_REGISTER_RESOURCE(return_value, ib_result, le_result);
	} else {
		ib_query->cursor_open = 0;
		RETURN_TRUE;
	}
}
/* }}} */


/* {{{ proto int ibase_free_query(int query)
   Free memory used by a query */
PHP_FUNCTION(ibase_free_query)
{
	pval **query_arg;
	ibase_query *ib_query;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=1 || zend_get_parameters_ex(1, &query_arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ZEND_FETCH_RESOURCE(ib_query, ibase_query *, query_arg, -1, "InterBase query", le_query);
	zend_list_delete((*query_arg)->value.lval);
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto int ibase_timefmt(string format)
   Sets the format of timestamp, date and time columns returned from queries */
PHP_FUNCTION(ibase_timefmt)
{
#if HAVE_STRFTIME
	pval ***args;
	char *fmt = NULL;
	int type = PHP_IBASE_TIMESTAMP;
	IBLS_FETCH();
	
	RESET_ERRMSG; /* ??? */


	if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 2){
		WRONG_PARAM_COUNT;
	}
	
	args = (pval ***) emalloc(sizeof(pval **)*ZEND_NUM_ARGS());
	if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		efree(args);
		RETURN_FALSE;
	}

	switch (ZEND_NUM_ARGS()) {
		case 2:
			convert_to_long_ex(args[1]);
			type = (*args[1])->value.lval;
		case 1:
			convert_to_string_ex(args[0]);
			fmt = (*args[0])->value.str.val;
	}

	switch (type) {
		case PHP_IBASE_TIMESTAMP:
			if (IBG(timestampformat))
				DL_FREE(IBG(timestampformat));
			IBG(timestampformat) = DL_STRDUP(fmt);
			break;
		case PHP_IBASE_DATE:
			if (IBG(dateformat))
				DL_FREE(IBG(dateformat));
			IBG(dateformat) = DL_STRDUP(fmt);
			break;
		case PHP_IBASE_TIME:
			if (IBG(timeformat))
				DL_FREE(IBG(timeformat));
			IBG(timeformat) = DL_STRDUP(fmt);
			break;
	}
    
	efree(args);
	RETURN_TRUE;
#else
	_php_ibase_module_error("ibase_timefmt not supported on this platform");
	RETURN_FALSE;
#endif
}
/* }}} */


/* {{{ proto int ibase_num_fields(int result)
   Get the number of fields in result */
PHP_FUNCTION(ibase_num_fields)
{
	pval **result;
	ibase_result *ib_result;
	IBLS_FETCH();	

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=1 || zend_get_parameters_ex(1, &result)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	ZEND_FETCH_RESOURCE(ib_result, ibase_result *, result, -1, "InterBase result", le_result);

	if (ib_result->out_sqlda == NULL) {
		_php_ibase_module_error("trying to get num fields from a non-select query");
		RETURN_FALSE;
	}

	RETURN_LONG(ib_result->out_sqlda->sqld);
}
/* }}} */


/* {{{ proto array ibase_field_info(int result, int field_number)
   Get information about a field */
PHP_FUNCTION(ibase_field_info)
{
	pval *ret_val;
	zval **result_arg, **field_arg;
	ibase_result *ib_result;
	char buf[30], *s;
	int len;
	XSQLVAR *var;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=2 || zend_get_parameters_ex(2, &result_arg, &field_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ZEND_FETCH_RESOURCE(ib_result, ibase_result *, result_arg, -1, "InterBase result", le_result);

	if (ib_result->out_sqlda == NULL) {
		_php_ibase_module_error("trying to get field info from a non-select query");
		RETURN_FALSE;
	}

	convert_to_long_ex(field_arg);

	if ((*field_arg)->value.lval<0 || (*field_arg)->value.lval>=ib_result->out_sqlda->sqld)
		RETURN_FALSE;

	if (array_init(return_value)==FAILURE)
		RETURN_FALSE;

	var = ib_result->out_sqlda->sqlvar + (*field_arg)->value.lval;

	add_get_index_stringl(return_value, 0, var->sqlname, var->sqlname_length, (void **) &ret_val, 1);
	add_assoc_stringl(return_value, "name", var->sqlname, var->sqlname_length, 1);

	add_get_index_stringl(return_value, 1, var->aliasname, var->aliasname_length, (void **) &ret_val, 1);
	add_assoc_stringl(return_value, "alias", var->aliasname, var->aliasname_length, 1);

	add_get_index_stringl(return_value, 2, var->relname, var->relname_length, (void **) &ret_val, 1);
	add_assoc_stringl(return_value, "relation", var->relname, var->relname_length, 1);

	len = sprintf(buf, "%d", var->sqllen);
	add_get_index_stringl(return_value, 3, buf, len, (void **) &ret_val, 1);
	add_assoc_stringl(return_value, "length", buf, len, 1);

	switch (var->sqltype & ~1) {
		case SQL_TEXT:	    s = "TEXT"; break;
		case SQL_VARYING:   s = "VARYING"; break;
		case SQL_SHORT:	    s = "SHORT"; break;
		case SQL_LONG:	    s = "LONG"; break;
		case SQL_FLOAT:	    s = "FLOAT"; break;
		case SQL_DOUBLE:    s = "DOUBLE"; break;
		case SQL_D_FLOAT:   s = "D_FLOAT"; break;
#ifdef SQL_INT64
		case SQL_INT64:     s = "INT64"; break;
#endif
#ifdef SQL_TIMESTAMP
		case SQL_TIMESTAMP:	s = "TIMESTAMP"; break;
		case SQL_TYPE_DATE:	s = "DATE"; break;
		case SQL_TYPE_TIME:	s = "TIME"; break;
#else
		case SQL_DATE:	    s = "DATE"; break;
#endif
		case SQL_BLOB:	    s = "BLOB"; break;
		case SQL_ARRAY:	    s = "ARRAY"; break;
		case SQL_QUAD:	    s = "QUAD"; break;
	default:
		sprintf(buf,"unknown (%d)", var->sqltype & ~1);
		s = buf;
		break;
	}
	add_get_index_stringl(return_value, 4, s, strlen(s), (void **) &ret_val, 1);
	add_assoc_stringl(return_value, "type", s, strlen(s), 1);
}
/* }}} */


/* blobs ----------------------------------- */


/* {{{ _php_ibase_blob_info(isc_blob_handle bl_handle,IBASE_BLOBINFO *bl_info) */
static int _php_ibase_blob_info(isc_blob_handle bl_handle,IBASE_BLOBINFO *bl_info)
{
	
	static char bl_items[] = {
		isc_info_blob_num_segments,
		isc_info_blob_max_segment,
		isc_info_blob_total_length,
		isc_info_blob_type
	};
	
	char bl_inf[sizeof(long)*8], *p;
	IBLS_FETCH();

	bl_info->max_segment = 0;
	bl_info->num_segments = 0;
	bl_info->total_length = 0;
	bl_info->bl_stream = 0;

	if (isc_blob_info(IB_STATUS,&bl_handle,sizeof(bl_items),bl_items,sizeof(bl_inf),bl_inf)) {
		_php_ibase_error();
		return FAILURE;
	}

	for (p = bl_inf; *p != isc_info_end && p < bl_inf+sizeof(bl_inf);) {
		unsigned short item_len;
		int item = *p++;

		item_len = (short)isc_vax_integer(p, 2);
		p += 2;
		switch (item) {
			case isc_info_blob_num_segments:
				bl_info->num_segments = isc_vax_integer(p, item_len);
				break;
			case isc_info_blob_max_segment:
				bl_info->max_segment = isc_vax_integer(p, item_len);
				break;
			case isc_info_blob_total_length:
				bl_info->total_length = isc_vax_integer(p, item_len);
				break;
			case isc_info_blob_type:
				bl_info->bl_stream = isc_vax_integer(p, item_len);
				break;
			case isc_info_end:
				break;
			case isc_info_truncated:
			case isc_info_error:  /* hmm. don't think so...*/
				_php_ibase_module_error("php module internal error in %s %d",__FILE__,__LINE__);
				return FAILURE;
		} /*switch*/
		p += item_len;
	} /*for*/
	return SUCCESS;
}
/* }}} */


/* {{{ proto int ibase_blob_create([int link_identifier])
   Create blob for adding data */
PHP_FUNCTION(ibase_blob_create)
{
	zval **link_arg;
	int trans_n = 0, trans_id = 0, link_id;
	ibase_db_link *ib_link;
	ibase_blob_handle *ib_blob;
	IBLS_FETCH();

	RESET_ERRMSG;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			link_id = IBG(default_link);
			ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, NULL, link_id, "InterBase link", le_link, le_plink);
			break;
		case 1:
			if (zend_get_parameters_ex(1, &link_arg) == FAILURE) {
				RETURN_FALSE;
			}
			get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, link_arg, &ib_link, &trans_n, &trans_id);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	/* open default transaction */
	if(_php_ibase_def_trans(ib_link, trans_n) == FAILURE){
		RETURN_FALSE;
	}
	
	ib_blob = (ibase_blob_handle *) emalloc(sizeof(ibase_blob_handle));
	ib_blob->trans_handle = ib_link->trans[trans_n];
	ib_blob->link = ib_link->link;
	ib_blob->bl_handle = NULL;
	
	if (isc_create_blob(IB_STATUS, &ib_blob->link, &ib_blob->trans_handle, &ib_blob->bl_handle,&ib_blob->bl_qd)) {
		efree(ib_blob);
		_php_ibase_error();
		RETURN_FALSE;
	}
	
	RETURN_LONG(zend_list_insert(ib_blob, le_blob));
}
/* }}} */


/* {{{ proto int ibase_blob_open(string blob_id)
   Open blob for retriving data parts */
PHP_FUNCTION(ibase_blob_open)
{
	pval *blob_arg;
	ibase_blob_handle *ib_blob, *ib_blob_id;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=1 || getParameters(ht, 1, &blob_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	ib_blob = (ibase_blob_handle *) emalloc(sizeof(ibase_blob_handle));

	GET_BLOB_ID_ARG(blob_arg,ib_blob_id);

	if (ib_blob_id == NULL) { /* blob IS NULL or argument unset */
		RETURN_FALSE;
	}
	
	ib_blob->link = ib_blob_id->link;
	ib_blob->trans_handle = ib_blob_id->trans_handle;
	ib_blob->bl_qd.gds_quad_high = ib_blob_id->bl_qd.gds_quad_high;
	ib_blob->bl_qd.gds_quad_low = ib_blob_id->bl_qd.gds_quad_low;
	ib_blob->bl_handle = NULL;
	if (isc_open_blob(IB_STATUS, &ib_blob->link, &ib_blob->trans_handle, &ib_blob->bl_handle, &ib_blob->bl_qd)) {
		efree(ib_blob);
		_php_ibase_error();
		RETURN_FALSE;
	}
	
	RETURN_LONG(zend_list_insert(ib_blob, le_blob));
}
/* }}} */


/* {{{ proto int ibase_blob_add(int blob_id, string data)
   Add data into created blob */
PHP_FUNCTION(ibase_blob_add)
{
	pval *blob_arg,*string_arg;
	ibase_blob_handle *ib_blob;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=2 || getParameters(ht, 2, &blob_arg, &string_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	GET_BLOB_HANDLE_ARG(blob_arg, ib_blob);
	
	convert_to_string(string_arg);

	if (isc_put_segment(IB_STATUS, &ib_blob->bl_handle, (unsigned short) string_arg->value.str.len, string_arg->value.str.val)) {
		_php_ibase_error();
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */


/* {{{ proto string ibase_blob_get(int blob_id, int len)
   Get len bytes data from open blob */
PHP_FUNCTION(ibase_blob_get)
{
	pval *blob_arg, *len_arg;
	int stat;
	char *bl_data;
	unsigned short max_len = 0, cur_len, seg_len;
	ibase_blob_handle *ib_blob;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS() != 2 || getParameters(ht, 2, &blob_arg, &len_arg) == FAILURE) {
		WRONG_PARAM_COUNT;
	}

	convert_to_long(len_arg);
	max_len = (unsigned short) len_arg->value.lval;

	GET_BLOB_HANDLE_ARG(blob_arg, ib_blob);

	if (ib_blob->bl_qd.gds_quad_high || ib_blob->bl_qd.gds_quad_low) { /*not null ?*/
		
		bl_data = emalloc(max_len+1);

		for (cur_len = stat = 0; stat == 0; ) {
			stat = isc_get_segment(IB_STATUS, &ib_blob->bl_handle, &seg_len, (unsigned short) (max_len-cur_len), &bl_data[cur_len]);
			cur_len += seg_len;
			if (cur_len > max_len) { /* never!*/
				efree(bl_data);
				_php_ibase_module_error("php module internal error in %s %d",__FILE__,__LINE__);
				RETURN_FALSE;
			}
		}

		bl_data[cur_len] = '\0';
		if (IB_STATUS[0] && (IB_STATUS[1] != isc_segstr_eof && IB_STATUS[1] != isc_segment)) {
			efree(bl_data);
			_php_ibase_error();
			RETURN_FALSE;
		}
		RETURN_STRINGL(bl_data,cur_len,0);
	} else { /* null blob */
		RETURN_STRING("",1); /* empty string */
	}
}
/* }}} */


#define BLOB_CLOSE 1
#define BLOB_CANCEL 2

/* {{{ _php_ibase_blob_end() */
/* Close or Cancel created or Close open blob */
static void _php_ibase_blob_end(INTERNAL_FUNCTION_PARAMETERS, int bl_end)
{
	pval *blob_arg;
	ibase_blob_handle *ib_blob;
	IBLS_FETCH();

	RESET_ERRMSG;
	
	if (ZEND_NUM_ARGS()!=1 || getParameters(ht, 1, &blob_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	GET_BLOB_HANDLE_ARG(blob_arg, ib_blob);

	if (bl_end == BLOB_CLOSE) { /* return id here */
		if (ib_blob->bl_qd.gds_quad_high || ib_blob->bl_qd.gds_quad_low) { /*not null ?*/
			if (isc_close_blob(IB_STATUS, &ib_blob->bl_handle)) {
				_php_ibase_error();
				RETURN_FALSE;
			}
		}
		ib_blob->bl_handle = NULL;
		RETVAL_STRINGL((char *)ib_blob, sizeof(ibase_blob_handle), 1);
		zend_list_delete(blob_arg->value.lval);
	} else { /* discard created blob */
		if (isc_cancel_blob(IB_STATUS, &ib_blob->bl_handle)) {
			_php_ibase_error();
			RETURN_FALSE;
		}
		ib_blob->bl_handle = NULL;
		zend_list_delete(blob_arg->value.lval);
		RETURN_TRUE;
	}
}
/* }}} */


/* {{{ proto int ibase_blob_close(int blob_id)
   Close blob */
PHP_FUNCTION(ibase_blob_close)
{
	_php_ibase_blob_end(INTERNAL_FUNCTION_PARAM_PASSTHRU,BLOB_CLOSE);
}
/* }}} */


/* {{{ proto int ibase_blob_cancel(int blob_id)
   Cancel creating blob */
PHP_FUNCTION(ibase_blob_cancel)
{
	_php_ibase_blob_end(INTERNAL_FUNCTION_PARAM_PASSTHRU,BLOB_CANCEL);
}
/* }}} */


/* {{{ proto object ibase_blob_info(string blob_id_str)
   Return blob length and other useful info */
PHP_FUNCTION(ibase_blob_info)
{
	pval *blob_arg, *result_var;
	ibase_blob_handle *ib_blob_id;
	IBASE_BLOBINFO bl_info;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=1 || getParameters(ht, 1, &blob_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}
	
	GET_BLOB_ID_ARG(blob_arg, ib_blob_id);

	if (array_init(return_value)==FAILURE){
		RETURN_FALSE;
	}
	
	if (ib_blob_id->bl_qd.gds_quad_high || ib_blob_id->bl_qd.gds_quad_low) { /*not null ?*/
		if (isc_open_blob(IB_STATUS, &ib_blob_id->link, &ib_blob_id->trans_handle,
						  &ib_blob_id->bl_handle, &ib_blob_id->bl_qd)) {
			_php_ibase_error();
			RETURN_FALSE;
		}

		if (_php_ibase_blob_info(ib_blob_id->bl_handle,&bl_info)) {
			RETURN_FALSE;
		}
		if (isc_close_blob(IB_STATUS, &ib_blob_id->bl_handle)) {
			_php_ibase_error();
			RETURN_FALSE;
		}
		ib_blob_id->bl_handle = NULL;
	} else { /* null blob, all values to zero	 */
		bl_info.max_segment = 0;
		bl_info.num_segments = 0;
		bl_info.total_length = 0;
		bl_info.bl_stream = 0;
	}

	/* FIXME */
	add_get_index_long(return_value, 0, bl_info.total_length, (void **)&result_var);
	/*
	zend_hash_pointer_update(return_value->value.ht, "length", sizeof("length"), result_var);
	*/

	add_get_index_long(return_value, 1, bl_info.num_segments, (void **)&result_var);
	/*
	zend_hash_pointer_update(return_value->value.ht, "numseg", sizeof("numseg"), result_var);
	*/

	add_get_index_long(return_value, 2, bl_info.max_segment, (void **)&result_var);
	/*
	zend_hash_pointer_update(return_value->value.ht, "maxseg", sizeof("maxseg"), result_var);
	*/

	add_get_index_long(return_value, 3, bl_info.bl_stream, (void **)&result_var);
	/*
	zend_hash_pointer_update(return_value->value.ht, "stream", sizeof("stream"), result_var);
	*/
	
	add_get_index_long(return_value, 4,
					   (!ib_blob_id->bl_qd.gds_quad_high
						&& !ib_blob_id->bl_qd.gds_quad_low),
					   (void **)&result_var);
	/*
	zend_hash_pointer_update(return_value->value.ht, "isnull", sizeof("isnull"), result_var);
	*/
}
/* }}} */


/* {{{ proto int ibase_blob_echo(string blob_id_str)
   Output blob contents to browser */
PHP_FUNCTION(ibase_blob_echo)
{
	pval *blob_arg;
	char bl_data[IBASE_BLOB_SEG];
	unsigned short seg_len;
	ibase_blob_handle *ib_blob_id;
	IBLS_FETCH();

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS()!=1 || getParameters(ht, 1, &blob_arg)==FAILURE) {
		WRONG_PARAM_COUNT;
	}

	GET_BLOB_ID_ARG(blob_arg, ib_blob_id);
	
	if (ib_blob_id) { /*not null ?*/
		
		if (isc_open_blob(IB_STATUS, &ib_blob_id->link, &ib_blob_id->trans_handle,
						  &ib_blob_id->bl_handle,&ib_blob_id->bl_qd)) {
			_php_ibase_error();
			RETURN_FALSE;
		}

		while (!isc_get_segment(IB_STATUS, &ib_blob_id->bl_handle, &seg_len, sizeof(bl_data), bl_data)
			   || IB_STATUS[1] == isc_segment) {
			PHPWRITE(bl_data,seg_len);
		}
	
		if (IB_STATUS[0] && (IB_STATUS[1] != isc_segstr_eof)) {
			_php_ibase_error();
			RETURN_FALSE;
		}

		if (isc_close_blob(IB_STATUS, &ib_blob_id->bl_handle)) {
			_php_ibase_error();
			RETURN_FALSE;
		}
		ib_blob_id->bl_handle = NULL;
	} /* not null ? */

	RETURN_TRUE;
}
/* }}} */

/*
extern int le_fp, le_pp;
extern int wsa_fp;
*/
/*to handle reading and writing to windows sockets*/

/* {{{ proto string ibase_blob_import([link_identifier,] int file_id)
   Create blob, copy file in it, and close it */

PHP_FUNCTION(ibase_blob_import)
{
	zval **link_arg, **file_arg;
	int trans_n = 0, link_id = 0, trans_id = 0, size;
	unsigned short b;
	int issock=0, socketd=0;
	ibase_blob_handle ib_blob;
	ibase_db_link *ib_link;
	char bl_data[IBASE_BLOB_SEG]; /* FIXME? blob_seg_size parameter?	 */
	FILE *fp;
	IBLS_FETCH();

	RESET_ERRMSG;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_get_parameters_ex(1, &file_arg) == FAILURE) {
				RETURN_FALSE;
			}
			link_id = IBG(default_link);
			ZEND_FETCH_RESOURCE2(ib_link, ibase_db_link *, NULL, link_id, "InterBase link", le_link, le_plink);
			break;
		case 2:
			if (zend_get_parameters_ex(2, &link_arg, &file_arg) == FAILURE) {
				RETURN_FALSE;
			}
			get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, link_arg, &ib_link, &trans_n, &trans_id);
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	/* open default transaction */
	if (_php_ibase_def_trans(ib_link, trans_n) == FAILURE) {
		RETURN_FALSE;
	}

	ZEND_FETCH_RESOURCE(fp, FILE *, file_arg, -1, "File-Handle", php_file_le_fopen());

	ib_blob.link = ib_link->link;
	ib_blob.trans_handle = ib_link->trans[trans_n];
	ib_blob.bl_handle = NULL;
	ib_blob.bl_qd.gds_quad_high = 0;
	ib_blob.bl_qd.gds_quad_low = 0;
	
	if (isc_create_blob(IB_STATUS, &ib_blob.link, &ib_blob.trans_handle, &ib_blob.bl_handle,&ib_blob.bl_qd)) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	size = 0;
	while (issock?(b=SOCK_FREAD(bl_data,sizeof(bl_data),socketd)):(b = fread(bl_data, 1, sizeof(bl_data), fp)) > 0) {
		if (isc_put_segment(IB_STATUS, &ib_blob.bl_handle, b, bl_data)) {
			_php_ibase_error();
			RETURN_FALSE;
		}
		size += b;
	}
	
	if (isc_close_blob(IB_STATUS, &ib_blob.bl_handle)) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	ib_blob.bl_handle = NULL;
	RETVAL_STRINGL((char *)&ib_blob, sizeof(ibase_blob_handle), 1);
}
/* }}} */

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */

