/*
   +----------------------------------------------------------------------+
   | PHP Version 4                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2003 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.0 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_0.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jouni Ahto <jouni.ahto@exdec.fi>                            |
   |          Andrew Avdeev <andy@simgts.mv.ru>                           |
   |          Ard Biesheuvel <a.k.biesheuvel@its.tudelft.nl>              |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifndef PHP_INTERBASE_H
#define PHP_INTERBASE_H

#if HAVE_IBASE

#include <ibase.h>

extern zend_module_entry ibase_module_entry;
#define phpext_interbase_ptr &ibase_module_entry

#ifndef ISC_INT64_FORMAT
#ifdef PHP_WIN32
#define ISC_INT64_FORMAT "I64"
#else
#define ISC_INT64_FORMAT "ll"
#endif
#endif

PHP_MINIT_FUNCTION(ibase);
PHP_RINIT_FUNCTION(ibase);
PHP_MSHUTDOWN_FUNCTION(ibase);
PHP_RSHUTDOWN_FUNCTION(ibase);
PHP_MINFO_FUNCTION(ibase);

PHP_FUNCTION(ibase_connect);
PHP_FUNCTION(ibase_pconnect);
PHP_FUNCTION(ibase_close);
PHP_FUNCTION(ibase_drop_db);
PHP_FUNCTION(ibase_query);
PHP_FUNCTION(ibase_fetch_row);
PHP_FUNCTION(ibase_fetch_assoc);
PHP_FUNCTION(ibase_fetch_object);
PHP_FUNCTION(ibase_free_result);
PHP_FUNCTION(ibase_name_result);
PHP_FUNCTION(ibase_prepare);
PHP_FUNCTION(ibase_execute);
PHP_FUNCTION(ibase_free_query);
#if HAVE_STRFTIME
PHP_FUNCTION(ibase_timefmt);
#endif

PHP_FUNCTION(ibase_num_fields);
PHP_FUNCTION(ibase_num_params);
#if abies_0
PHP_FUNCTION(ibase_num_rows);
#endif
PHP_FUNCTION(ibase_affected_rows);
PHP_FUNCTION(ibase_field_info);
PHP_FUNCTION(ibase_param_info);

PHP_FUNCTION(ibase_trans);
PHP_FUNCTION(ibase_commit);
PHP_FUNCTION(ibase_rollback);
PHP_FUNCTION(ibase_commit_ret);
PHP_FUNCTION(ibase_rollback_ret);

PHP_FUNCTION(ibase_blob_create);
PHP_FUNCTION(ibase_blob_add);
PHP_FUNCTION(ibase_blob_cancel);
PHP_FUNCTION(ibase_blob_open);
PHP_FUNCTION(ibase_blob_get);
PHP_FUNCTION(ibase_blob_close);
PHP_FUNCTION(ibase_blob_echo);
PHP_FUNCTION(ibase_blob_info);
PHP_FUNCTION(ibase_blob_import);
#ifdef SQL_DIALECT_V6
PHP_FUNCTION(ibase_add_user);
PHP_FUNCTION(ibase_modify_user);
PHP_FUNCTION(ibase_delete_user);
#endif
PHP_FUNCTION(ibase_errmsg);
PHP_FUNCTION(ibase_errcode);

PHP_FUNCTION(ibase_wait_event);
PHP_FUNCTION(ibase_set_event_handler);
PHP_FUNCTION(ibase_free_event_handler);

#define IBASE_MSGSIZE 256
#define MAX_ERRMSG (IBASE_MSGSIZE*2)
#define IBASE_BLOB_SEG 4096

ZEND_BEGIN_MODULE_GLOBALS(ibase)
	ISC_STATUS status[20];
	long default_link;
	long num_links, num_persistent;
	long max_links, max_persistent;
	long allow_persistent;
	char *default_user, *default_password;
	char *timestampformat;
	char *cfg_timestampformat;
	char *dateformat;
	char *cfg_dateformat;
	char *timeformat;
	char *cfg_timeformat;
	char errmsg[MAX_ERRMSG];
	long sql_code;
ZEND_END_MODULE_GLOBALS(ibase)

typedef struct {
	isc_db_handle handle;
	struct tr_list *tr_list;
	unsigned short dialect;
} ibase_db_link;

typedef struct {
	isc_tr_handle handle;
	unsigned short link_cnt;
	unsigned long affected_rows;
	ibase_db_link *db_link[1];
} ibase_trans;

typedef struct tr_list {
	ibase_trans *trans;
	struct tr_list *next;
} ibase_tr_list;

typedef struct {
	ISC_ARRAY_DESC ar_desc;
	int el_type, /* sqltype kinda SQL_TEXT, ...*/
		el_size; /* element size in bytes */
	ISC_LONG ISC_FAR ar_size; /* all array size in bytes */
} ibase_array;

typedef struct {
	isc_tr_handle trans_handle; 
	isc_db_handle link;
	ISC_QUAD bl_qd;
	isc_blob_handle bl_handle;
} ibase_blob_handle;

typedef struct {
	ibase_db_link *link;
	ibase_trans *trans;
	isc_stmt_handle stmt;
	int drop_stmt;
	XSQLDA *in_sqlda, *out_sqlda;
	ibase_array *in_array, *out_array;
	int in_array_cnt, out_array_cnt;
	unsigned short dialect;
	int cursor_open;
	char statement_type;
	char *query;
	int trans_res_id;
} ibase_query;

typedef struct {
	isc_db_handle link; /* db link for this result */
	isc_tr_handle trans;
	isc_stmt_handle stmt;
	int drop_stmt;
	XSQLDA *out_sqlda;
	ibase_array *out_array;
	unsigned char has_more_rows;
	char statement_type;
} ibase_result;

typedef struct {
	int link_res_id;
	ibase_db_link *link;
	ISC_LONG event_id;
	unsigned short event_count;
	char **events;
	char *event_buffer, *result_buffer;
	zval *callback;
	void **thread_ctx;
} ibase_event;

typedef struct _php_ibase_varchar {
	short var_len;
	char var_str[1];
} IBASE_VCHAR;

enum php_interbase_option {
	PHP_IBASE_DEFAULT 			= 0,
	PHP_IBASE_TEXT 				= 1,
	PHP_IBASE_UNIXTIME 			= 2,
	PHP_IBASE_TIMESTAMP 		= 4,
	PHP_IBASE_DATE 				= 8,
	PHP_IBASE_TIME 				= 16,
	/* transactions */	
	PHP_IBASE_WRITE 			= 2,
	PHP_IBASE_READ 				= 4,
	PHP_IBASE_COMMITTED 		= 8,
	PHP_IBASE_CONSISTENCY 		= 16,
	PHP_IBASE_CONCURRENCY 		= 32,
	PHP_IBASE_REC_VERSION 		= 64,
	PHP_IBASE_REC_NO_VERSION 	= 128,
	PHP_IBASE_NOWAIT 			= 256,
	PHP_IBASE_WAIT 				= 512
};

#ifdef ZTS
#define IBG(v) TSRMG(ibase_globals_id, zend_ibase_globals *, v)
#else
#define IBG(v) (ibase_globals.v)
#endif

#else

#define phpext_interbase_ptr NULL

#endif /* HAVE_IBASE */

#endif /* PHP_INTERBASE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
