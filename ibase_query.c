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

// #pragma GCC diagnostic error "-Wextra"
// #pragma GCC diagnostic error "-Wall"
// #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"

#if HAVE_IBASE

#include "ext/standard/php_standard.h"
#include "php_interbase.h"
#include "php_ibase_includes.h"
#include "firebird_utils.h"

#define ISC_LONG_MIN    INT_MIN
#define ISC_LONG_MAX    INT_MAX

#define QUERY_RESULT    1
#define EXECUTE_RESULT  2

#define FETCH_ROW       1
#define FETCH_ARRAY     2

typedef struct {
	unsigned short vary_length;
	char vary_string[1];
} IBVARY;

static int le_query;

static void _php_ibase_alloc_xsqlda_vars(XSQLDA *sqlda, ISC_SHORT *nullinds);
static int _php_ibase_set_query_info(ibase_query *ib_query);
static int _php_ibase_fetch_query_res(zval *from, ibase_query **ib_query);
static int _php_ibase_alloc_ht_aliases(ibase_query *ib_query);
static void _php_ibase_alloc_ht_ind(ibase_query *ib_query);
static void _php_ibase_free_query_impl(INTERNAL_FUNCTION_PARAMETERS, int as_result);

static void _php_ibase_free_xsqlda(XSQLDA *sqlda) /* {{{ */
{
	int i;
	XSQLVAR *var;

	IBDEBUG("Free XSQLDA?");
	if (sqlda) {
		IBDEBUG("Freeing XSQLDA...");
		var = sqlda->sqlvar;
		for (i = 0; i < sqlda->sqld; i++, var++) {
			efree(var->sqldata);
		}
		efree(sqlda);
	}
}
/* }}} */

static void _php_ibase_free_query(ibase_query *ib_query) /* {{{ */
{
	IBDEBUG("Freeing query...");

	if(ib_query->in_nullind)efree(ib_query->in_nullind);
	if(ib_query->out_nullind)efree(ib_query->out_nullind);
	if(ib_query->bind_buf)efree(ib_query->bind_buf);
	if(ib_query->in_sqlda)efree(ib_query->in_sqlda); // Note to myself: no need for _php_ibase_free_xsqlda()
	if(ib_query->out_sqlda)_php_ibase_free_xsqlda(ib_query->out_sqlda);
	if(ib_query->in_array)efree(ib_query->in_array);
	if(ib_query->out_array)efree(ib_query->out_array);
	if(ib_query->query)efree(ib_query->query);
	if(ib_query->ht_aliases)zend_array_destroy(ib_query->ht_aliases);
	if(ib_query->ht_ind)zend_array_destroy(ib_query->ht_ind);

	efree(ib_query);
}
/* }}} */

static void php_ibase_free_query_rsrc(zend_resource *rsrc) /* {{{ */
{
	ibase_query *ib_query = (ibase_query *)rsrc->ptr;

	if (ib_query != NULL) {
		IBDEBUG("Preparing to free query by dtor...");
		_php_ibase_free_query(ib_query);
	}
}
/* }}} */

void php_ibase_query_minit(INIT_FUNC_ARGS) /* {{{ */
{
	(void)type;
	le_query = zend_register_list_destructors_ex(php_ibase_free_query_rsrc, NULL,
		LE_QUERY, module_number);
}
/* }}} */

static int _php_ibase_alloc_array(ibase_array **ib_arrayp, XSQLDA *sqlda, /* {{{ */
	isc_db_handle link, isc_tr_handle trans, unsigned short *array_cnt)
{
	unsigned short i, n;
	ibase_array *ar;

	/* first check if we have any arrays at all */
	for (i = *array_cnt = 0; i < sqlda->sqld; ++i) {
		if ((sqlda->sqlvar[i].sqltype & ~1) == SQL_ARRAY) {
			++*array_cnt;
		}
	}
	if (! *array_cnt) return SUCCESS;

	ar = safe_emalloc(sizeof(ibase_array), *array_cnt, 0);

	for (i = n = 0; i < sqlda->sqld; ++i) {
		unsigned short dim;
		zend_ulong ar_size = 1;
		XSQLVAR *var = &sqlda->sqlvar[i];

		if ((var->sqltype & ~1) != SQL_ARRAY) {
			 continue;
		}

		ibase_array *a = &ar[n++];
		ISC_ARRAY_DESC *ar_desc = &a->ar_desc;

		if (isc_array_lookup_bounds(IB_STATUS, &link, &trans, var->relname,
				var->sqlname, ar_desc)) {
			_php_ibase_error();
			efree(ar);
			return FAILURE;
		}

		switch (ar_desc->array_desc_dtype) {
			case blr_text:
			case blr_text2:
				a->el_type = SQL_TEXT;
				a->el_size = ar_desc->array_desc_length;
				break;
#ifdef SQL_BOOLEAN
				case blr_bool:
					a->el_type = SQL_BOOLEAN;
					a->el_size = sizeof(FB_BOOLEAN);
					break;
#endif
			case blr_short:
				a->el_type = SQL_SHORT;
				a->el_size = sizeof(short);
				break;
			case blr_long:
				a->el_type = SQL_LONG;
				a->el_size = sizeof(ISC_LONG);
				break;
			case blr_float:
				a->el_type = SQL_FLOAT;
				a->el_size = sizeof(float);
				break;
			case blr_double:
				a->el_type = SQL_DOUBLE;
				a->el_size = sizeof(double);
				break;
			case blr_int64:
				a->el_type = SQL_INT64;
				a->el_size = sizeof(ISC_INT64);
				break;
			case blr_timestamp:
				a->el_type = SQL_TIMESTAMP;
				a->el_size = sizeof(ISC_TIMESTAMP);
				break;
			case blr_sql_date:
				a->el_type = SQL_TYPE_DATE;
				a->el_size = sizeof(ISC_DATE);
				break;
			case blr_sql_time:
				a->el_type = SQL_TYPE_TIME;
				a->el_size = sizeof(ISC_TIME);
				break;
#if FB_API_VER >= 40
			// These are converted to VARCHAR via isc_dpb_set_bind tag at connect
			// blr_dec64
			// blr_dec128
			// blr_int128
			case blr_sql_time_tz:
				a->el_type = SQL_TIME_TZ;
				a->el_size = sizeof(ISC_TIME_TZ);
				break;
			case blr_timestamp_tz:
				a->el_type = SQL_TIMESTAMP_TZ;
				a->el_size = sizeof(ISC_TIMESTAMP_TZ);
				break;
#endif
			case blr_varying:
			case blr_varying2:
				/**
				 * IB has a strange way of handling VARCHAR arrays. It doesn't store
				 * the length in the first short, as with VARCHAR fields. It does,
				 * however, expect the extra short to be allocated for each element.
				 */
				a->el_type = SQL_TEXT;
				a->el_size = ar_desc->array_desc_length + sizeof(short);
				break;
			case blr_quad:
			case blr_blob_id:
			case blr_cstring:
			case blr_cstring2:
				/**
				 * These types are mentioned as array types in the manual, but I
				 * wouldn't know how to create an array field with any of these
				 * types. I assume these types are not applicable to arrays, and
				 * were mentioned erroneously.
				 */
			default:
				_php_ibase_module_error("Unsupported array type %d in relation '%s' column '%s'",
					ar_desc->array_desc_dtype, var->relname, var->sqlname);
				efree(ar);
				return FAILURE;
		} /* switch array_desc_type */

		/* calculate elements count */
		for (dim = 0; dim < ar_desc->array_desc_dimensions; dim++) {
			ar_size *= 1 + ar_desc->array_desc_bounds[dim].array_bound_upper
				-ar_desc->array_desc_bounds[dim].array_bound_lower;
		}
		a->ar_size = a->el_size * ar_size;
	} /* for column */
	*ib_arrayp = ar;
	return SUCCESS;
}
/* }}} */

