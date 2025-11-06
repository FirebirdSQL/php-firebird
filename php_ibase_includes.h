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
   |          others                                                      |
   +----------------------------------------------------------------------+
   | You'll find history on Github                                        |
   | https://github.com/FirebirdSQL/php-firebird/commits/master           |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_IBASE_INCLUDES_H
#define PHP_IBASE_INCLUDES_H

#include <ibase.h>

#ifndef SQLDA_CURRENT_VERSION
#define SQLDA_CURRENT_VERSION SQLDA_VERSION1
#endif

#ifndef METADATALENGTH
#if FB_API_VER >= 40
#    define METADATALENGTH 63*4
#else
#    define METADATALENGTH 31
#endif
#endif

#define RESET_ERRMSG do { IBG(errmsg)[0] = '\0'; IBG(sql_code) = 0; } while (0)

#define IB_STATUS (IBG(status))

#ifdef IBASE_DEBUG
#define IBDEBUG(a) php_printf("::: %s (%s:%d)\n", a, __FILE__, __LINE__);
#endif

#ifndef IBDEBUG
#define IBDEBUG(a)
#endif

extern int le_link, le_plink, le_trans;

#define LE_LINK "Firebird/InterBase link"
#define LE_PLINK "Firebird/InterBase persistent link"
#define LE_TRANS "Firebird/InterBase transaction"

#define IBASE_MSGSIZE 512
#define MAX_ERRMSG (IBASE_MSGSIZE*2)

#define IB_DEF_DATE_FMT "%Y-%m-%d"
#define IB_DEF_TIME_FMT "%H:%M:%S"

/* this value should never be > USHRT_MAX */
#define IBASE_BLOB_SEG 4096

ZEND_BEGIN_MODULE_GLOBALS(ibase)
	ISC_STATUS status[20];
	zend_resource *default_link;
	zend_long num_links, num_persistent;
	char errmsg[MAX_ERRMSG];
	zend_long sql_code;
	zend_long default_trans_params;
	zend_long default_lock_timeout; // only used togetger with trans_param IBASE_LOCK_TIMEOUT
	void *get_master_interface;
	void *master_instance;
	void *get_statement_interface;
	int client_version;
	int client_major_version;
	int client_minor_version;
ZEND_END_MODULE_GLOBALS(ibase)

ZEND_EXTERN_MODULE_GLOBALS(ibase)

typedef struct {
	isc_db_handle handle;
	struct tr_list *tr_list;
	unsigned short dialect;
	struct event *event_head;
} ibase_db_link;

typedef struct {
	isc_tr_handle handle;
	unsigned short link_cnt;
	unsigned long affected_rows;
	ibase_db_link *db_link[1]; /* last member */
} ibase_trans;

typedef struct tr_list {
	ibase_trans *trans;
	struct tr_list *next;
} ibase_tr_list;

typedef struct {
	isc_blob_handle bl_handle;
	unsigned short type;
	ISC_QUAD bl_qd;
} ibase_blob;

typedef struct event {
	ibase_db_link *link;
	zend_resource* link_res;
	ISC_LONG event_id;
	unsigned short event_count;
	char **events;
    unsigned char *event_buffer, *result_buffer;
	zval callback;
	void *thread_ctx;
	struct event *event_next;
	enum event_state { NEW, ACTIVE, DEAD } state;
} ibase_event;

/* sql variables union
 * used for convert and binding input variables
 */
typedef struct {
	union {
// Boolean data type exists since FB 3.0
#ifdef SQL_BOOLEAN
		FB_BOOLEAN bval;
#endif
		short sval;
		float fval;
		ISC_LONG lval;
		ISC_QUAD qval;
		ISC_TIMESTAMP tsval;
		ISC_DATE dtval;
		ISC_TIME tmval;
	} val;
	short nullind;
} BIND_BUF;

typedef struct {
	ISC_ARRAY_DESC ar_desc;
	ISC_LONG ar_size; /* size of entire array in bytes */
	unsigned short el_type, el_size;
} ibase_array;

typedef struct _ib_query {
	ibase_db_link *link;
	ibase_trans *trans;
	zend_resource *trans_res;
	zend_resource *res;
	isc_stmt_handle stmt;
	XSQLDA *in_sqlda, *out_sqlda;
	ibase_array *in_array, *out_array;
	unsigned short type, has_more_rows, is_open;
	unsigned short in_array_cnt, out_array_cnt;
	unsigned short dialect;
	char *query;
	ISC_UCHAR statement_type;
	BIND_BUF *bind_buf;
	ISC_SHORT *in_nullind, *out_nullind;
	ISC_USHORT in_fields_count, out_fields_count;
	HashTable *ht_aliases, *ht_ind; // Precomputed for ibase_fetch_*()
	int was_result_once;
} ibase_query;

enum php_interbase_option {
	PHP_IBASE_DEFAULT            = 0,
	PHP_IBASE_CREATE             = 0,
	/* fetch flags */
	PHP_IBASE_FETCH_BLOBS        = 1,
	PHP_IBASE_FETCH_ARRAYS       = 2,
	PHP_IBASE_UNIXTIME           = 4,
	/* transaction access mode */
	PHP_IBASE_WRITE              = 1,
	PHP_IBASE_READ               = 2,
	/* transaction isolation level */
	PHP_IBASE_CONCURRENCY        = 4,
	PHP_IBASE_COMMITTED          = 8,
		PHP_IBASE_REC_NO_VERSION = 32,
		PHP_IBASE_REC_VERSION    = 64,
	PHP_IBASE_CONSISTENCY        = 16,
	/* transaction lock resolution */
	PHP_IBASE_WAIT               = 128,
	PHP_IBASE_NOWAIT             = 256,
		PHP_IBASE_LOCK_TIMEOUT   = 512,
};

