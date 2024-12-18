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
   | Authors: Ard Biesheuvel <a.k.biesheuvel@its.tudelft.nl>              |
   |          Martin Köditz <martin.koeditz@it-syn.de>                    |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"

/*
* Since PHP 8 we have troubles building sources on Windows,
* because of missing TSRMLS_FETCH_FROM_CTX and TSRMLS_SET_CTX symbols.
*/
#if PHP_VERSION_ID >= 80000
# define FBIRD_TSRMLS_FETCH_FROM_CTX(user_data)
# define FBIRD_TSRMLS_SET_CTX(user_data)
#else
# define FBIRD_TSRMLS_FETCH_FROM_CTX(user_data) TSRMLS_FETCH_FROM_CTX(user_data)
# define FBIRD_TSRMLS_SET_CTX(user_data) TSRMLS_SET_CTX(user_data)
#endif

#if HAVE_IBASE

#include "php_interbase.h"
#include "php_ibase_includes.h"

static int le_event;

static void _php_ibase_event_free(unsigned char *event_buf, unsigned char *result_buf) /* {{{ */
{
	isc_free(event_buf);
	isc_free(result_buf);
}
/* }}} */

void _php_ibase_free_event(ibase_event *event) /* {{{ */
{
	unsigned short i;

	event->state = DEAD;

	if (event->link != NULL) {
		ibase_event **node;

		zend_list_delete(event->link_res);
		if (event->link->handle != 0 &&
				isc_cancel_events(IB_STATUS, &event->link->handle, &event->event_id)) {
			_php_ibase_error();
		}

		/* delete this event from the link struct */
		for (node = &event->link->event_head; *node != event; node = &(*node)->event_next);
		*node = event->event_next;
	}

	if (Z_TYPE(event->callback) != IS_UNDEF) {
		zval_ptr_dtor(&event->callback);
		ZVAL_UNDEF(&event->callback);

		_php_ibase_event_free(event->event_buffer,event->result_buffer);

		for (i = 0; i < event->event_count; ++i) {
			if (event->events[i]) {
				efree(event->events[i]);
			}
		}
		efree(event->events);
	}
}
/* }}} */

static void _php_ibase_free_event_rsrc(zend_resource *rsrc) /* {{{ */
{
	ibase_event *e = (ibase_event *) rsrc->ptr;

	_php_ibase_free_event(e);
	efree(e);
}
/* }}} */

void php_ibase_events_minit(INIT_FUNC_ARGS) /* {{{ */
{
	le_event = zend_register_list_destructors_ex(_php_ibase_free_event_rsrc, NULL,
	    "interbase event", module_number);
}
/* }}} */

static void _php_ibase_event_block(ibase_db_link *ib_link, unsigned short count,
    char **events, unsigned short *l, unsigned char **event_buf, unsigned char **result_buf)
{
    ISC_STATUS dummy_result[20];
    ISC_ULONG dummy_count[15];

    /**
	 * Unfortunately, there's no clean and portable way in C to pass arguments to
	 * a variadic function if you don't know the number of arguments at compile time.
	 * (And even if there were a way, the Interbase API doesn't provide a version of
	 * this function that takes a va_list as an argument)
	 *
	 * In this case, the number of arguments is limited to 18 by the underlying API,
	 * so we can work around it.
	 */

    *l = (unsigned short) isc_event_block(event_buf, result_buf, count, events[0],
        events[1], events[2], events[3], events[4], events[5], events[6], events[7],
        events[8], events[9], events[10], events[11], events[12], events[13], events[14]);

    /**
	 * Currently, this is the only way to correctly initialize an event buffer.
	 * This is clearly something that should be fixed, cause the semantics of
	 * isc_wait_for_event() indicate that it blocks until an event occurs.
	 * If the Firebird people ever fix this, these lines should be removed,
	 * otherwise, events will have to fire twice before ibase_wait_event() returns.
	 */

    isc_wait_for_event(dummy_result, &ib_link->handle, *l, *event_buf, *result_buf);
    isc_event_counts(dummy_count, *l, *event_buf, *result_buf);
}
/* }}} */

/* {{{ proto string ibase_wait_event([resource link_identifier,] string event [, string event [, ...]])
   Waits for any one of the passed Interbase events to be posted by the database, and returns its name */