/* allocate and prepare query */
static int _php_ibase_prepare(ibase_query **new_query, ibase_db_link *link, /* {{{ */
	ibase_trans *trans, zend_resource *trans_res, char *query)
{
	/* Return FAILURE, if querystring is empty */
	if (*query == '\0') {
		php_error_docref(NULL, E_WARNING, "Querystring empty.");
		return FAILURE;
	}

	ibase_query *ib_query = ecalloc(1, sizeof(ibase_query));

	ib_query->res = zend_register_resource(ib_query, le_query);
	ib_query->link = link;
	ib_query->trans = trans;
	ib_query->trans_res = trans_res;
	ib_query->dialect = link->dialect;
	ib_query->query = estrdup(query);

	if (isc_dsql_allocate_statement(IB_STATUS, &link->handle, &ib_query->stmt)) {
		_php_ibase_error();
		goto _php_ibase_alloc_query_error;
	}

	if (isc_dsql_prepare(IB_STATUS, &ib_query->trans->handle, &ib_query->stmt,
			0, query, link->dialect, NULL)) {
		IBDEBUG("isc_dsql_prepare() failed\n");
		_php_ibase_error();
		goto _php_ibase_alloc_query_error;
	}

	if(_php_ibase_set_query_info(ib_query)){
		goto _php_ibase_alloc_query_error;
	}

	if(ib_query->out_fields_count) {
		ib_query->out_sqlda = (XSQLDA *) emalloc(XSQLDA_LENGTH(ib_query->out_fields_count));
		ib_query->out_sqlda->sqln = ib_query->out_fields_count;
		ib_query->out_sqlda->version = SQLDA_CURRENT_VERSION;

		if (isc_dsql_describe(IB_STATUS, &ib_query->stmt, SQLDA_CURRENT_VERSION, ib_query->out_sqlda)) {
			IBDEBUG("isc_dsql_describe() failed\n");
			_php_ibase_error();
			goto _php_ibase_alloc_query_error;
		}

		assert(ib_query->out_sqlda->sqln == ib_query->out_sqlda->sqld);
		assert(ib_query->out_sqlda->sqld == ib_query->out_fields_count);

		ib_query->out_nullind = safe_emalloc(sizeof(*ib_query->out_nullind), ib_query->out_sqlda->sqld, 0);
		_php_ibase_alloc_xsqlda_vars(ib_query->out_sqlda, ib_query->out_nullind);
		if (FAILURE == _php_ibase_alloc_array(&ib_query->out_array, ib_query->out_sqlda,
			link->handle, trans->handle, &ib_query->out_array_cnt)) {
			goto _php_ibase_alloc_query_error;
		}
	}

	if(ib_query->in_fields_count) {
		ib_query->in_sqlda = emalloc(XSQLDA_LENGTH(ib_query->in_fields_count));
		ib_query->in_sqlda->sqln = ib_query->in_fields_count;
		ib_query->in_sqlda->version = SQLDA_CURRENT_VERSION;

		if (isc_dsql_describe_bind(IB_STATUS, &ib_query->stmt, SQLDA_CURRENT_VERSION, ib_query->in_sqlda)) {
			IBDEBUG("isc_dsql_describe_bind() failed\n");
			_php_ibase_error();
			goto _php_ibase_alloc_query_error;
		}

		assert(ib_query->in_sqlda->sqln == ib_query->in_sqlda->sqld);
		assert(ib_query->in_sqlda->sqld == ib_query->in_fields_count);

		ib_query->bind_buf = safe_emalloc(sizeof(BIND_BUF), ib_query->in_sqlda->sqld, 0);
		ib_query->in_nullind = safe_emalloc(sizeof(*ib_query->in_nullind), ib_query->in_sqlda->sqld, 0);
		if (FAILURE == _php_ibase_alloc_array(&ib_query->in_array, ib_query->in_sqlda,
			link->handle, trans->handle, &ib_query->in_array_cnt)) {
			goto _php_ibase_alloc_query_error;
		}
	}

	*new_query = ib_query;

	return SUCCESS;

_php_ibase_alloc_query_error:
	zend_list_delete(ib_query->res);

	return FAILURE;
}
/* }}} */

static int _php_ibase_bind_array(zval *val, char *buf, zend_ulong buf_size, /* {{{ */
	ibase_array *array, int dim)
{
	zval null_val, *pnull_val = &null_val;
	int u_bound = array->ar_desc.array_desc_bounds[dim].array_bound_upper,
		l_bound = array->ar_desc.array_desc_bounds[dim].array_bound_lower,
		dim_len = 1 + u_bound - l_bound;

	ZVAL_NULL(pnull_val);

	if (dim < array->ar_desc.array_desc_dimensions) {
		zend_ulong slice_size = buf_size / dim_len;
		unsigned short i;
		zval *subval = val;

		if (Z_TYPE_P(val) == IS_ARRAY) {
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
		}

		for (i = 0; i < dim_len; ++i) {

			if (Z_TYPE_P(val) == IS_ARRAY &&
				(subval = zend_hash_get_current_data(Z_ARRVAL_P(val))) == NULL)
			{
				subval = pnull_val;
			}

			if (_php_ibase_bind_array(subval, buf, slice_size, array, dim+1) == FAILURE)
			{
				return FAILURE;
			}
			buf += slice_size;

			if (Z_TYPE_P(val) == IS_ARRAY) {
				zend_hash_move_forward(Z_ARRVAL_P(val));
			}
		}

		if (Z_TYPE_P(val) == IS_ARRAY) {
			zend_hash_internal_pointer_reset(Z_ARRVAL_P(val));
		}

	} else {
		/* expect a single value */
		if (Z_TYPE_P(val) == IS_NULL) {
			memset(buf, 0, buf_size);
		} else if (array->ar_desc.array_desc_scale < 0) {

			/* no coercion for array types */
			double l;

			convert_to_double(val);

			if (Z_DVAL_P(val) > 0) {
				l = Z_DVAL_P(val) * pow(10, -array->ar_desc.array_desc_scale) + .5;
			} else {
				l = Z_DVAL_P(val) * pow(10, -array->ar_desc.array_desc_scale) - .5;
			}

			switch (array->el_type) {
				case SQL_SHORT:
					if (l > SHRT_MAX || l < SHRT_MIN) {
						_php_ibase_module_error("Array parameter exceeds field width");
						return FAILURE;
					}
					*(short*) buf = (short) l;
					break;
				case SQL_LONG:
					if (l > ISC_LONG_MAX || l < ISC_LONG_MIN) {
						_php_ibase_module_error("Array parameter exceeds field width");
						return FAILURE;
					}
					*(ISC_LONG*) buf = (ISC_LONG) l;
					break;
				case SQL_INT64:
					{
						long double l;

						convert_to_string(val);

						if (!sscanf(Z_STRVAL_P(val), "%Lf", &l)) {
							_php_ibase_module_error("Cannot convert '%s' to long double",
								 Z_STRVAL_P(val));
							return FAILURE;
						}

						if (l > 0) {
							*(ISC_INT64 *) buf = (ISC_INT64) (l * pow(10,
								-array->ar_desc.array_desc_scale) + .5);
						} else {
							*(ISC_INT64 *) buf = (ISC_INT64) (l * pow(10,
								-array->ar_desc.array_desc_scale) - .5);
						}
					}
					break;
			}
		} else {
			struct tm t = { 0 };

			switch (array->el_type) {
#ifndef HAVE_STRPTIME
				unsigned short n;
#endif
#if (SIZEOF_ZEND_LONG < 8)
				ISC_INT64 l;
#endif

				case SQL_SHORT:
					convert_to_long(val);
					if (Z_LVAL_P(val) > SHRT_MAX || Z_LVAL_P(val) < SHRT_MIN) {
						_php_ibase_module_error("Array parameter exceeds field width");
						return FAILURE;
					}
					*(short *) buf = (short) Z_LVAL_P(val);
					break;
				case SQL_LONG:
					convert_to_long(val);
#if (SIZEOF_ZEND_LONG > 4)
					if (Z_LVAL_P(val) > ISC_LONG_MAX || Z_LVAL_P(val) < ISC_LONG_MIN) {
						_php_ibase_module_error("Array parameter exceeds field width");
						return FAILURE;
					}
#endif
					*(ISC_LONG *) buf = (ISC_LONG) Z_LVAL_P(val);
					break;
				case SQL_INT64:
#if (SIZEOF_ZEND_LONG >= 8)
					convert_to_long(val);
					*(zend_long *) buf = Z_LVAL_P(val);
#else
					convert_to_string(val);
					if (!sscanf(Z_STRVAL_P(val), "%" LL_MASK "d", &l)) {
						_php_ibase_module_error("Cannot convert '%s' to long integer",
							 Z_STRVAL_P(val));
						return FAILURE;
					} else {
						*(ISC_INT64 *) buf = l;
					}
#endif
					break;
				case SQL_FLOAT:
					convert_to_double(val);
					*(float*) buf = (float) Z_DVAL_P(val);
					break;
#ifdef SQL_BOOLEAN
				case SQL_BOOLEAN:
					convert_to_boolean(val);
					// On Windows error unresolved symbol Z_BVAL_P is thrown, so we use Z_LVAL_P
					*(FB_BOOLEAN*) buf = Z_LVAL_P(val);
					break;
#endif
				case SQL_DOUBLE:
					convert_to_double(val);
					*(double*) buf = Z_DVAL_P(val);
					break;
				case SQL_TIMESTAMP:
				// TODO:
				// case SQL_TIMESTAMP_TZ:
					convert_to_string(val);
#ifdef HAVE_STRPTIME
					strptime(Z_STRVAL_P(val), INI_STR("ibase.timestampformat"), &t);
#else
					n = sscanf(Z_STRVAL_P(val), "%d%*[/]%d%*[/]%d %d%*[:]%d%*[:]%d",
						&t.tm_mon, &t.tm_mday, &t.tm_year, &t.tm_hour, &t.tm_min, &t.tm_sec);

					if (n != 3 && n != 6) {
						_php_ibase_module_error("Invalid date/time format (expected 3 or 6 fields, got %d."
							" Use format 'm/d/Y H:i:s'. You gave '%s')", n, Z_STRVAL_P(val));
						return FAILURE;
					}
					t.tm_year -= 1900;
					t.tm_mon--;
#endif
					isc_encode_timestamp(&t, (ISC_TIMESTAMP * ) buf);
					break;
				case SQL_TYPE_DATE:
					convert_to_string(val);
#ifdef HAVE_STRPTIME
					strptime(Z_STRVAL_P(val), INI_STR("ibase.dateformat"), &t);
#else
					n = sscanf(Z_STRVAL_P(val), "%d%*[/]%d%*[/]%d", &t.tm_mon, &t.tm_mday, &t.tm_year);

					if (n != 3) {
						_php_ibase_module_error("Invalid date format (expected 3 fields, got %d. "
							"Use format 'm/d/Y' You gave '%s')", n, Z_STRVAL_P(val));
						return FAILURE;
					}
					t.tm_year -= 1900;
					t.tm_mon--;
#endif
					isc_encode_sql_date(&t, (ISC_DATE *) buf);
					break;
				case SQL_TYPE_TIME:
				// TODO:
				// case SQL_TIME_TZ:
					convert_to_string(val);
#ifdef HAVE_STRPTIME
					strptime(Z_STRVAL_P(val), INI_STR("ibase.timeformat"), &t);
#else
					n = sscanf(Z_STRVAL_P(val), "%d%*[:]%d%*[:]%d", &t.tm_hour, &t.tm_min, &t.tm_sec);

					if (n != 3) {
						_php_ibase_module_error("Invalid time format (expected 3 fields, got %d. "
							"Use format 'H:i:s'. You gave '%s')", n, Z_STRVAL_P(val));
						return FAILURE;
					}
#endif
					isc_encode_sql_time(&t, (ISC_TIME *) buf);
					break;
				default:
					convert_to_string(val);
					strlcpy(buf, Z_STRVAL_P(val), buf_size);
			}
		}
	}
	return SUCCESS;
}
/* }}} */