#define IBG(v) ZEND_MODULE_GLOBALS_ACCESSOR(ibase, v)

#if defined(ZTS) && defined(COMPILE_DL_INTERBASE)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#define BLOB_ID_LEN     18
#define BLOB_ID_MASK    "0x%" LL_MASK "x"

#define BLOB_INPUT      1
#define BLOB_OUTPUT     2

#ifdef PHP_WIN32
// Case switch, because of troubles on Windows and PHP 8.0
#if PHP_VERSION_ID < 80000
   #define LL_MASK "I64"
#else
   #define LL_MASK "ll"
#endif
#define LL_LIT(lit) lit ## I64
typedef void (__stdcall *info_func_t)(char*);
#else
#define LL_MASK "ll"
#define LL_LIT(lit) lit ## ll
typedef void (*info_func_t)(char*);
#endif

void _php_ibase_error(void);
void _php_ibase_module_error(const char *, ...)
	PHP_ATTRIBUTE_FORMAT(printf,1,2);

/* determine if a resource is a link or transaction handle */
#define PHP_IBASE_LINK_TRANS(zv, lh, th)                                                    \
		do {                                                                                \
			if (!zv) {                                                                      \
				lh = (ibase_db_link *)zend_fetch_resource2(                                 \
					IBG(default_link), "InterBase link", le_link, le_plink);                \
			} else {                                                                        \
				_php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, zv, &lh, &th);  \
			}                                                                               \
			if (SUCCESS != _php_ibase_def_trans(lh, &th)) { RETURN_FALSE; }                 \
		} while (0)

int _php_ibase_def_trans(ibase_db_link *ib_link, ibase_trans **trans);
void _php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAMETERS, zval *link_id,
	ibase_db_link **ib_link, ibase_trans **trans);

/* provided by ibase_query.c */
void php_ibase_query_minit(INIT_FUNC_ARGS);

/* provided by ibase_blobs.c */
void php_ibase_blobs_minit(INIT_FUNC_ARGS);
int _php_ibase_string_to_quad(char const *id, ISC_QUAD *qd);
zend_string *_php_ibase_quad_to_string(ISC_QUAD const qd);
int _php_ibase_blob_get(zval *return_value, ibase_blob *ib_blob, zend_ulong max_len);
int _php_ibase_blob_add(zval *string_arg, ibase_blob *ib_blob);

/* provided by ibase_events.c */
void php_ibase_events_minit(INIT_FUNC_ARGS);
void _php_ibase_free_event(ibase_event *event);

/* provided by ibase_service.c */
void php_ibase_service_minit(INIT_FUNC_ARGS);

#ifdef __cplusplus
extern "C" {
#endif

void _php_ibase_insert_alias(HashTable *ht, const char *alias);
static int _php_ibase_get_vars_count(ibase_query *ib_query);
static int _php_ibase_fetch_query_res(zval *from, ibase_query **ib_query);
static int _php_ibase_alloc_ht_aliases(ibase_query *ib_query);
static void _php_ibase_alloc_ht_ind(ibase_query *ib_query);
static void _php_ibase_free_query_impl(INTERNAL_FUNCTION_PARAMETERS, int as_result);

#ifdef __cplusplus
}
#endif

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifdef PHP_DEBUG
void fbp_dump_buffer(int len, const unsigned char *buffer);
void fbp_dump_buffer_raw(int len, const unsigned char *buffer);
#endif

void fbp_error_ex(long level, const char *, ...)
    PHP_ATTRIBUTE_FORMAT(printf,2,3);

#ifdef PHP_WIN32
#define fbp_fatal(msg, ...)   fbp_error_ex(E_ERROR,   msg " (%s:%d)\n", ## __VA_ARGS__, __FILE__, __LINE__)
#define fbp_warning(msg, ...) fbp_error_ex(E_WARNING, msg " (%s:%d)\n", ## __VA_ARGS__, __FILE__, __LINE__)
#define fbp_notice(msg, ...)  fbp_error_ex(E_NOTICE,  msg " (%s:%d)\n", ## __VA_ARGS__, __FILE__, __LINE__)
#else
#define fbp_fatal(msg, ...)   fbp_error_ex(E_ERROR,   msg " (%s:%d)\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define fbp_warning(msg, ...) fbp_error_ex(E_WARNING, msg " (%s:%d)\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#define fbp_notice(msg, ...)  fbp_error_ex(E_NOTICE,  msg " (%s:%d)\n" __VA_OPT__(,) __VA_ARGS__, __FILE__, __LINE__)
#endif

typedef ISC_STATUS (ISC_EXPORT *fb_get_statement_interface_t)(
	ISC_STATUS* status_vector, void* db_handle, isc_stmt_handle* stmt_handle
);

typedef void* (ISC_EXPORT *fb_get_master_interface_t)(void);

#endif /* PHP_IBASE_INCLUDES_H */
