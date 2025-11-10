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
  +----------------------------------------------------------------------+
*/

#ifndef PDO_FIREBIRD_UTILS_H
#define PDO_FIREBIRD_UTILS_H

#if FB_API_VER >= 40

#include <ibase.h>
#include "php_ibase_includes.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned fbu_get_client_version(void *master_ptr);
ISC_TIME fbu_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions);
ISC_DATE fbu_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day);

void fbu_decode_time_tz(void *master_ptr, const ISC_TIME_TZ* timeTz, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
	unsigned timeZoneBufferLength, char* timeZoneBuffer);

void fbu_decode_timestamp_tz(void *master_ptr, const ISC_TIMESTAMP_TZ* timestampTz,
	unsigned* year, unsigned* month, unsigned* day,
	unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
	unsigned timeZoneBufferLength, char* timeZoneBuffer);

int fbu_insert_aliases(void *master_ptr, ISC_STATUS* st, ibase_query *ib_query, void *statement_ptr);
int fbu_insert_field_info(void *master_ptr, ISC_STATUS* st, int is_outvar, int num, zval *into_array, void *statement_ptr);

#ifdef __cplusplus
}
#endif

#endif // FB_API_VER >= 40

#endif	/* PDO_FIREBIRD_UTILS_H */