static int _php_ibase_bind(ibase_query *ib_query, zval *b_vars) /* {{{ */
{
	BIND_BUF *buf = ib_query->bind_buf;
	XSQLDA *sqlda = ib_query->in_sqlda;

	int i, array_cnt = 0, rv = SUCCESS;

	for (i = 0; i < sqlda->sqld; ++i) { /* bound vars */

		zval *b_var = &b_vars[i];
		XSQLVAR *var = &sqlda->sqlvar[i];

		var->sqlind = &buf[i].nullind;
		var->sqldata = (void*)&buf[i].val;

		/* check if a NULL should be inserted */
		switch (Z_TYPE_P(b_var)) {
			int force_null;

			case IS_STRING:

				force_null = 0;

				/* for these types, an empty string can be handled like a NULL value */
				switch (var->sqltype & ~1) {
					case SQL_SHORT:
					case SQL_LONG:
					case SQL_INT64:
					case SQL_FLOAT:
					case SQL_DOUBLE:
					case SQL_TIMESTAMP:
					case SQL_TYPE_DATE:
					case SQL_TYPE_TIME:
#if FB_API_VER >= 40
					case SQL_INT128:
					case SQL_DEC16:
					case SQL_DEC34:
					case SQL_TIMESTAMP_TZ:
					case SQL_TIME_TZ:
#endif
						force_null = (Z_STRLEN_P(b_var) == 0);
				}

				if (! force_null) break;

			case IS_NULL:
					buf[i].nullind = -1;

				if (var->sqltype & SQL_ARRAY) ++array_cnt;

				continue;
		}

		/* if we make it to this point, we must provide a value for the parameter */

		buf[i].nullind = 0;

		switch (var->sqltype & ~1) {
			struct tm t;

			case SQL_TIMESTAMP:
			// TODO:
			// case SQL_TIMESTAMP_TZ:
			// case SQL_TIME_TZ:
			case SQL_TYPE_DATE:
			case SQL_TYPE_TIME:
				if (Z_TYPE_P(b_var) == IS_LONG) {
					struct tm *res;
					res = php_gmtime_r(&Z_LVAL_P(b_var), &t);
					if (!res) {
						return FAILURE;
					}
				} else {
#ifdef HAVE_STRPTIME
					char *format = INI_STR("ibase.timestampformat");

					convert_to_string(b_var);

					switch (var->sqltype & ~1) {
						case SQL_TYPE_DATE:
							format = INI_STR("ibase.dateformat");
							break;
						case SQL_TYPE_TIME:
						// TODO:
						// case SQL_TIME_TZ:
							format = INI_STR("ibase.timeformat");
					}
					if (!strptime(Z_STRVAL_P(b_var), format, &t)) {
						/* strptime() cannot handle it, so let IB have a try */
						break;
					}
#else /* ifndef HAVE_STRPTIME */
					break; /* let IB parse it as a string */
#endif
				}

				switch (var->sqltype & ~1) {
					default: /* == case SQL_TIMESTAMP */
						isc_encode_timestamp(&t, &buf[i].val.tsval);
						break;
					case SQL_TYPE_DATE:
						isc_encode_sql_date(&t, &buf[i].val.dtval);
						break;
					case SQL_TYPE_TIME:
					// TODO:
					// case SQL_TIME_TZ:
						isc_encode_sql_time(&t, &buf[i].val.tmval);
						break;
				}
				continue;

			case SQL_BLOB:

				convert_to_string(b_var);

				if (Z_STRLEN_P(b_var) != BLOB_ID_LEN ||
					!_php_ibase_string_to_quad(Z_STRVAL_P(b_var), &buf[i].val.qval)) {

					ibase_blob ib_blob = { 0 };
					ib_blob.type = BLOB_INPUT;

					if (isc_create_blob(IB_STATUS, &ib_query->link->handle,
							&ib_query->trans->handle, &ib_blob.bl_handle, &ib_blob.bl_qd)) {
						_php_ibase_error();
						return FAILURE;
					}

					if (_php_ibase_blob_add(b_var, &ib_blob) != SUCCESS) {
						return FAILURE;
					}

					if (isc_close_blob(IB_STATUS, &ib_blob.bl_handle)) {
						_php_ibase_error();
						return FAILURE;
					}
					buf[i].val.qval = ib_blob.bl_qd;
				}
				continue;
#ifdef SQL_BOOLEAN
			case SQL_BOOLEAN:

				switch (Z_TYPE_P(b_var)) {
					case IS_LONG:
					case IS_DOUBLE:
					case IS_TRUE:
					case IS_FALSE:
						*(FB_BOOLEAN *)var->sqldata = zend_is_true(b_var) ? FB_TRUE : FB_FALSE;
						break;
					case IS_STRING:
					{
						zend_long lval;
						double dval;

						if ((Z_STRLEN_P(b_var) == 0)) {
							*(FB_BOOLEAN *)var->sqldata = FB_FALSE;
							break;
						}

						switch (is_numeric_string(Z_STRVAL_P(b_var), Z_STRLEN_P(b_var), &lval, &dval, 0)) {
							case IS_LONG:
								*(FB_BOOLEAN *)var->sqldata = (lval != 0) ? FB_TRUE : FB_FALSE;
								break;
							case IS_DOUBLE:
								*(FB_BOOLEAN *)var->sqldata = (dval != 0) ? FB_TRUE : FB_FALSE;
								break;
							default:
								if (!zend_binary_strncasecmp(Z_STRVAL_P(b_var), Z_STRLEN_P(b_var), "true", 4, 4)) {
									*(FB_BOOLEAN *)var->sqldata = FB_TRUE;
								} else if (!zend_binary_strncasecmp(Z_STRVAL_P(b_var), Z_STRLEN_P(b_var), "false", 5, 5)) {
									*(FB_BOOLEAN *)var->sqldata = FB_FALSE;
								} else {
									_php_ibase_module_error("Parameter %d: cannot convert string to boolean", i+1);
									rv = FAILURE;
									continue;
								}
						}
						break;
					}
					case IS_NULL:
						buf[i].nullind = -1;
						break;
					default:
						_php_ibase_module_error("Parameter %d: must be boolean", i+1);
						rv = FAILURE;
						continue;
				}
				var->sqltype = SQL_BOOLEAN;
				continue;
#endif
			case SQL_ARRAY:

				if (Z_TYPE_P(b_var) != IS_ARRAY) {
					convert_to_string(b_var);

					if (Z_STRLEN_P(b_var) != BLOB_ID_LEN ||
						!_php_ibase_string_to_quad(Z_STRVAL_P(b_var), &buf[i].val.qval)) {

						_php_ibase_module_error("Parameter %d: invalid array ID",i+1);
						rv = FAILURE;
					}
				} else {
					/* convert the array data into something IB can understand */
					ibase_array *ar = &ib_query->in_array[array_cnt];
					void *array_data = emalloc(ar->ar_size);
					ISC_QUAD array_id = { 0, 0 };

					if (FAILURE == _php_ibase_bind_array(b_var, array_data, ar->ar_size,
							ar, 0)) {
						_php_ibase_module_error("Parameter %d: failed to bind array argument", i+1);
						efree(array_data);
						rv = FAILURE;
						continue;
					}

					if (isc_array_put_slice(IB_STATUS, &ib_query->link->handle, &ib_query->trans->handle,
							&array_id, &ar->ar_desc, array_data, &ar->ar_size)) {
						_php_ibase_error();
						efree(array_data);
						return FAILURE;
					}
					buf[i].val.qval = array_id;
					efree(array_data);
				}
				++array_cnt;
				continue;
			} /* switch */

			/* we end up here if none of the switch cases handled the field */
			convert_to_string(b_var);
			var->sqldata = Z_STRVAL_P(b_var);
			var->sqllen	 = (ISC_SHORT)Z_STRLEN_P(b_var);
			var->sqltype = SQL_TEXT;
	} /* for */
	return rv;
}
/* }}} */