PHP_FUNCTION(ibase_wait_event)
{
	zval *args;
	ibase_db_link *ib_link;
	int num_args;
	unsigned char *event_buffer, *result_buffer;
	char *events[15];
	unsigned short i = 0, event_count = 0, buffer_size;
	ISC_ULONG occurred_event[15];

	RESET_ERRMSG;

	/* no more than 15 events */
	if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 16) {
		WRONG_PARAM_COUNT;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &num_args) == FAILURE) {
		return;
	}

	if (Z_TYPE(args[0]) == IS_RESOURCE) {
		if ((ib_link = (ibase_db_link *)zend_fetch_resource2_ex(&args[0], "InterBase link", le_link, le_plink)) == NULL) {
			RETURN_FALSE;
		}
		i = 1;
	} else {
		if (ZEND_NUM_ARGS() > 15) {
			WRONG_PARAM_COUNT;
		}
		if ((ib_link = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), "InterBase link", le_link, le_plink)) == NULL) {
			RETURN_FALSE;
		}
	}

	for (; i < ZEND_NUM_ARGS(); ++i) {
		convert_to_string_ex(&args[i]);
		events[event_count++] = Z_STRVAL(args[i]);
	}

	/* fills the required data structure with information about the events */
	_php_ibase_event_block(ib_link, event_count, events, &buffer_size, &event_buffer, &result_buffer);

	/* now block until an event occurs */
	if (isc_wait_for_event(IB_STATUS, &ib_link->handle, buffer_size, event_buffer, result_buffer)) {
		_php_ibase_error();
		_php_ibase_event_free(event_buffer,result_buffer);
		RETURN_FALSE;
	}

	/* find out which event occurred */
	isc_event_counts(occurred_event, buffer_size, event_buffer, result_buffer);
	for (i = 0; i < event_count; ++i) {
		if (occurred_event[i]) {
			zend_string *result = zend_string_init(events[i], strlen(events[i]), 0);
			_php_ibase_event_free(event_buffer,result_buffer);
			RETURN_STR(result);
		}
	}

	/* If we reach this line, isc_wait_for_event() did return, but we don't know
	   which event fired. */
	_php_ibase_event_free(event_buffer,result_buffer);
	RETURN_FALSE;
}
/* }}} */

#if FB_API_VER >= 20
#define PHP_ISC_CALLBACK ISC_EVENT_CALLBACK
static ISC_EVENT_CALLBACK _php_ibase_callback(ibase_event *event, /* {{{ */
	ISC_USHORT buffer_size, ISC_UCHAR *result_buf)
#else
#define PHP_ISC_CALLBACK isc_callback
static isc_callback  _php_ibase_callback(ibase_event *event, /* {{{ */
	unsigned short buffer_size, unsigned char *result_buf)
#endif
{
	/* this function is called asynchronously by the Interbase client library. */
	FBIRD_TSRMLS_FETCH_FROM_CTX(event->thread_ctx);

	/**
	 * The callback function is called when the event is first registered and when the event
	 * is cancelled. I consider this is a bug. By clearing event->callback first and setting
	 * it to -1 later, we make sure nothing happens if no event was actually posted.
	 */
	switch (event->state) {
		unsigned short i;
		ISC_ULONG occurred_event[15];
		zval return_value, args[2];

		default: /* == DEAD */
			break;
		case ACTIVE:
			/* copy the updated results into the result buffer */
			memcpy(event->result_buffer, result_buf, buffer_size);

			ZVAL_RES(&args[1], event->link_res);

			/* find out which event occurred */
			isc_event_counts(occurred_event, buffer_size, event->event_buffer, event->result_buffer);
			for (i = 0; i < event->event_count; ++i) {
				if (occurred_event[i]) {
					ZVAL_STRING(&args[0], event->events[i]);
					//efree(event->events[i]);
					break;
				}
			}

			/* call the callback provided by the user */
			if (SUCCESS != call_user_function(NULL, NULL,
					&event->callback, &return_value, 2, args)) {
				_php_ibase_module_error("Error calling callback %s", Z_STRVAL(event->callback));
				break;
			}

			if (Z_TYPE(return_value) == IS_FALSE) {
				event->state = DEAD;
				break;
			}
		case NEW:
			/* re-register the event */
			if (isc_que_events(IB_STATUS, &event->link->handle, &event->event_id, buffer_size,
				event->event_buffer,(PHP_ISC_CALLBACK)_php_ibase_callback, (void *)event)) {

				_php_ibase_error();
			}
			event->state = ACTIVE;
	}
	return 0;
}
/* }}} */

