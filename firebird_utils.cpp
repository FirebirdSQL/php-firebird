/*
  +----------------------------------------------------------------------+
  | Copyright (c) The PHP Group                                          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | https://www.php.net/license/3_01.txt                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Simonov Denis <sim-mail@list.ru>                             |
  | Author: Martins Lazdans <marrtins@dqdp.net>                          |
  +----------------------------------------------------------------------+
*/

extern "C" {
#include <ibase.h>
#include "php.h"
#include "firebird_utils.h"
#include "php_ibase_includes.h"
#include "ext/date/php_date.h"
#include "zend_interfaces.h"
}

#if FB_API_VER >= 30
#include <firebird/Interface.h>
#include <cstring>

/* Returns the client version. 0 bytes are minor version, 1 bytes are major version. */
extern "C" unsigned fbu_get_client_version(void *master_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->getClientVersion();
}

extern "C" ISC_TIME fbu_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->encodeTime(hours, minutes, seconds, fractions);
}

extern "C" ISC_DATE fbu_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->encodeDate(year, month, day);
}

extern "C" void fbu_decode_time(void *master_ptr, ISC_TIME time, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	util->decodeTime(time, hours, minutes, seconds, fractions);
}

extern "C" void fbu_decode_date(void *master_ptr, ISC_DATE date, unsigned* year, unsigned* month, unsigned* day)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	util->decodeDate(date, year, month, day);
}

#else

extern "C" ISC_TIME fbu_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions)
{
	struct tm t = {};
	t.tm_hour = (int)hours;
	t.tm_min  = (int)minutes;
	t.tm_sec  = (int)seconds;
	ISC_TIME result;
	isc_encode_sql_time(&t, &result);
	return result;
}

extern "C" ISC_DATE fbu_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day)
{
	struct tm t = {};
	t.tm_year = (int)year - 1900;
	t.tm_mon  = (int)month - 1;
	t.tm_mday = (int)day;
	ISC_DATE result;
	isc_encode_sql_date(&t, &result);
	return result;
}

extern "C" void fbu_decode_time(void *master_ptr, ISC_TIME time, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions)
{
	struct tm t = {};
	isc_decode_sql_time(&time, &t);
	*hours    = (unsigned)t.tm_hour;
	*minutes  = (unsigned)t.tm_min;
	*seconds  = (unsigned)t.tm_sec;
	*fractions = 0;
}

extern "C" void fbu_decode_date(void *master_ptr, ISC_DATE date, unsigned* year, unsigned* month, unsigned* day)
{
	struct tm t = {};
	isc_decode_sql_date(&date, &t);
	*year  = (unsigned)(t.tm_year + 1900);
	*month = (unsigned)(t.tm_mon + 1);
	*day   = (unsigned)t.tm_mday;
}

#endif // FB_API_VER >= 30

#if FB_API_VER >= 40
static void fbu_copy_status(const ISC_STATUS* from, ISC_STATUS* to, size_t maxLength)
{
	for(size_t i=0; i < maxLength; ++i) {
		memcpy(to + i, from + i, sizeof(ISC_STATUS));
		if (from[i] == isc_arg_end) {
			break;
		}
	}
}

extern "C" void fbu_encode_time_tz(void *master_ptr, ISC_TIME_TZ* timeTz, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions, const char* timeZone)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->encodeTimeTz(&st, timeTz, hours, minutes, seconds, fractions, timeZone);
}

extern "C" void fbu_encode_timestamp_tz(void *master_ptr, ISC_TIMESTAMP_TZ* timeStampTz, unsigned year, unsigned month, unsigned day, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions, const char* timeZone)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->encodeTimeStampTz(&st, timeStampTz, year, month, day, hours, minutes, seconds, fractions, timeZone);
}

/* Decodes a time with time zone into its time components. */
extern "C" void fbu_decode_time_tz(void *master_ptr, const ISC_TIME_TZ* timeTz, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
   unsigned timeZoneBufferLength, char* timeZoneBuffer)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->decodeTimeTz(&st, timeTz, hours, minutes, seconds, fractions,
						timeZoneBufferLength, timeZoneBuffer);
}