static void _php_ibase_alloc_xsqlda_vars(XSQLDA *sqlda, ISC_SHORT *nullinds) /* {{{ */
{
	int i;

	for (i = 0; i < sqlda->sqld; i++) {
		XSQLVAR *var = &sqlda->sqlvar[i];

		// Convert CHAR to VARCHAR to avoid dealiong with padding with
		// multi character charsets

		if ((var->sqltype & ~1) == SQL_TEXT) {
			var->sqltype = SQL_VARYING | (var->sqltype & 1);
		}

		switch (var->sqltype & ~1) {
			case SQL_TEXT:
				var->sqldata = safe_emalloc(sizeof(char), var->sqllen, 0);
				break;
			case SQL_VARYING:
				var->sqldata = safe_emalloc(sizeof(char), var->sqllen + sizeof(short), 0);
				break;
#ifdef SQL_BOOLEAN
			case SQL_BOOLEAN:
				var->sqldata = emalloc(sizeof(FB_BOOLEAN));
				break;
#endif
			case SQL_SHORT:
				var->sqldata = emalloc(sizeof(short));
				break;
			case SQL_LONG:
				var->sqldata = emalloc(sizeof(ISC_LONG));
				break;
			case SQL_FLOAT:
				var->sqldata = emalloc(sizeof(float));
					break;
			case SQL_DOUBLE:
				var->sqldata = emalloc(sizeof(double));
				break;
			case SQL_INT64:
				var->sqldata = emalloc(sizeof(ISC_INT64));
				break;
			case SQL_TIMESTAMP:
				var->sqldata = emalloc(sizeof(ISC_TIMESTAMP));
				break;
			case SQL_TYPE_DATE:
				var->sqldata = emalloc(sizeof(ISC_DATE));
				break;
			case SQL_TYPE_TIME:
				var->sqldata = emalloc(sizeof(ISC_TIME));
				break;
			case SQL_BLOB:
			case SQL_ARRAY:
				var->sqldata = emalloc(sizeof(ISC_QUAD));
				break;
#if FB_API_VER >= 40
			// These are converted to VARCHAR via isc_dpb_set_bind tag at connect
			// case SQL_DEC16:
			// case SQL_DEC34:
			// case SQL_INT128:
			case SQL_TIMESTAMP_TZ:
				var->sqldata = emalloc(sizeof(ISC_TIMESTAMP_TZ));
				break;
			case SQL_TIME_TZ:
				var->sqldata = emalloc(sizeof(ISC_TIME_TZ));
				break;
#endif
			default:
				// TODO: report human readable type. Grab ints from sqlda_pub.h
				// and just map to char *
				php_error(E_WARNING, "Unhandled sqltype: %d for sqlname %s %s:%d. Probably compiled against old fbclient library (%d)", var->sqltype, var->sqlname, __FILE__, __LINE__, FB_API_VER);
				break;
		} /* switch */

		if (var->sqltype & 1) { /* sql NULL flag */
			var->sqlind = &nullinds[i];
		} else {
			var->sqlind = NULL;
		}
	} /* for */
}
/* }}} */

static int _php_ibase_exec(INTERNAL_FUNCTION_PARAMETERS, ibase_query *ib_query, zval *args, int bind_n) /* {{{ */
{
	int i, rv = FAILURE;
	static char info_count[] = { isc_info_sql_records };
	char result[64];
	ISC_STATUS isc_result;
	int argc = ib_query->in_fields_count;

	(void)execute_data;

	RESET_ERRMSG;
	RETVAL_FALSE;

	if (bind_n != argc) {
		php_error_docref(NULL, (bind_n < argc) ? E_WARNING : E_NOTICE,
			"Statement expects %d arguments, %d given", argc, bind_n);

		if (bind_n < argc) {
			return FAILURE;
		}
	}

	/* Have we used this cursor before and it's still open (exec proc has no cursor) ? */
	if (ib_query->is_open && ib_query->statement_type != isc_info_sql_stmt_exec_procedure) {
		IBDEBUG("Implicitly closing a cursor");
		if (isc_dsql_free_statement(IB_STATUS, &ib_query->stmt, DSQL_close)) {
			_php_ibase_error();
			return FAILURE;
		}
		ib_query->is_open = 0;
	}

	for (i = 0; i < argc; ++i) {
		SEPARATE_ZVAL(&args[i]);
	}

	switch (ib_query->statement_type) {
		isc_tr_handle tr;
		ibase_tr_list **l;
		ibase_trans *trans;

		case isc_info_sql_stmt_start_trans:

			/* a SET TRANSACTION statement should be executed with a NULL trans handle */
			tr = 0;

			if (isc_dsql_execute_immediate(IB_STATUS, &ib_query->link->handle, &tr, 0,
					ib_query->query, ib_query->dialect, NULL)) {
				_php_ibase_error();
				goto _php_ibase_ex_error;
			}

			trans = (ibase_trans *) emalloc(sizeof(ibase_trans));
			trans->handle = tr;
			trans->link_cnt = 1;
			trans->affected_rows = 0;
			trans->db_link[0] = ib_query->link;

			if (ib_query->link->tr_list == NULL) {
				ib_query->link->tr_list = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
				ib_query->link->tr_list->trans = NULL;
				ib_query->link->tr_list->next = NULL;
			}

			/* link the transaction into the connection-transaction list */
			for (l = &ib_query->link->tr_list; *l != NULL; l = &(*l)->next);
			*l = (ibase_tr_list *) emalloc(sizeof(ibase_tr_list));
			(*l)->trans = trans;
			(*l)->next = NULL;

			RETVAL_RES(zend_register_resource(trans, le_trans));
			Z_TRY_ADDREF_P(return_value);

			return SUCCESS;

		case isc_info_sql_stmt_commit:
		case isc_info_sql_stmt_rollback:

			if (isc_dsql_execute_immediate(IB_STATUS, &ib_query->link->handle,
					&ib_query->trans->handle, 0, ib_query->query, ib_query->dialect, NULL)) {
				_php_ibase_error();
				goto _php_ibase_ex_error;
			}

			if (ib_query->trans->handle == 0 && ib_query->trans_res != NULL) {
				/* transaction was released by the query and was a registered resource,
				   so we have to release it */
				zend_list_delete(ib_query->trans_res);
				ib_query->trans_res = NULL;
			}

			RETVAL_TRUE;

			return SUCCESS;

		default:
			RETVAL_FALSE;
	}

	if (ib_query->in_fields_count) { /* has placeholders */
		IBDEBUG("Query wants XSQLDA for input");
		if (_php_ibase_bind(ib_query, args) == FAILURE) {
			IBDEBUG("Could not bind input XSQLDA");
			goto _php_ibase_ex_error;
		}
	}

	if (ib_query->statement_type == isc_info_sql_stmt_exec_procedure) {
		isc_result = isc_dsql_execute2(IB_STATUS, &ib_query->trans->handle,
			&ib_query->stmt, SQLDA_CURRENT_VERSION, ib_query->in_sqlda, ib_query->out_sqlda);
	} else {
		isc_result = isc_dsql_execute(IB_STATUS, &ib_query->trans->handle,
			&ib_query->stmt, SQLDA_CURRENT_VERSION, ib_query->in_sqlda);
	}

	if (isc_result) {
		IBDEBUG("Could not execute query");
		_php_ibase_error();
		goto _php_ibase_ex_error;
	}

	ib_query->trans->affected_rows = 0;

	// TODO: test INSERT / UPDATE / UPDATE OR INSERT with  ... RETURNING
	if (ib_query->out_sqlda) { /* output variables in select, select for update */
		ib_query->has_more_rows = 1;
		ib_query->is_open = 1;

		RETVAL_RES(ib_query->res);
		Z_TRY_ADDREF_P(return_value);

		return SUCCESS;
	}

	switch (ib_query->statement_type) {

		unsigned long affected_rows;

		case isc_info_sql_stmt_insert:
		case isc_info_sql_stmt_update:
		case isc_info_sql_stmt_delete:
		case isc_info_sql_stmt_exec_procedure:

			if (isc_dsql_sql_info(IB_STATUS, &ib_query->stmt, sizeof(info_count),
					info_count, sizeof(result), result)) {
				_php_ibase_error();
				goto _php_ibase_ex_error;
			}

			affected_rows = 0;

			if (result[0] == isc_info_sql_records) {
				unsigned i = 3, result_size = isc_vax_integer(&result[1],2);

				while (result[i] != isc_info_end && i < result_size) {
					short len = (short)isc_vax_integer(&result[i+1],2);
					if (result[i] != isc_info_req_select_count) {
						affected_rows += isc_vax_integer(&result[i+3],len);
					}
					i += len+3;
				}
			}

			ib_query->trans->affected_rows = affected_rows;

			if (!ib_query->out_sqlda) { /* no result set is being returned */
				if (affected_rows) {
					RETVAL_LONG(affected_rows);
				} else {
					RETVAL_TRUE;
				}
				break;
			}
		default:
			RETVAL_TRUE;
	}

	rv = SUCCESS;

_php_ibase_ex_error:
	return rv;
}
/* }}} */

/* {{{ proto mixed ibase_query([resource link_identifier, [ resource link_identifier, ]] string query [, mixed bind_arg [, mixed bind_arg [, ...]]])
   Execute a query */
