/*
   +----------------------------------------------------------------------+
   | PHP version 4.0                                                      |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997, 1998, 1999, 2000 The PHP Group                   |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | http://www.php.net/license/2_01.txt.                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jouni Ahto <jah@mork.net>					                  |
   |		  Andrew Avdeev <andy@simgts.mv.ru>							  |
   +----------------------------------------------------------------------+
 */

/* $Id$ */

#ifndef _PHP_IBASE_H
#define _PHP_IBASE_H

#if COMPILE_DL
#undef HAVE_IBASE
#define HAVE_IBASE 1
#endif

#if HAVE_IBASE
#include <ibase.h>

extern zend_module_entry ibase_module_entry;
#define phpext_interbase_ptr &ibase_module_entry

#ifdef PHP_WIN32
#define PHP_IBASE_API __declspec(dllexport)
#else
#define PHP_IBASE_API
#endif

extern PHP_MINIT_FUNCTION(ibase);
extern PHP_RINIT_FUNCTION(ibase);
extern PHP_MSHUTDOWN_FUNCTION(ibase);
extern PHP_RSHUTDOWN_FUNCTION(ibase);
PHP_MINFO_FUNCTION(ibase);

PHP_FUNCTION(ibase_connect);
PHP_FUNCTION(ibase_pconnect);
PHP_FUNCTION(ibase_close);
PHP_FUNCTION(ibase_query);
PHP_FUNCTION(ibase_fetch_row);
PHP_FUNCTION(ibase_fetch_object);
PHP_FUNCTION(ibase_free_result);
PHP_FUNCTION(ibase_prepare);
PHP_FUNCTION(ibase_execute);
PHP_FUNCTION(ibase_free_query);
PHP_FUNCTION(ibase_timefmt);

PHP_FUNCTION(ibase_num_fields);
PHP_FUNCTION(ibase_field_info);

PHP_FUNCTION(ibase_trans);
PHP_FUNCTION(ibase_commit);
PHP_FUNCTION(ibase_rollback);

PHP_FUNCTION(ibase_blob_create);
PHP_FUNCTION(ibase_blob_add);
PHP_FUNCTION(ibase_blob_cancel);
PHP_FUNCTION(ibase_blob_open);
PHP_FUNCTION(ibase_blob_get);
PHP_FUNCTION(ibase_blob_close);
PHP_FUNCTION(ibase_blob_echo);
PHP_FUNCTION(ibase_blob_info);
PHP_FUNCTION(ibase_blob_import);

PHP_FUNCTION(ibase_errmsg);

#define IBASE_MSGSIZE 256
#define MAX_ERRMSG (IBASE_MSGSIZE*2)
#define IBASE_TRANS_ON_LINK 10
#define IBASE_BLOB_SEG 4096

typedef struct {
	ISC_STATUS status[20];
	long default_link;
	long num_links, num_persistent;
	long max_links, max_persistent;
	long allow_persistent;
	int le_blob, le_link, le_plink, le_result, le_query;
	char *default_user, *default_password;
	char *timeformat;
	char *cfg_timeformat;
	char *errmsg;
} php_ibase_globals;

typedef struct {
	isc_tr_handle trans[IBASE_TRANS_ON_LINK];
	isc_db_handle link;
} ibase_db_link;

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
	isc_db_handle link; /* db link for this result */
	isc_tr_handle trans;
	isc_stmt_handle stmt;
	XSQLDA *in_sqlda, *out_sqlda;
	ibase_array *in_array, *out_array;
	int in_array_cnt, out_array_cnt;
} ibase_query;

typedef struct {
	isc_db_handle link; /* db link for this result */
	isc_tr_handle trans;
	isc_stmt_handle stmt;
	int drop_stmt;
	XSQLDA *out_sqlda;
	ibase_array *out_array;
} ibase_result;

typedef struct _php_ibase_varchar {
	short var_len;
	char var_str[1];
} IBASE_VCHAR;

/*
extern ibase_module php_ibase_module;
*/

enum php_interbase_option {
	PHP_IBASE_DEFAULT = 0,
	PHP_IBASE_TEXT = 1,
	PHP_IBASE_TIMESTAMP = 2,
	PHP_IBASE_READ = 4,
	PHP_IBASE_COMMITTED = 8,
	PHP_IBASE_CONSISTENCY = 16,
	PHP_IBASE_NOWAIT = 32
};

#ifdef ZTS
#define IBLS_D php_ibase_globals *ibase_globals
#define IBG(v) (ibase_globals->v)
#define IBLS_FETCH() php_ibase_globals *ibase_globals = ts_resource(ibase_globals_id)
#else
#define IBLS_D
#define IBG(v) (ibase_globals.v)
#define IBLS_FETCH()
extern PHP_IBASE_API php_ibase_globals ibase_globals;
#endif

#else

#define phpext_interbase_ptr NULL

#endif /* HAVE_IBASE */

#endif /* _PHP_IBASE_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