/* Decodes a timestamp with time zone into its date and time components */
extern "C" void fbu_decode_timestamp_tz(void *master_ptr, const ISC_TIMESTAMP_TZ* timestampTz,
	unsigned* year, unsigned* month, unsigned* day,
	unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
	unsigned timeZoneBufferLength, char* timeZoneBuffer)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	Firebird::IStatus* status = master->getStatus();
	Firebird::CheckStatusWrapper st(status);
	util->decodeTimeStampTz(&st, timestampTz, year, month, day,
							hours, minutes, seconds, fractions,
							timeZoneBufferLength, timeZoneBuffer);
}

extern "C" void fbu_release_statement(void *statement_ptr)
{
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	if (statement) statement->release();
}

extern "C" ISC_STATUS fbu_insert_aliases(void *master_ptr, ISC_STATUS* st, ibase_query *ib_query, void *statement_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::ThrowStatusWrapper status(master->getStatus());
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	Firebird::IMessageMetadata* meta = NULL;

	try {
		meta = statement->getOutputMetadata(&status);
		unsigned cols = meta->getCount(&status);

		assert(cols == ib_query->out_fields_count);

		for (unsigned i = 0; i < cols; ++i)
		{
			_php_ibase_insert_alias(ib_query->ht_aliases, meta->getAlias(&status, i));
		}

		meta->release();
	}
	catch (Firebird::FbException&)
	{
		if (status.hasData())  {
			fbu_copy_status((const ISC_STATUS*)status.getErrors(), st, 20);
			return st[1];
		}
	}

	status.dispose();

	return 0;
}

extern "C" ISC_STATUS fbu_insert_field_info(void *master_ptr, ISC_STATUS* st, int is_outvar, unsigned int num,
	zval *into_array, void *statement_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::ThrowStatusWrapper status(master->getStatus());
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	Firebird::IMessageMetadata* meta = NULL;

	try {
		if(is_outvar) {
			meta = statement->getOutputMetadata(&status);
		} else {
			meta = statement->getInputMetadata(&status);
		}

		add_index_string(into_array, 0, meta->getField(&status, num));
		add_assoc_string(into_array, "name", meta->getField(&status, num));

		add_index_string(into_array, 1, meta->getAlias(&status, num));
		add_assoc_string(into_array, "alias", meta->getAlias(&status, num));

		add_index_string(into_array, 2, meta->getRelation(&status, num));
		add_assoc_string(into_array, "relation", meta->getRelation(&status, num));

		meta->release();
	}
	catch (Firebird::FbException&)
	{
		if (status.hasData())  {
			fbu_copy_status((const ISC_STATUS*)status.getErrors(), st, 20);
			return st[1];
		}
	}

	status.dispose();

	return 0;
}

#endif // FB_API_VER >= 40

/*
 * Unused, kept for reference. Parses a decimal string into the scaled integer a
 * NUMERIC/DECIMAL column carries on the wire ("22.33" scale -2 -> 2233).
 */
#if 0
#ifdef _MSC_VER
#include <intrin.h>
static int u64_mul10_gt(uint64_t r, uint64_t digit, uint64_t max, uint64_t *out)
{
#ifdef _WIN64
	uint64_t hi, lo = _umul128(r, 10, &hi);
	if (hi || lo > max - digit) return 1;
	*out = lo + digit;
	return 0;
#else
	// TODO: needs testing
	/* x86: no _umul128; equivalent overflow check without 128-bit arithmetic */
	if (r > (max - digit) / 10) return 1;
	*out = r * 10 + digit;
	return 0;
#endif
}
#else
static int u64_mul10_gt(uint64_t r, uint64_t digit, uint64_t max, uint64_t *out)
{
	unsigned __int128 v = (unsigned __int128)r * 10 + digit;
	if (v > max) return 1;
	*out = (uint64_t)v;
	return 0;
}
#endif