PHP_FUNCTION(ibase_query)
{
	zval *zlink, *ztrans, *bind_args = NULL;
	char *query;
	size_t query_len;
	int bind_i, bind_num;
	zend_resource *trans_res = NULL;
	ibase_db_link *ib_link = NULL;
	ibase_trans *trans = NULL;

	RESET_ERRMSG;

	RETVAL_FALSE;

	switch (ZEND_NUM_ARGS()) {
		zend_long l;

		default:
			if (SUCCESS == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, 3, "rrs",
					&zlink, &ztrans, &query, &query_len)) {

				ib_link = (ibase_db_link*)zend_fetch_resource2_ex(zlink, LE_LINK, le_link, le_plink);
				trans = (ibase_trans*)zend_fetch_resource_ex(ztrans, LE_TRANS,	le_trans);

				trans_res = Z_RES_P(ztrans);
				bind_i = 3;
				break;
			}
		case 2:
			if (SUCCESS == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, 2, "rs",
					&zlink, &query, &query_len)) {
				_php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, zlink, &ib_link, &trans);

				if (trans != NULL) {
					trans_res = Z_RES_P(zlink);
				}
				bind_i = 2;
				break;
			}

			/* the statement is 'CREATE DATABASE ...' if the link argument is IBASE_CREATE */
			if (SUCCESS == zend_parse_parameters_ex(ZEND_PARSE_PARAMS_QUIET, ZEND_NUM_ARGS(),
					 "ls", &l, &query, &query_len) && l == PHP_IBASE_CREATE) {
				isc_db_handle db = 0;
				isc_tr_handle trh = 0;

				if (((l = INI_INT("ibase.max_links")) != -1) && (IBG(num_links) >= l)) {
					_php_ibase_module_error("CREATE DATABASE is not allowed: maximum link count "
						"(" ZEND_LONG_FMT ") reached", l);

				} else if (isc_dsql_execute_immediate(IB_STATUS, &db, &trh, (short)query_len,
						query, SQL_DIALECT_CURRENT, NULL)) {
					_php_ibase_error();

				} else if (!db) {
					_php_ibase_module_error("Connection to created database could not be "
						"established");

				} else {

					/* register the link as a resource; unfortunately, we cannot register
					   it in the hash table, because we don't know the connection params */
					ib_link = (ibase_db_link *) emalloc(sizeof(ibase_db_link));
					ib_link->handle = db;
					ib_link->dialect = SQL_DIALECT_CURRENT;
					ib_link->tr_list = NULL;
					ib_link->event_head = NULL;

					RETVAL_RES(zend_register_resource(ib_link, le_link));
					Z_TRY_ADDREF_P(return_value);

					IBG(default_link) = Z_RES_P(return_value);
					Z_TRY_ADDREF_P(return_value);
				}
				return;
			}
		case 1:
		case 0:
			if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS() ? 1 : 0, "s", &query,
					&query_len)) {
				ib_link = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink);

				bind_i = 1;
				break;
			}
			return;
	}

	/* open default transaction */
	if (ib_link == NULL || FAILURE == _php_ibase_def_trans(ib_link, &trans)){
		return;
	}

	ibase_query *ib_query;
	if(FAILURE == _php_ibase_prepare(&ib_query, ib_link, trans, trans_res, query)) {
		return;
	}

	{ // was while
		int bind_n = ZEND_NUM_ARGS() - bind_i;
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &bind_args, &bind_num) == FAILURE) {
			goto _php_ibase_query_error;
		}

		if (FAILURE == _php_ibase_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, ib_query, &bind_args[bind_i], bind_n)) {
			goto _php_ibase_query_error;
		}

		ib_query->was_result_once = 1;

		return;
	}

	assert(false && "UNREACHABLE");

_php_ibase_query_error:
	zend_list_delete(ib_query->res);
}
/* }}} */

/* {{{ proto int ibase_affected_rows( [ resource link_identifier ] )
   Returns the number of rows affected by the previous INSERT, UPDATE or DELETE statement */
PHP_FUNCTION(ibase_affected_rows)
{
	ibase_trans *trans = NULL;
	ibase_db_link *ib_link;
	zval *arg = NULL;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r", &arg) == FAILURE) {
		return;
	}

	if (!arg) {
		ib_link = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink);
		if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
			RETURN_FALSE;
		}
		trans = ib_link->tr_list->trans;
	} else {
		/* one id was passed, could be db or trans id */
		_php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, arg, &ib_link, &trans);
		if (trans == NULL) {
			ib_link = (ibase_db_link *)zend_fetch_resource2_ex(arg, LE_LINK, le_link, le_plink);

			if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
				RETURN_FALSE;
			}
			trans = ib_link->tr_list->trans;
		}
	}
	RETURN_LONG(trans->affected_rows);
}
/* }}} */

static int _php_ibase_var_zval(zval *val, void *data, int type, int len, /* {{{ */
	int scale, size_t flag)
{
	static ISC_INT64 const scales[] = { 1, 10, 100, 1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000,
		LL_LIT(10000000000),
		LL_LIT(100000000000),
		LL_LIT(1000000000000),
		LL_LIT(10000000000000),
		LL_LIT(100000000000000),
		LL_LIT(1000000000000000),
		LL_LIT(10000000000000000),
		LL_LIT(100000000000000000),
		LL_LIT(1000000000000000000)
	};

	switch (type & ~1) {
		unsigned short l;
		zend_long n;
		char string_data[255];
		struct tm t;
		char *format;

		case SQL_VARYING:
			len = ((IBVARY *) data)->vary_length;
			data = ((IBVARY *) data)->vary_string;
			/* no break */
		case SQL_TEXT:
			ZVAL_STRINGL(val, (char*)data, len);
			break;
#ifdef SQL_BOOLEAN
		case SQL_BOOLEAN:
			ZVAL_BOOL(val, *(FB_BOOLEAN *) data);
			break;
#endif
		case SQL_SHORT:
			n = *(short *) data;
			goto _sql_long;
		case SQL_INT64:
// TODO: do tests cover these cases?
#if (SIZEOF_ZEND_LONG >= 8)
			n = *(zend_long *) data;
			goto _sql_long;
#else
			if (scale == 0) {
				l = slprintf(string_data, sizeof(string_data), "%" LL_MASK "d", *(ISC_INT64 *) data);
				ZVAL_STRINGL(val,string_data,l);
			} else {
				ISC_INT64 n = *(ISC_INT64 *) data, f = scales[-scale];

				if (n >= 0) {
					l = slprintf(string_data, sizeof(string_data), "%" LL_MASK "d.%0*" LL_MASK "d", n / f, -scale, n % f);
				} else if (n <= -f) {
					l = slprintf(string_data, sizeof(string_data), "%" LL_MASK "d.%0*" LL_MASK "d", n / f, -scale, -n % f);
				 } else {
					l = slprintf(string_data, sizeof(string_data), "-0.%0*" LL_MASK "d", -scale, -n % f);
				}
				ZVAL_STRINGL(val,string_data,l);
			}
			break;
#endif
		case SQL_LONG:
			n = *(ISC_LONG *) data;
		_sql_long:
			if (scale == 0) {
				ZVAL_LONG(val,n);
			} else {
				zend_long f = (zend_long) scales[-scale];

				if (n >= 0) {
					l = slprintf(string_data, sizeof(string_data), ZEND_LONG_FMT ".%0*" ZEND_LONG_FMT_SPEC, n / f, -scale,  n % f);
				} else if (n <= -f) {
					l = slprintf(string_data, sizeof(string_data), ZEND_LONG_FMT ".%0*" ZEND_LONG_FMT_SPEC, n / f, -scale,  -n % f);
				} else {
					l = slprintf(string_data, sizeof(string_data), "-0.%0*" ZEND_LONG_FMT_SPEC, -scale, -n % f);
				}
				ZVAL_STRINGL(val, string_data, l);
			}
			break;
		case SQL_FLOAT:
			ZVAL_DOUBLE(val, *(float *) data);
			break;
		case SQL_DOUBLE:
			ZVAL_DOUBLE(val, *(double *) data);
			break;
#if FB_API_VER >= 40
		// These are converted to VARCHAR via isc_dpb_set_bind tag at connect
		// case SQL_DEC16:
		// case SQL_DEC34:
		// case SQL_INT128:
		case SQL_TIME_TZ:
		case SQL_TIMESTAMP_TZ:
			// Should be converted to VARCHAR via isc_dpb_set_bind tag at
			// connect if fbclient does not have fb_get_master_instance().
			// Assert this just in case.
			if(!IBG(master_instance)) {
				assert(false && "UNREACHABLE");
			}

			char timeZoneBuffer[40] = {0};
			unsigned year, month, day, hours, minutes, seconds, fractions;

			if((type & ~1) == SQL_TIME_TZ){
				format = INI_STR("ibase.timeformat");
				fb_decode_time_tz(IBG(master_instance), (ISC_TIME_TZ *) data, &hours, &minutes, &seconds, &fractions, sizeof(timeZoneBuffer), timeZoneBuffer);
				ISC_TIME time = fb_encode_time(IBG(master_instance), hours, minutes, seconds, fractions);
				isc_decode_sql_time(&time, &t);
			} else {
				format = INI_STR("ibase.timestampformat");
				fb_decode_timestamp_tz(IBG(master_instance), (ISC_TIMESTAMP_TZ *) data, &year, &month, &day, &hours, &minutes, &seconds, &fractions, sizeof(timeZoneBuffer), timeZoneBuffer);
				ISC_TIMESTAMP ts;
				ts.timestamp_date = fb_encode_date(IBG(master_instance), year, month, day);
				ts.timestamp_time = fb_encode_time(IBG(master_instance), hours, minutes, seconds, fractions);
				isc_decode_timestamp(&ts, &t);
			}

			if (((type & ~1) != SQL_TIME_TZ) && (flag & PHP_IBASE_UNIXTIME)) {
				ZVAL_LONG(val, mktime(&t));
			} else {
				char timeBuf[80] = {0};
				l = strftime(timeBuf, sizeof(timeBuf), format, &t);
				if (l == 0) {
					return FAILURE;
				}

				size_t tz_len = sprintf(string_data, "%s %s", timeBuf, timeZoneBuffer);
				ZVAL_STRINGL(val, string_data, tz_len);
			}
			break;
#endif
		case SQL_TIMESTAMP:
			format = INI_STR("ibase.timestampformat");
			isc_decode_timestamp((ISC_TIMESTAMP *) data, &t);
			goto format_date_time;
		case SQL_TYPE_DATE:
			format = INI_STR("ibase.dateformat");
			isc_decode_sql_date((ISC_DATE *) data, &t);
			goto format_date_time;
		case SQL_TYPE_TIME:
			format = INI_STR("ibase.timeformat");
			isc_decode_sql_time((ISC_TIME *) data, &t);

format_date_time:
			/*
			  XXX - Might have to remove this later - seems that isc_decode_date()
			   always sets tm_isdst to 0, sometimes incorrectly (InterBase 6 bug?)
			*/
			t.tm_isdst = -1;
#if HAVE_STRUCT_TM_TM_ZONE
			t.tm_zone = tzname[0];
#endif
			if (((type & ~1) != SQL_TYPE_TIME) && (flag & PHP_IBASE_UNIXTIME)) {
				ZVAL_LONG(val, mktime(&t));
			} else {
				l = strftime(string_data, sizeof(string_data), format, &t);
				ZVAL_STRINGL(val, string_data, l);
				break;
			}
	} /* switch (type) */
	return SUCCESS;
}
/* }}}	*/