/* {{{ proto resource ibase_set_event_handler([resource link_identifier,] callback handler, string event [, string event [, ...]])
   Register the callback for handling each of the named events */
PHP_FUNCTION(ibase_set_event_handler)
{
	/**
	 * The callback passed to this function should take an event name (string) and a
	 * link resource id (int) as arguments. The value returned from the function is
	 * used to determine if the event handler should remain set.
	 */
	zval *args, *cb_arg;
	ibase_db_link *ib_link;
	ibase_event *event;
	unsigned short i = 1, buffer_size;
	int num_args;
	zend_resource *link_res;

	RESET_ERRMSG;

	/* Minimum and maximum number of arguments allowed */
	if (ZEND_NUM_ARGS() < 2 || ZEND_NUM_ARGS() > 17) {
		WRONG_PARAM_COUNT;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "+", &args, &num_args) == FAILURE) {
		return;
	}

	/* get a working link */
	if (Z_TYPE(args[0]) != IS_STRING) {
		/* resource, callback, event_1 [, ... event_15]
		 * No more than 15 events
		 */
		if (ZEND_NUM_ARGS() < 3 || ZEND_NUM_ARGS() > 17) {
			WRONG_PARAM_COUNT;
		}

		cb_arg = &args[1];
		i = 2;

		if ((ib_link = (ibase_db_link *)zend_fetch_resource2_ex(&args[0], "InterBase link", le_link, le_plink)) == NULL) {
			RETURN_FALSE;
		}

		link_res = Z_RES(args[0]);

	} else {
		/* callback, event_1 [, ... event_15]
		 * No more than 15 events
		 */
		if (ZEND_NUM_ARGS() < 2 || ZEND_NUM_ARGS() > 16) {
			WRONG_PARAM_COUNT;
		}

		cb_arg = &args[0];

		if ((ib_link = (ibase_db_link *)zend_fetch_resource2(IBG(default_link), "InterBase link", le_link, le_plink)) == NULL) {
			RETURN_FALSE;
		}
		link_res = IBG(default_link);
	}

	/* get the callback */
	if (!zend_is_callable(cb_arg, 0, NULL)) {
		zend_string *cb_name = zend_get_callable_name(cb_arg);
		_php_ibase_module_error("Callback argument %s is not a callable function", ZSTR_VAL(cb_name));
		zend_string_release_ex(cb_name, 0);
		RETURN_FALSE;
	}

	/* allocate the event resource */
	event = (ibase_event *) safe_emalloc(sizeof(ibase_event), 1, 0);
	FBIRD_TSRMLS_SET_CTX(event->thread_ctx);
	event->link_res = link_res;
	GC_ADDREF(link_res);
	event->link = ib_link;
	event->event_count = 0;
	event->state = NEW;
	event->events = (char **) safe_emalloc(sizeof(char *), 15, 0);

	ZVAL_DUP(&event->callback, cb_arg);

	for (; i < 15; ++i) {
		if (i < ZEND_NUM_ARGS()) {
			convert_to_string_ex(&args[i]);
			event->events[event->event_count++] = estrdup(Z_STRVAL(args[i]));
		} else {
			event->events[i] = NULL;
		}
	}

	/* fills the required data structure with information about the events */
	_php_ibase_event_block(ib_link, event->event_count, event->events,
		&buffer_size, &event->event_buffer, &event->result_buffer);

	/* now register the events with the Interbase API */
	if (isc_que_events(IB_STATUS, &ib_link->handle, &event->event_id, buffer_size,
		event->event_buffer,(PHP_ISC_CALLBACK)_php_ibase_callback, (void *)event)) {

		_php_ibase_error();
		efree(event);
		RETURN_FALSE;
	}

	event->event_next = ib_link->event_head;
	ib_link->event_head = event;

	RETVAL_RES(zend_register_resource(event, le_event));
	Z_TRY_ADDREF_P(return_value);
}
/* }}} */

/* {{{ proto bool ibase_free_event_handler(resource event)
   Frees the event handler set by ibase_set_event_handler() */
PHP_FUNCTION(ibase_free_event_handler)
{
	zval *event_arg;

	RESET_ERRMSG;

	if (SUCCESS == zend_parse_parameters(ZEND_NUM_ARGS(), "r", &event_arg)) {
		ibase_event *event;

		event = (ibase_event *)zend_fetch_resource_ex(event_arg, "Interbase event", le_event);

		event->state = DEAD;

		zend_list_delete(Z_RES_P(event_arg));
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

#endif /* HAVE_IBASE */