/*
 * Parse a decimal string straight into the scaled integer that a NUMERIC/DECIMAL column
 * carries on the wire, so that binding such a parameter needs no string round-trip:
 * "22.33" with scale -2 yields sign=1, res=2233.
 *
 * `scale` is the column's sqlscale (negative), `max` the largest magnitude the target
 * integer type can hold as a positive value; the extra headroom of the negative range
 * (-32768 vs 32767) is accounted for internally.
 *
 * Returns STRNUM_PARSE_OK with *res already scaled to `scale`, or:
 *   STRNUM_PARSE_OVERFLOW - the value does not fit the target type
 *   STRNUM_PARSE_ERROR    - not a plain decimal literal this function can convert
 *                           exactly (exponent notation, whitespace, stray characters)
 *
 * Callers must treat both failures as "hand the original string to Firebird" rather than
 * as a user error: Firebird accepts more spellings than this parser does (' 5 ', '5e3',
 * '1e-2'), and reproducing its full numeric grammar here would buy nothing.
 *
 * A literal carrying more fraction digits than the column keeps ('22.335' into
 * NUMERIC(x,2)) is deliberately refused rather than rounded here. Firebird rounds half
 * away from zero and then enforces the column's *declared* precision, which XSQLVAR does
 * not expose -- rounding '99.999' into NUMERIC(4,2) has to fail, and only Firebird knows
 * that. Such literals are rare as bound parameters; letting Firebird handle them keeps
 * behaviour identical while the common exact-literal case still binds natively.
 */
extern "C" int fbu_string_to_numeric(const char *s, size_t slen, int scale, uint64_t max,
	int *sign, int *exp, uint64_t *res)
{
	const char* p = s;
	const char *end = s + slen;

	*sign = 1;
	*exp = scale;
	*res = 0;

	if (!slen) return STRNUM_PARSE_ERROR;

	if (*p == '-') {
		*sign = -1;
		p++;
	} else if (*p == '+') {
		p++;
	}

	if (p == end) return STRNUM_PARSE_ERROR; /* sign with no digits */

	/* The negative range holds one more than the positive one */
	if (*sign == -1) max += 1;

	/* Number of fraction digits the column actually stores */
	int want_frac = scale < 0 ? -scale : 0;

	uint64_t r = 0;
	int digits = 0, frac_digits = 0, seen_dot = 0;

	while (p < end) {
		char c = *p++;

		if (c == '.') {
			if (seen_dot) return STRNUM_PARSE_ERROR;
			seen_dot = 1;
			continue;
		}

		if (c < '0' || c > '9') return STRNUM_PARSE_ERROR;

		digits++;

		/* More decimals than the column keeps: needs rounding, which only Firebird can
		 * do safely here (see above) */
		if (seen_dot && frac_digits >= want_frac) return STRNUM_PARSE_ERROR;

		if (u64_mul10_gt(r, (uint64_t)(c - '0'), max, &r)) return STRNUM_PARSE_OVERFLOW;

		if (seen_dot) frac_digits++;
	}

	if (!digits) return STRNUM_PARSE_ERROR; /* "." alone */

	/* Pad to the column's scale: "22.3" into NUMERIC(x,2) is 2230, not 223 */
	while (frac_digits < want_frac) {
		if (u64_mul10_gt(r, 0, max, &r)) return STRNUM_PARSE_OVERFLOW;
		frac_digits++;
	}

	*res = r;

	return STRNUM_PARSE_OK;
}

#endif // unused: fbu_string_to_numeric()