static int _php_ibase_arr_zval(zval *ar_zval, char *data, zend_ulong data_size, /* {{{ */
	ibase_array *ib_array, int dim, size_t flag)
{
	/**
	 * Create multidimension array - recursion function
	 */
	int
		u_bound = ib_array->ar_desc.array_desc_bounds[dim].array_bound_upper,
		l_bound = ib_array->ar_desc.array_desc_bounds[dim].array_bound_lower,
		dim_len = 1 + u_bound - l_bound;
	unsigned short i;

	if (dim < ib_array->ar_desc.array_desc_dimensions) { /* array again */
		zend_ulong slice_size = data_size / dim_len;

		array_init(ar_zval);

		for (i = 0; i < dim_len; ++i) {
			zval slice_zval;

			/* recursion here */
			if (FAILURE == _php_ibase_arr_zval(&slice_zval, data, slice_size, ib_array, dim + 1,
					flag)) {
				return FAILURE;
			}
			data += slice_size;

			add_index_zval(ar_zval, l_bound + i, &slice_zval);
		}
	} else { /* data at last */

		if (FAILURE == _php_ibase_var_zval(ar_zval, data, ib_array->el_type,
				ib_array->ar_desc.array_desc_length, ib_array->ar_desc.array_desc_scale, flag)) {
			return FAILURE;
		}

		/* fix for peculiar handling of VARCHAR arrays;
		   truncate the field to the cstring length */
		if (ib_array->ar_desc.array_desc_dtype == blr_varying ||
			ib_array->ar_desc.array_desc_dtype == blr_varying2) {

			Z_STRLEN_P(ar_zval) = strlen(Z_STRVAL_P(ar_zval));
		}
	}
	return SUCCESS;
}
/* }}} */

void _php_ibase_insert_alias(HashTable *ht, const char *alias)
{
	char buf[METADATALENGTH + 3 + 1]; // _00 + \0
	zval t2;
	int i = 0;
	char const *base = "FIELD"; /* use 'FIELD' if name is empty */

	size_t alias_len = strlen(alias);
	size_t alias_len_w_suff = alias_len + 3;

	switch (*alias) {
		void *p;

		default:
			i = 1;
			base = alias;

			while ((p = zend_symtable_str_find_ptr(
					ht, alias, alias_len)) != NULL) {

		case '\0':
				// TODO: i > 99?
				snprintf(buf, sizeof(buf), "%s_%02d", base, i++);
				alias = buf;
				alias_len = alias_len_w_suff;
			}
	}

	ZVAL_NULL(&t2);
	zend_hash_str_add_new(ht, alias, alias_len, &t2);
}

static void _php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAMETERS, int fetch_type) /* {{{ */
{
	zval *res_arg, *result;
	zend_long flag = 0;
	zend_long i, array_cnt = 0;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|l", &res_arg, &flag)) {
		RETURN_FALSE;
	}

	if(_php_ibase_fetch_query_res(res_arg, &ib_query)) {
		RETURN_FALSE;
	}

	if (ib_query->out_sqlda == NULL || !ib_query->has_more_rows || !ib_query->is_open) {
		RETURN_FALSE;
	}

	assert(ib_query->out_fields_count > 0);

	if (ib_query->statement_type != isc_info_sql_stmt_exec_procedure) {
		if (isc_dsql_fetch(IB_STATUS, &ib_query->stmt, 1, ib_query->out_sqlda)) {
			ib_query->has_more_rows = 0;
			ib_query->is_open = 0;

			if (IB_STATUS[0] && IB_STATUS[1]) { /* error in fetch */
				_php_ibase_error();
			}

			if(isc_dsql_free_statement(IB_STATUS, &ib_query->stmt, DSQL_close)){
				_php_ibase_error();
			}

			RETURN_FALSE;
		}
	} else {
		ib_query->has_more_rows = 0;
		ib_query->is_open = 0;
	}

	assert(ib_query->out_fields_count == ib_query->out_sqlda->sqld);

	HashTable *ht_ret;
	if(!(fetch_type & FETCH_ROW)) {
		if(!ib_query->ht_aliases){
			if(_php_ibase_alloc_ht_aliases(ib_query)){
				_php_ibase_error();
				RETURN_FALSE;
			}
		}
		ht_ret = zend_array_dup(ib_query->ht_aliases);
	} else {
		if(!ib_query->ht_ind)_php_ibase_alloc_ht_ind(ib_query);
		ht_ret = zend_array_dup(ib_query->ht_ind);
	}

	for(i = 0; i < ib_query->out_fields_count; ++i) {
		XSQLVAR *var = &ib_query->out_sqlda->sqlvar[i];

		// NULLs are already set
		if (!(((var->sqltype & 1) == 0) || *var->sqlind != -1)) {
			zend_hash_move_forward(ht_ret);
			continue;
		}

		result = zend_hash_get_current_data(ht_ret);

		switch (var->sqltype & ~1) {

			default:
				_php_ibase_var_zval(result, var->sqldata, var->sqltype, var->sqllen,
					var->sqlscale, flag);
				break;
			case SQL_BLOB:
				if (flag & PHP_IBASE_FETCH_BLOBS) { /* fetch blob contents into hash */

					ibase_blob blob_handle;
					zend_ulong max_len = 0;
					static char bl_items[] = {isc_info_blob_total_length};
					char bl_info[20];
					unsigned short i;

					blob_handle.bl_handle = 0;
					blob_handle.bl_qd = *(ISC_QUAD *) var->sqldata;

					if (isc_open_blob(IB_STATUS, &ib_query->link->handle, &ib_query->trans->handle,
							&blob_handle.bl_handle, &blob_handle.bl_qd)) {
						_php_ibase_error();
						goto _php_ibase_fetch_error;
					}

					if (isc_blob_info(IB_STATUS, &blob_handle.bl_handle, sizeof(bl_items),
							bl_items, sizeof(bl_info), bl_info)) {
						_php_ibase_error();
						goto _php_ibase_fetch_error;
					}

					/* find total length of blob's data */
					for (i = 0; i < sizeof(bl_info); ) {
						unsigned short item_len;
						char item = bl_info[i++];

						if (item == isc_info_end || item == isc_info_truncated ||
							item == isc_info_error || i >= sizeof(bl_info)) {

							_php_ibase_module_error("Could not determine BLOB size (internal error)"
								);
							goto _php_ibase_fetch_error;
						}

						item_len = (unsigned short) isc_vax_integer(&bl_info[i], 2);

						if (item == isc_info_blob_total_length) {
							max_len = isc_vax_integer(&bl_info[i+2], item_len);
							break;
						}
						i += item_len+2;
					}

					if (max_len == 0) {
						ZVAL_STRING(result, "");
					} else if (SUCCESS != _php_ibase_blob_get(result, &blob_handle,
							max_len)) {
						goto _php_ibase_fetch_error;
					}

					if (isc_close_blob(IB_STATUS, &blob_handle.bl_handle)) {
						_php_ibase_error();
						goto _php_ibase_fetch_error;
					}

				} else { /* blob id only */
					ISC_QUAD bl_qd = *(ISC_QUAD *) var->sqldata;
					ZVAL_NEW_STR(result, _php_ibase_quad_to_string(bl_qd));
				}
				break;
			case SQL_ARRAY:
				if (flag & PHP_IBASE_FETCH_ARRAYS) { /* array can be *huge* so only fetch if asked */
					ISC_QUAD ar_qd = *(ISC_QUAD *) var->sqldata;
					ibase_array *ib_array = &ib_query->out_array[array_cnt++];
					void *ar_data = emalloc(ib_array->ar_size);

					if (isc_array_get_slice(IB_STATUS, &ib_query->link->handle,
							&ib_query->trans->handle, &ar_qd, &ib_array->ar_desc,
							ar_data, &ib_array->ar_size)) {
						_php_ibase_error();
						efree(ar_data);
						goto _php_ibase_fetch_error;
					}

					if (FAILURE == _php_ibase_arr_zval(result, ar_data, ib_array->ar_size, ib_array,
							0, flag)) {
						efree(ar_data);
						goto _php_ibase_fetch_error;
					}
					efree(ar_data);

				} else { /* blob id only */
					ISC_QUAD ar_qd = *(ISC_QUAD *) var->sqldata;
					ZVAL_NEW_STR(result, _php_ibase_quad_to_string(ar_qd));
				}
				break;
			_php_ibase_fetch_error:
				RETURN_FALSE;
		} /* switch */

		zend_hash_move_forward(ht_ret);
	}

	RETVAL_ARR(ht_ret);
}
/* }}} */

