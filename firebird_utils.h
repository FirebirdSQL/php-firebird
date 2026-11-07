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

#ifndef PDO_FIREBIRD_UTILS_H
#define PDO_FIREBIRD_UTILS_H

#define STRNUM_PARSE_OK       0
#define STRNUM_PARSE_ERROR    1
#define STRNUM_PARSE_OVERFLOW 2

#ifdef __cplusplus
extern "C" {
#endif



#include <ibase.h>
#include "php_ibase_includes.h"

ISC_TIME fbu_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions);
ISC_DATE fbu_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day);
void fbu_decode_time(void *master_ptr, ISC_TIME time, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions);
void fbu_decode_date(void *master_ptr, ISC_DATE date, unsigned* year, unsigned* month, unsigned* day);

#if FB_API_VER >= 30
unsigned fbu_get_client_version(void *master_ptr);
#endif // FB_API_VER >= 30


#if FB_API_VER >= 40
void fbu_encode_time_tz(void *master_ptr, ISC_TIME_TZ* timeTz, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions, const char* timeZone);
void fbu_encode_timestamp_tz(void *master_ptr, ISC_TIMESTAMP_TZ* timeStampTz, unsigned year, unsigned month, unsigned day, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions,
	const char* timeZone);
void fbu_decode_time_tz(void *master_ptr, const ISC_TIME_TZ* timeTz, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
	unsigned timeZoneBufferLength, char* timeZoneBuffer);
void fbu_decode_timestamp_tz(void *master_ptr, const ISC_TIMESTAMP_TZ* timestampTz,
	unsigned* year, unsigned* month, unsigned* day,
	unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
	unsigned timeZoneBufferLength, char* timeZoneBuffer);
ISC_STATUS fbu_insert_field_info(void *master_ptr, ISC_STATUS* st, int is_outvar, unsigned int num, zval *into_array, void *statement_ptr);
ISC_STATUS fbu_insert_aliases(void *master_ptr, ISC_STATUS* st, ibase_query *ib_query, void *statement_ptr);
void fbu_release_statement(void *statement_ptr);
#endif // FB_API_VER >= 4

/* Unused; see the note on the definition in firebird_utils.cpp before reviving it.
int fbu_string_to_numeric(const char *s, size_t slen, int scale, uint64_t max,
	int *sign, int *exp, uint64_t *res); */
const char* fbu_get_sql_type_name(XSQLVAR *var, char *buf, size_t buf_size);
void fbu_init_date_object(const char *tzbuff, zval *o);

#ifdef __cplusplus
}
#endif

#endif	/* PDO_FIREBIRD_UTILS_H */
