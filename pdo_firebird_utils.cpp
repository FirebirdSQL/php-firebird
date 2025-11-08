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

#include <ibase.h>

#if FB_API_VER >= 40

#include <firebird/Interface.h>
#include <cstring>
#include "php.h"
#include "pdo_firebird_utils.h"
#include "php_ibase_includes.h"

/* Returns the client version. 0 bytes are minor version, 1 bytes are major version. */
extern "C" unsigned fb_get_client_version(void *master_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->getClientVersion();
}

extern "C" ISC_TIME fb_encode_time(void *master_ptr, unsigned hours, unsigned minutes, unsigned seconds, unsigned fractions)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->encodeTime(hours, minutes, seconds, fractions);
}

extern "C" ISC_DATE fb_encode_date(void *master_ptr, unsigned year, unsigned month, unsigned day)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::IUtil* util = master->getUtilInterface();
	return util->encodeDate(year, month, day);
}

static void fb_copy_status(const ISC_STATUS* from, ISC_STATUS* to, size_t maxLength)
{
	for(size_t i=0; i < maxLength; ++i) {
		memcpy(to + i, from + i, sizeof(ISC_STATUS));
		if (from[i] == isc_arg_end) {
			break;
		}
	}
}

/* Decodes a time with time zone into its time components. */
extern "C" void fb_decode_time_tz(void *master_ptr, const ISC_TIME_TZ* timeTz, unsigned* hours, unsigned* minutes, unsigned* seconds, unsigned* fractions,
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
extern "C" void fb_decode_timestamp_tz(void *master_ptr, const ISC_TIMESTAMP_TZ* timestampTz,
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

extern "C" int fb_insert_aliases(void *master_ptr, ISC_STATUS* st, ibase_query *ib_query, void *statement_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::ThrowStatusWrapper status(master->getStatus());
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	Firebird::IMessageMetadata* meta = NULL;
	ISC_STATUS res;

	try {
		meta = statement->getOutputMetadata(&status);
		unsigned cols = meta->getCount(&status);

		assert(cols == ib_query->out_fields_count);

		for (unsigned i = 0; i < cols; ++i)
		{
			_php_ibase_insert_alias(ib_query->ht_aliases, meta->getAlias(&status, i));
		}
	}
	catch (const Firebird::FbException& error)
	{
		if (status.hasData())  {
			fb_copy_status((const ISC_STATUS*)status.getErrors(), st, 20);
			return st[1];
		}
	}

	return 0;
}

extern "C" int fb_insert_field_info(void *master_ptr, ISC_STATUS* st, int is_outvar, int num,
	zval *into_array, void *statement_ptr)
{
	Firebird::IMaster* master = (Firebird::IMaster*)master_ptr;
	Firebird::ThrowStatusWrapper status(master->getStatus());
	Firebird::IStatement* statement = (Firebird::IStatement *)statement_ptr;
	Firebird::IMessageMetadata* meta = NULL;
	ISC_STATUS res;

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
	}
	catch (const Firebird::FbException& error)
	{
		if (status.hasData())  {
			fb_copy_status((const ISC_STATUS*)status.getErrors(), st, 20);
			return st[1];
		}
	}

	return 0;
}

#endif // FB_API_VER >= 40