/* {{{ proto array ibase_fetch_row(resource result [, int fetch_flags])
   Fetch a row  from the results of a query */
PHP_FUNCTION(ibase_fetch_row)
{
	_php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_ROW);
}
/* }}} */

/* {{{ proto array ibase_fetch_assoc(resource result [, int fetch_flags])
   Fetch a row  from the results of a query */
PHP_FUNCTION(ibase_fetch_assoc)
{
	_php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_ARRAY);
}
/* }}} */

/* {{{ proto object ibase_fetch_object(resource result [, int fetch_flags])
   Fetch a object from the results of a query */
PHP_FUNCTION(ibase_fetch_object)
{
	_php_ibase_fetch_hash(INTERNAL_FUNCTION_PARAM_PASSTHRU, FETCH_ARRAY);

	if (Z_TYPE_P(return_value) == IS_ARRAY) {
		convert_to_object(return_value);
	}
}
/* }}} */


/* {{{ proto bool ibase_name_result(resource result, string name)
   Assign a name to a result for use with ... WHERE CURRENT OF <name> statements */
PHP_FUNCTION(ibase_name_result)
{
	zval *result_arg;
	char *name_arg;
	size_t name_arg_len;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &result_arg, &name_arg, &name_arg_len) == FAILURE) {
		return;
	}

	if(_php_ibase_fetch_query_res(result_arg, &ib_query)) {
		return;
	}

	if (isc_dsql_set_cursor_name(IB_STATUS, &ib_query->stmt, name_arg, 0)) {
		_php_ibase_error();
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool ibase_free_result(resource result)
   Free the memory used by a result */
PHP_FUNCTION(ibase_free_result)
{
	_php_ibase_free_query_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto resource ibase_prepare(resource link_identifier[, string query [, resource trans_identifier ]])
   Prepare a query for later execution */
PHP_FUNCTION(ibase_prepare)
{
	zval *link_arg, *trans_arg;
	ibase_db_link *ib_link;
	ibase_trans *trans = NULL;
	size_t query_len;
	zend_resource *trans_res = NULL;
	ibase_query *ib_query;
	char *query;

	RESET_ERRMSG;

	if (ZEND_NUM_ARGS() == 1) {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &query, &query_len) == FAILURE) {
			return;
		}
		ib_link = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink);
	} else if (ZEND_NUM_ARGS() == 2) {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &link_arg, &query, &query_len) == FAILURE) {
			return;
		}
		_php_ibase_get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, link_arg, &ib_link, &trans);

		if (trans != NULL) {
			trans_res = Z_RES_P(link_arg);
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "rrs", &link_arg, &trans_arg, &query, &query_len) == FAILURE) {
			return;
		}
		ib_link = (ibase_db_link *)zend_fetch_resource2_ex(link_arg, LE_LINK, le_link, le_plink);
		trans = (ibase_trans *)zend_fetch_resource_ex(trans_arg, LE_TRANS, le_trans);
		trans_res = Z_RES_P(trans_arg);
	}

	if (FAILURE == _php_ibase_def_trans(ib_link, &trans)) {
		RETURN_FALSE;
	}

	if(FAILURE == _php_ibase_prepare(&ib_query, ib_link, trans, trans_res, query)){
		RETURN_FALSE;
	}

	RETVAL_RES(ib_query->res);
	Z_TRY_ADDREF_P(return_value);
}
/* }}} */

/* {{{ proto mixed ibase_execute(resource query [, mixed bind_arg [, mixed bind_arg [, ...]]])
   Execute a previously prepared query */
PHP_FUNCTION(ibase_execute)
{
	zval *query, *args = NULL;
	ibase_query *ib_query;
	int bind_n = 0;

	RESET_ERRMSG;

	RETVAL_FALSE;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r*", &query, &args, &bind_n)) {
		return;
	}

	if(_php_ibase_fetch_query_res(query, &ib_query)) {
		return;
	}

	// was do {
		_php_ibase_exec(INTERNAL_FUNCTION_PARAM_PASSTHRU, ib_query, args, bind_n);

		/* free the query if trans handle was released */
		// if (ib_query->trans->handle == 0) {
		// 	zend_list_delete(Z_RES_P(query));
		// }
	// } while (0);
}
/* }}} */

/* {{{ proto bool ibase_free_query(resource query)
   Free memory used by a query */
PHP_FUNCTION(ibase_free_query)
{
	_php_ibase_free_query_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int ibase_num_fields(resource query_result)
   Get the number of fields in result */
PHP_FUNCTION(ibase_num_fields)
{
	zval *result;
	XSQLDA *sqlda;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &result) == FAILURE) {
		return;
	}

	if(_php_ibase_fetch_query_res(result, &ib_query)) {
		return;
	}

	sqlda = ib_query->out_sqlda;

	if (sqlda == NULL) {
		RETURN_LONG(0);
	} else {
		RETURN_LONG(sqlda->sqld);
	}
}
/* }}} */

static void _php_ibase_field_info(zval *return_value, ibase_query *ib_query, int is_outvar, int num) /* {{{ */
{
	unsigned short len;
	char buf[16], *s = buf;
	XSQLDA *sqlda;
	XSQLVAR *var;

	if(is_outvar){
		sqlda = ib_query->out_sqlda;
		if (sqlda == NULL) {
			_php_ibase_module_error("Trying to get field info from a non-select query");
			RETURN_FALSE;
		}
	} else {
		sqlda = ib_query->in_sqlda;
		if (sqlda == NULL) {
			// TODO: Add warning? Remove above warning?
			RETURN_FALSE;
		}
	}

	var = sqlda->sqlvar;

	if (!var || num < 0 || num >= sqlda->sqld)RETURN_FALSE;

	var += num;

	array_init(return_value);

	// AFAIK describe bind does not set sqlname, aliasname, relname?
	// Confirmation needed so I leave this as is. After that we can check
	// is_outvar

#if FB_API_VER >= 40
	if(IBG(master_instance) && IBG(get_statement_interface)) {
		void *statement = NULL;
		if(((fb_get_statement_interface_t)IBG(get_statement_interface))(IB_STATUS, &statement, &ib_query->stmt)){
			_php_ibase_error();
			RETURN_FALSE;
		}

		if(fb_insert_field_info(IBG(master_instance), IB_STATUS, is_outvar, num, return_value, statement)){
			_php_ibase_error();
			RETURN_FALSE;
		}
	} else {
#endif
		// Old API
		add_index_stringl(return_value, 0, var->sqlname, strlen(var->sqlname));
		add_assoc_stringl(return_value, "name", var->sqlname, strlen(var->sqlname));

		add_index_stringl(return_value, 1, var->aliasname, strlen(var->aliasname));
		add_assoc_stringl(return_value, "alias", var->aliasname, strlen(var->aliasname));

		add_index_stringl(return_value, 2, var->relname, strlen(var->relname));
		add_assoc_stringl(return_value, "relation", var->relname, strlen(var->relname));
#if FB_API_VER >= 40
	}
#endif

	len = slprintf(buf, 16, "%d", var->sqllen);
	add_index_stringl(return_value, 3, buf, len);
	add_assoc_stringl(return_value, "length", buf, len);

	/*
	* SQL_ consts are part of Firebird-API.
	*/

	if (var->sqlscale < 0) {
		unsigned short precision = 0;

		switch (var->sqltype & ~1) {
#ifdef SQL_BOOLEAN
			case SQL_BOOLEAN:
				precision = 1;
				break;
#endif
			case SQL_SHORT:
				precision = 4;
				break;
			case SQL_LONG:
				precision = 9;
				break;
			case SQL_INT64:
				precision = 18;
				break;
		}
		len = slprintf(buf, 16, "NUMERIC(%d,%d)", precision, -var->sqlscale);
		add_index_stringl(return_value, 4, s, len);
		add_assoc_stringl(return_value, "type", s, len);
	} else {
		switch (var->sqltype & ~1) {
			case SQL_TEXT:
				s = "CHAR";
				break;
			case SQL_VARYING:
				s = "VARCHAR";
				break;
			case SQL_SHORT:
				s = "SMALLINT";
				break;
#ifdef SQL_BOOLEAN
			case SQL_BOOLEAN:
				s = "BOOLEAN";
				break;
#endif
			case SQL_LONG:
				s = "INTEGER";
				break;
			case SQL_FLOAT:
				s = "FLOAT"; break;
			case SQL_DOUBLE:
			case SQL_D_FLOAT:
				s = "DOUBLE PRECISION"; break;
			case SQL_INT64:
				s = "BIGINT";
				break;
			case SQL_TIMESTAMP:
				s = "TIMESTAMP";
				break;
			case SQL_TYPE_DATE:
				s = "DATE";
				break;
			case SQL_TYPE_TIME:
				s = "TIME";
				break;
			case SQL_BLOB:
				s = "BLOB";
				break;
			case SQL_ARRAY:
				s = "ARRAY";
				break;
				/* FIXME: provide more detailed information about the field type, field size
				 * and array dimensions */
			case SQL_QUAD:
				s = "QUAD";
				break;
#if FB_API_VER >= 40
			// These are converted to VARCHAR via isc_dpb_set_bind tag at
			// connect and will appear to clients as VARCHAR
			// case SQL_DEC16:
			// case SQL_DEC34:
			// case SQL_INT128:
			case SQL_TIMESTAMP_TZ:
				s = "TIMESTAMP WITH TIME ZONE";
				break;
			case SQL_TIME_TZ:
				s = "TIME WITH TIME ZONE";
				break;
#endif
		}
		add_index_string(return_value, 4, s);
		add_assoc_string(return_value, "type", s);
	}
}
/* }}} */