// +-----------+------------------------+---------------------+
// | Precision | Data type              | Dialect 3           |
// +-----------+------------------------+---------------------+
// | 1 - 4     | NUMERIC                | SMALLINT            |
// | 1 - 4     | DECIMAL                | INTEGER             |
// | 5 - 9     | NUMERIC or DECIMAL     | INTEGER             |
// | 10 - 18   | NUMERIC or DECIMAL     | BIGINT              |
// | 19 - 38   | NUMERIC or DECIMAL     | INT128              |
// +-----------+------------------------+---------------------+
extern "C" const char* fbu_get_sql_type_name(XSQLVAR *var, char *buf, size_t buf_size)
{
	unsigned short precision = 0;

	if (var->sqlscale < 0) {
		switch (var->sqltype & ~1)
		{
			case SQL_SHORT  : precision = 4; break;
			case SQL_LONG   : precision = 9; break;
			case SQL_INT64  : precision = 18; break;
#if FB_API_VER >= 40
			case SQL_INT128 : precision = 38; break;
#endif
			default: fbp_fatal("BUG: unhandled type: %d with scale: %d", var->sqltype, var->sqlscale);
		}

		slprintf(buf, buf_size, "NUMERIC(%d,%d)", precision, -var->sqlscale);

		return buf;
	} else {
		switch (var->sqltype & ~1)
		{
			case SQL_TEXT:            return "CHAR";
			case SQL_VARYING:         return "VARCHAR";
			case SQL_SHORT:           return "SMALLINT";
			case SQL_LONG:            return "INTEGER";
			case SQL_INT64:           return "BIGINT";
			case SQL_FLOAT:           return "FLOAT";
			case SQL_DOUBLE:          return "DOUBLE PRECISION";
			case SQL_D_FLOAT:         return "D_FLOAT";
			case SQL_TIMESTAMP:       return "TIMESTAMP";
			case SQL_TYPE_DATE:       return "DATE";
			case SQL_TYPE_TIME:       return "TIME";
			case SQL_BLOB:            return "BLOB";
			case SQL_ARRAY:           return "ARRAY";
			case SQL_QUAD:            return "QUAD";
			case SQL_NULL:            return "NULL";
#if FB_API_VER >= 30
			case SQL_BOOLEAN:         return "BOOLEAN";
#endif
#if FB_API_VER >= 40
			case SQL_DEC16:           return "DECFLOAT(16)";
			case SQL_DEC34:           return "DECFLOAT(34)";
			case SQL_INT128:          return "INT128";
			case SQL_TIMESTAMP_TZ:    return "TIMESTAMP WITH TIME ZONE";
			case SQL_TIMESTAMP_TZ_EX: return "EXTENDED TIMESTAMP WITH TIME ZONE";
			case SQL_TIME_TZ:         return "TIME WITH TIME ZONE";
			case SQL_TIME_TZ_EX:      return "EXTENDED TIME WITH TIME ZONE";
#endif
			default:                  return "UNKNOWN";
		}
	}
}

extern "C" void fbu_init_date_object(const char *tzbuff, zval *o)
{
	zval tzo, tzo_arg;

	php_date_instantiate(php_date_get_date_ce(), o);

	if (tzbuff) {
		ZVAL_STRING(&tzo_arg, tzbuff);

		object_init_ex(&tzo, php_date_get_timezone_ce());
		#if PHP_MAJOR_VERSION >= 8
			zend_call_known_instance_method_with_1_params(
				Z_OBJCE(tzo)->constructor, Z_OBJ(tzo), NULL, &tzo_arg);
		#else
			zend_call_method_with_1_params(&tzo, php_date_get_timezone_ce(), NULL, "__construct", NULL, &tzo_arg);
		#endif

		php_date_initialize(Z_PHPDATE_P(o), "", 0, NULL, &tzo, 0);
		zval_ptr_dtor(&tzo_arg);
		zval_ptr_dtor(&tzo);
	} else {
		php_date_initialize(Z_PHPDATE_P(o), "", 0, NULL, NULL, 0);
	}

	// zend_string *tt = php_format_date(ff, strlen(ff), ((ISC_TIME_TZ *)data)->utc_time, 0);
	// php_date_time_set(&new_object, h, i, s, ms, return_value);
	// php_date_initialize_from_ts_long(date_obj, 0, 0);
}