/* {{{ proto array ibase_field_info(resource query_result, int field_number)
   Get information about a field */
PHP_FUNCTION(ibase_field_info)
{
	zval *result_arg;
	zend_long field_arg;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &result_arg, &field_arg) == FAILURE) {
		return;
	}

	if(_php_ibase_fetch_query_res(result_arg, &ib_query)) {
		return;
	}

	_php_ibase_field_info(return_value, ib_query, 1, (ISC_SHORT)field_arg);
}
/* }}} */

/* {{{ proto int ibase_num_params(resource query)
   Get the number of params in a prepared query */
PHP_FUNCTION(ibase_num_params)
{
	zval *result;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &result) == FAILURE) {
		return;
	}

	if(_php_ibase_fetch_query_res(result, &ib_query)) {
		return;
	}

	if (ib_query->in_sqlda == NULL) {
		RETURN_LONG(0);
	} else {
		RETURN_LONG(ib_query->in_sqlda->sqld);
	}
}
/* }}} */

/* {{{ proto array ibase_param_info(resource query, int field_number)
   Get information about a parameter */
PHP_FUNCTION(ibase_param_info)
{
	zval *result_arg;
	zend_long field_arg;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &result_arg, &field_arg) == FAILURE) {
		return;
	}

	if(_php_ibase_fetch_query_res(result_arg, &ib_query)) {
		return;
	}

	_php_ibase_field_info(return_value, ib_query, 0, field_arg);
}
/* }}} */

static int _php_ibase_set_query_info(ibase_query *ib_query)
{
	int rv = FAILURE;
	// size_t buf_size = 128;
	// ISC_UCHAR *buf = emalloc(buf_size);

	ISC_UCHAR buf[64] = {0};
	size_t buf_size = sizeof(buf);

	size_t pos;

	static ISC_UCHAR info_req[] = {
		isc_info_sql_stmt_type,
		isc_info_sql_select,
			isc_info_sql_num_variables,
		isc_info_sql_bind,
			isc_info_sql_num_variables,
	};
// _php_ibase_parse_info_retry:
// 	memset(buf, 0, buf_size);
	pos = 0;

	// Assume buf will be tagged with `isc_info_truncated` and later in parsing
	// we will catch that. Until `isc_info_truncated` is reached assume pos +=
	// 2, etc are safe.
	if (isc_dsql_sql_info(IB_STATUS, &ib_query->stmt, sizeof(info_req), (ISC_SCHAR *)info_req, buf_size, (ISC_SCHAR *)buf)) {
		_php_ibase_error();
		goto _php_ibase_parse_info_fail;
	}

	int ctx = 0;
	while((buf[pos] != isc_info_end) && (pos < buf_size))
	{
		const ISC_UCHAR tag = buf[pos++];
		switch(tag) {
			case isc_info_sql_stmt_type: {
				const ISC_USHORT size = (ISC_USHORT)isc_portable_integer(&buf[pos], 2); pos += 2;
				ib_query->statement_type = (ISC_UCHAR)isc_portable_integer(&buf[pos], size); pos += size;
			} break;
			case isc_info_sql_select: ctx = 1; break;
			case isc_info_sql_bind: ctx = 2; break;
			case isc_info_sql_num_variables: {
				const ISC_USHORT size = (ISC_USHORT)isc_portable_integer(&buf[pos], 2); pos += 2;
				const ISC_USHORT count = (ISC_USHORT)isc_portable_integer(&buf[pos], size); pos += size;
				if(ctx == 1) {
					ib_query->out_fields_count = count;
				} else if(ctx == 2) {
					ib_query->in_fields_count = count;
				} else {
					fbp_fatal("isc_info_sql_num_variables: unknown ctx %d", ctx);
				}
				ctx = 0;
			} break;
			case isc_info_truncated: {
				fbp_fatal("BUG: sql_info buffer truncated, current capacity: %ld", buf_size);
				// fbp_notice("BUG: sql_info buffer truncated, current capacity: %ld", buf_size);
				// Dynamic resize
				// buf_size *= 2;
				// buf = erealloc(buf, buf_size);
				// goto _php_ibase_parse_info_retry;
			} break;
			case isc_info_error: {
				fbp_fatal("sql_info buffer error, pos: %lu", pos);
				goto _php_ibase_parse_info_fail;
			} break;
			default: {
				fbp_fatal("BUG: unrecognized sql_info entry: %d, pos: %lu", tag, pos);
				goto _php_ibase_parse_info_fail;
			} break;
		}
	}

	if(buf[pos] != isc_info_end) {
		fbp_fatal("BUG: sql_info unexpected end of buffer at pos: %lu", pos);
		goto _php_ibase_parse_info_fail;
	}

	rv = SUCCESS;

_php_ibase_parse_info_fail:
	// efree(buf);
	return rv;
}

static int _php_ibase_fetch_query_res(zval *from, ibase_query **ib_query)
{
	*ib_query = zend_fetch_resource_ex(from, LE_QUERY, le_query);

	if(*ib_query == NULL) {
		// TODO: throw something or not? notice? warning?
		// fbp_notice("query already freed");
		return FAILURE;
	}

	return SUCCESS;
}

// We can't rely on aliasname coming from XSQLVAR if we want long field names
// (>31). We also can't rely on parsing buffer from isc_dsql_sql_info() because
// it's 32KB limit can be easily overflown with combination of long field names
// and large amounts of fields. So I added wrapper to use newer API but that
// also require runtime fbclient > 40 hence the runtime checks. Ideally rewrite
// everything using newer API but that's a bit of work.
static int _php_ibase_alloc_ht_aliases(ibase_query *ib_query)
{
	ALLOC_HASHTABLE(ib_query->ht_aliases);
	zend_hash_init(ib_query->ht_aliases, ib_query->out_fields_count, NULL, ZVAL_PTR_DTOR, 0);

#if FB_API_VER >= 40
	if(IBG(master_instance) && IBG(get_statement_interface)) {
		void *statement = NULL;
		if(((fb_get_statement_interface_t)IBG(get_statement_interface))(IB_STATUS, &statement, &ib_query->stmt)){
			return FAILURE;
		}

		if(fb_insert_aliases(IBG(master_instance), IB_STATUS, ib_query, statement)){
			return FAILURE;
		}
	} else {
#endif
		// Old API
		for(size_t i = 0; i < ib_query->out_fields_count; i++){
			XSQLVAR *var = &ib_query->out_sqlda->sqlvar[i];

			_php_ibase_insert_alias(ib_query->ht_aliases, var->aliasname);
		}
#if FB_API_VER >= 40
	}
#endif

	return SUCCESS;
}

static void _php_ibase_alloc_ht_ind(ibase_query *ib_query)
{
	ALLOC_HASHTABLE(ib_query->ht_ind);
	zend_hash_init(ib_query->ht_ind, ib_query->out_fields_count, NULL, ZVAL_PTR_DTOR, 0);

	zval t2;
	ZVAL_NULL(&t2);

	for(size_t i = 0; i < ib_query->out_fields_count; i++) {
		zend_hash_index_add(ib_query->ht_ind, i, &t2);
	}
}

static void _php_ibase_free_query_impl(INTERNAL_FUNCTION_PARAMETERS, int as_result)
{
	zval *query_arg;
	ibase_query *ib_query;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &query_arg) == FAILURE) {
		RETURN_FALSE;
	}

	if(_php_ibase_fetch_query_res(query_arg, &ib_query)) {
		RETURN_FALSE;
	}

	if(!as_result || ib_query->was_result_once) {
		zend_list_close(Z_RES_P(query_arg));
	}

	RETURN_TRUE;
}

#endif /* HAVE_IBASE */
