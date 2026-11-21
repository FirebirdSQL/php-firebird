/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a3a92a17f7d509f4eefd6f423eeff37a59279322 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_connect, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, database, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, username, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, password, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, charset, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, buffers, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dialect, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, role, IS_STRING, 0, "\"\"")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, sync, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_ibase_pconnect arginfo_ibase_connect

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_close, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, link_identifier, "null")
ZEND_END_ARG_INFO()

#define arginfo_ibase_drop_db arginfo_ibase_close

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_query, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, bind_arg, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_fetch_row, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fetch_flags, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

#define arginfo_ibase_fetch_assoc arginfo_ibase_fetch_row

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_TYPE_MASK_EX(arginfo_ibase_fetch_object, 0, 1, stdClass, MAY_BE_FALSE)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, fetch_flags, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_free_result, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_name_result, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_prepare, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, trans_query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_execute, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, bind_arg, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_free_query, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_gen_id, 0, 1, MAY_BE_LONG|MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_TYPE_INFO(0, generator, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, increment, IS_LONG, 0, "1")
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, link_identifier, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_num_fields, 0, 1, IS_LONG, 0)
	ZEND_ARG_INFO(0, query_result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_num_params, 0, 1, IS_LONG, 0)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_affected_rows, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, link_identifier, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_query_affected_rows, 0, 1, MAY_BE_LONG|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_field_info, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, query_result)
	ZEND_ARG_TYPE_INFO(0, field_number, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_param_info, 0, 2, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_TYPE_INFO(0, field_number, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_trans, 0, 0, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, trans_args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_ibase_commit arginfo_ibase_close

#define arginfo_ibase_rollback arginfo_ibase_close

#define arginfo_ibase_commit_ret arginfo_ibase_close

#define arginfo_ibase_rollback_ret arginfo_ibase_close

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_blob_info, 0, 1, MAY_BE_ARRAY|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, blob_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_blob_create, 0, 0, 0)
	ZEND_ARG_INFO_WITH_DEFAULT_VALUE(0, link_identifier, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_blob_add, 0, 2, _IS_BOOL, 1)
	ZEND_ARG_INFO(0, blob_handle)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_blob_cancel, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, blob_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_blob_close, 0, 1, MAY_BE_STRING|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, blob_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_blob_open, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, blob_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_blob_get, 0, 2, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, blob_handle)
	ZEND_ARG_TYPE_INFO(0, len, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_blob_echo, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, blob_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_blob_import, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_errmsg, 0, 0, MAY_BE_STRING|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_errcode, 0, 0, MAY_BE_LONG|MAY_BE_FALSE)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_add_user, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, user_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, password, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, first_name, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, middle_name, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, last_name, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

#define arginfo_ibase_modify_user arginfo_ibase_add_user

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_delete_user, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, user_name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_service_attach, 0, 0, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, host, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dba_username, IS_STRING, 1, "null")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, dba_password, IS_STRING, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_service_detach, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, service_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_backup, 0, 3, MAY_BE_STRING|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, source_db, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, dest_file, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, verbose, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_restore, 0, 3, MAY_BE_STRING|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, source_file, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, dest_db, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "0")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, verbose, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_maintain_db, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, db, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, action, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, argument, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_db_info, 0, 3, MAY_BE_STRING|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, db, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, action, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, argument, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_server_info, 0, 2, MAY_BE_STRING|MAY_BE_ARRAY|MAY_BE_BOOL)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_TYPE_INFO(0, action, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_MASK_EX(arginfo_ibase_wait_event, 0, 1, MAY_BE_STRING|MAY_BE_FALSE)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, event, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ibase_set_event_handler, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, handler)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, event, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_free_event_handler, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_INFO(0, event)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ibase_get_client_version, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

#define arginfo_ibase_get_client_major_version arginfo_ibase_get_client_version

#define arginfo_ibase_get_client_minor_version arginfo_ibase_get_client_version

#define arginfo_fbird_connect arginfo_ibase_connect

#define arginfo_fbird_pconnect arginfo_ibase_connect

#define arginfo_fbird_close arginfo_ibase_close

#define arginfo_fbird_drop_db arginfo_ibase_close

#define arginfo_fbird_query arginfo_ibase_query

#define arginfo_fbird_fetch_row arginfo_ibase_fetch_row

#define arginfo_fbird_fetch_assoc arginfo_ibase_fetch_row

#define arginfo_fbird_fetch_object arginfo_ibase_fetch_object

#define arginfo_fbird_free_result arginfo_ibase_free_result

#define arginfo_fbird_name_result arginfo_ibase_name_result

#define arginfo_fbird_prepare arginfo_ibase_prepare

#define arginfo_fbird_execute arginfo_ibase_execute

#define arginfo_fbird_free_query arginfo_ibase_free_query

#define arginfo_fbird_gen_id arginfo_ibase_gen_id

#define arginfo_fbird_num_fields arginfo_ibase_num_fields

#define arginfo_fbird_num_params arginfo_ibase_num_params

#define arginfo_fbird_affected_rows arginfo_ibase_affected_rows

#define arginfo_fbird_query_affected_rows arginfo_ibase_query_affected_rows

#define arginfo_fbird_field_info arginfo_ibase_field_info

#define arginfo_fbird_param_info arginfo_ibase_param_info

#define arginfo_fbird_trans arginfo_ibase_trans

#define arginfo_fbird_commit arginfo_ibase_close

#define arginfo_fbird_rollback arginfo_ibase_close

#define arginfo_fbird_commit_ret arginfo_ibase_close

#define arginfo_fbird_rollback_ret arginfo_ibase_close

#define arginfo_fbird_blob_info arginfo_ibase_blob_info

#define arginfo_fbird_blob_create arginfo_ibase_blob_create

#define arginfo_fbird_blob_add arginfo_ibase_blob_add

#define arginfo_fbird_blob_cancel arginfo_ibase_blob_cancel

#define arginfo_fbird_blob_close arginfo_ibase_blob_close

#define arginfo_fbird_blob_open arginfo_ibase_blob_open

#define arginfo_fbird_blob_get arginfo_ibase_blob_get

#define arginfo_fbird_blob_echo arginfo_ibase_blob_echo

#define arginfo_fbird_blob_import arginfo_ibase_blob_import

#define arginfo_fbird_errmsg arginfo_ibase_errmsg

#define arginfo_fbird_errcode arginfo_ibase_errcode

#define arginfo_fbird_add_user arginfo_ibase_add_user

#define arginfo_fbird_modify_user arginfo_ibase_add_user

#define arginfo_fbird_delete_user arginfo_ibase_delete_user

#define arginfo_fbird_service_attach arginfo_ibase_service_attach

#define arginfo_fbird_service_detach arginfo_ibase_service_detach

#define arginfo_fbird_backup arginfo_ibase_backup

#define arginfo_fbird_restore arginfo_ibase_restore

#define arginfo_fbird_maintain_db arginfo_ibase_maintain_db

#define arginfo_fbird_db_info arginfo_ibase_db_info

#define arginfo_fbird_server_info arginfo_ibase_server_info

#define arginfo_fbird_wait_event arginfo_ibase_wait_event

#define arginfo_fbird_set_event_handler arginfo_ibase_set_event_handler

#define arginfo_fbird_free_event_handler arginfo_ibase_free_event_handler

#define arginfo_fbird_get_client_version arginfo_ibase_get_client_version

#define arginfo_fbird_get_client_major_version arginfo_ibase_get_client_version

#define arginfo_fbird_get_client_minor_version arginfo_ibase_get_client_version

ZEND_FUNCTION(ibase_connect);
ZEND_FUNCTION(ibase_pconnect);
ZEND_FUNCTION(ibase_close);
ZEND_FUNCTION(ibase_drop_db);
ZEND_FUNCTION(ibase_query);
ZEND_FUNCTION(ibase_fetch_row);
ZEND_FUNCTION(ibase_fetch_assoc);
ZEND_FUNCTION(ibase_fetch_object);
ZEND_FUNCTION(ibase_free_result);
ZEND_FUNCTION(ibase_name_result);
ZEND_FUNCTION(ibase_prepare);
ZEND_FUNCTION(ibase_execute);
ZEND_FUNCTION(ibase_free_query);
ZEND_FUNCTION(ibase_gen_id);
ZEND_FUNCTION(ibase_num_fields);
ZEND_FUNCTION(ibase_num_params);
ZEND_FUNCTION(ibase_affected_rows);
ZEND_FUNCTION(ibase_query_affected_rows);
ZEND_FUNCTION(ibase_field_info);
ZEND_FUNCTION(ibase_param_info);
ZEND_FUNCTION(ibase_trans);
ZEND_FUNCTION(ibase_commit);
ZEND_FUNCTION(ibase_rollback);
ZEND_FUNCTION(ibase_commit_ret);
ZEND_FUNCTION(ibase_rollback_ret);
ZEND_FUNCTION(ibase_blob_info);
ZEND_FUNCTION(ibase_blob_create);
ZEND_FUNCTION(ibase_blob_add);
ZEND_FUNCTION(ibase_blob_cancel);
ZEND_FUNCTION(ibase_blob_close);
ZEND_FUNCTION(ibase_blob_open);
ZEND_FUNCTION(ibase_blob_get);
ZEND_FUNCTION(ibase_blob_echo);
ZEND_FUNCTION(ibase_blob_import);
ZEND_FUNCTION(ibase_errmsg);
ZEND_FUNCTION(ibase_errcode);
ZEND_FUNCTION(ibase_add_user);
ZEND_FUNCTION(ibase_modify_user);
ZEND_FUNCTION(ibase_delete_user);
ZEND_FUNCTION(ibase_service_attach);
ZEND_FUNCTION(ibase_service_detach);
ZEND_FUNCTION(ibase_backup);
ZEND_FUNCTION(ibase_restore);
ZEND_FUNCTION(ibase_maintain_db);
ZEND_FUNCTION(ibase_db_info);
ZEND_FUNCTION(ibase_server_info);
ZEND_FUNCTION(ibase_wait_event);
ZEND_FUNCTION(ibase_set_event_handler);
ZEND_FUNCTION(ibase_free_event_handler);
ZEND_FUNCTION(ibase_get_client_version);
ZEND_FUNCTION(ibase_get_client_major_version);
ZEND_FUNCTION(ibase_get_client_minor_version);

static const zend_function_entry ext_functions[] = {
	ZEND_FE(ibase_connect, arginfo_ibase_connect)
	ZEND_FE(ibase_pconnect, arginfo_ibase_pconnect)
	ZEND_FE(ibase_close, arginfo_ibase_close)
	ZEND_FE(ibase_drop_db, arginfo_ibase_drop_db)
	ZEND_FE(ibase_query, arginfo_ibase_query)
	ZEND_FE(ibase_fetch_row, arginfo_ibase_fetch_row)
	ZEND_FE(ibase_fetch_assoc, arginfo_ibase_fetch_assoc)
	ZEND_FE(ibase_fetch_object, arginfo_ibase_fetch_object)
	ZEND_FE(ibase_free_result, arginfo_ibase_free_result)
	ZEND_FE(ibase_name_result, arginfo_ibase_name_result)
	ZEND_FE(ibase_prepare, arginfo_ibase_prepare)
	ZEND_FE(ibase_execute, arginfo_ibase_execute)
	ZEND_FE(ibase_free_query, arginfo_ibase_free_query)
	ZEND_FE(ibase_gen_id, arginfo_ibase_gen_id)
	ZEND_FE(ibase_num_fields, arginfo_ibase_num_fields)
	ZEND_FE(ibase_num_params, arginfo_ibase_num_params)
	ZEND_FE(ibase_affected_rows, arginfo_ibase_affected_rows)
	ZEND_FE(ibase_query_affected_rows, arginfo_ibase_query_affected_rows)
	ZEND_FE(ibase_field_info, arginfo_ibase_field_info)
	ZEND_FE(ibase_param_info, arginfo_ibase_param_info)
	ZEND_FE(ibase_trans, arginfo_ibase_trans)
	ZEND_FE(ibase_commit, arginfo_ibase_commit)
	ZEND_FE(ibase_rollback, arginfo_ibase_rollback)
	ZEND_FE(ibase_commit_ret, arginfo_ibase_commit_ret)
	ZEND_FE(ibase_rollback_ret, arginfo_ibase_rollback_ret)
	ZEND_FE(ibase_blob_info, arginfo_ibase_blob_info)
	ZEND_FE(ibase_blob_create, arginfo_ibase_blob_create)
	ZEND_FE(ibase_blob_add, arginfo_ibase_blob_add)
	ZEND_FE(ibase_blob_cancel, arginfo_ibase_blob_cancel)
	ZEND_FE(ibase_blob_close, arginfo_ibase_blob_close)
	ZEND_FE(ibase_blob_open, arginfo_ibase_blob_open)
	ZEND_FE(ibase_blob_get, arginfo_ibase_blob_get)
	ZEND_FE(ibase_blob_echo, arginfo_ibase_blob_echo)
	ZEND_FE(ibase_blob_import, arginfo_ibase_blob_import)
	ZEND_FE(ibase_errmsg, arginfo_ibase_errmsg)
	ZEND_FE(ibase_errcode, arginfo_ibase_errcode)
	ZEND_FE(ibase_add_user, arginfo_ibase_add_user)
	ZEND_FE(ibase_modify_user, arginfo_ibase_modify_user)
	ZEND_FE(ibase_delete_user, arginfo_ibase_delete_user)
	ZEND_FE(ibase_service_attach, arginfo_ibase_service_attach)
	ZEND_FE(ibase_service_detach, arginfo_ibase_service_detach)
	ZEND_FE(ibase_backup, arginfo_ibase_backup)
	ZEND_FE(ibase_restore, arginfo_ibase_restore)
	ZEND_FE(ibase_maintain_db, arginfo_ibase_maintain_db)
	ZEND_FE(ibase_db_info, arginfo_ibase_db_info)
	ZEND_FE(ibase_server_info, arginfo_ibase_server_info)
	ZEND_FE(ibase_wait_event, arginfo_ibase_wait_event)
	ZEND_FE(ibase_set_event_handler, arginfo_ibase_set_event_handler)
	ZEND_FE(ibase_free_event_handler, arginfo_ibase_free_event_handler)
	ZEND_FE(ibase_get_client_version, arginfo_ibase_get_client_version)
	ZEND_FE(ibase_get_client_major_version, arginfo_ibase_get_client_major_version)
	ZEND_FE(ibase_get_client_minor_version, arginfo_ibase_get_client_minor_version)
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_connect", zif_ibase_connect, arginfo_fbird_connect, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_connect", zif_ibase_connect, arginfo_fbird_connect, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_pconnect", zif_ibase_pconnect, arginfo_fbird_pconnect, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_pconnect", zif_ibase_pconnect, arginfo_fbird_pconnect, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_close", zif_ibase_close, arginfo_fbird_close, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_close", zif_ibase_close, arginfo_fbird_close, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_drop_db", zif_ibase_drop_db, arginfo_fbird_drop_db, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_drop_db", zif_ibase_drop_db, arginfo_fbird_drop_db, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_query", zif_ibase_query, arginfo_fbird_query, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_query", zif_ibase_query, arginfo_fbird_query, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_fetch_row", zif_ibase_fetch_row, arginfo_fbird_fetch_row, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_fetch_row", zif_ibase_fetch_row, arginfo_fbird_fetch_row, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_fetch_assoc", zif_ibase_fetch_assoc, arginfo_fbird_fetch_assoc, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_fetch_assoc", zif_ibase_fetch_assoc, arginfo_fbird_fetch_assoc, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_fetch_object", zif_ibase_fetch_object, arginfo_fbird_fetch_object, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_fetch_object", zif_ibase_fetch_object, arginfo_fbird_fetch_object, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_free_result", zif_ibase_free_result, arginfo_fbird_free_result, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_free_result", zif_ibase_free_result, arginfo_fbird_free_result, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_name_result", zif_ibase_name_result, arginfo_fbird_name_result, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_name_result", zif_ibase_name_result, arginfo_fbird_name_result, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_prepare", zif_ibase_prepare, arginfo_fbird_prepare, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_prepare", zif_ibase_prepare, arginfo_fbird_prepare, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_execute", zif_ibase_execute, arginfo_fbird_execute, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_execute", zif_ibase_execute, arginfo_fbird_execute, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_free_query", zif_ibase_free_query, arginfo_fbird_free_query, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_free_query", zif_ibase_free_query, arginfo_fbird_free_query, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_gen_id", zif_ibase_gen_id, arginfo_fbird_gen_id, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_gen_id", zif_ibase_gen_id, arginfo_fbird_gen_id, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_num_fields", zif_ibase_num_fields, arginfo_fbird_num_fields, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_num_fields", zif_ibase_num_fields, arginfo_fbird_num_fields, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_num_params", zif_ibase_num_params, arginfo_fbird_num_params, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_num_params", zif_ibase_num_params, arginfo_fbird_num_params, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_affected_rows", zif_ibase_affected_rows, arginfo_fbird_affected_rows, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_affected_rows", zif_ibase_affected_rows, arginfo_fbird_affected_rows, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_query_affected_rows", zif_ibase_query_affected_rows, arginfo_fbird_query_affected_rows, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_query_affected_rows", zif_ibase_query_affected_rows, arginfo_fbird_query_affected_rows, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_field_info", zif_ibase_field_info, arginfo_fbird_field_info, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_field_info", zif_ibase_field_info, arginfo_fbird_field_info, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_param_info", zif_ibase_param_info, arginfo_fbird_param_info, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_param_info", zif_ibase_param_info, arginfo_fbird_param_info, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_trans", zif_ibase_trans, arginfo_fbird_trans, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_trans", zif_ibase_trans, arginfo_fbird_trans, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_commit", zif_ibase_commit, arginfo_fbird_commit, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_commit", zif_ibase_commit, arginfo_fbird_commit, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_rollback", zif_ibase_rollback, arginfo_fbird_rollback, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_rollback", zif_ibase_rollback, arginfo_fbird_rollback, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_commit_ret", zif_ibase_commit_ret, arginfo_fbird_commit_ret, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_commit_ret", zif_ibase_commit_ret, arginfo_fbird_commit_ret, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_rollback_ret", zif_ibase_rollback_ret, arginfo_fbird_rollback_ret, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_rollback_ret", zif_ibase_rollback_ret, arginfo_fbird_rollback_ret, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_info", zif_ibase_blob_info, arginfo_fbird_blob_info, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_info", zif_ibase_blob_info, arginfo_fbird_blob_info, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_create", zif_ibase_blob_create, arginfo_fbird_blob_create, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_create", zif_ibase_blob_create, arginfo_fbird_blob_create, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_add", zif_ibase_blob_add, arginfo_fbird_blob_add, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_add", zif_ibase_blob_add, arginfo_fbird_blob_add, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_cancel", zif_ibase_blob_cancel, arginfo_fbird_blob_cancel, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_cancel", zif_ibase_blob_cancel, arginfo_fbird_blob_cancel, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_close", zif_ibase_blob_close, arginfo_fbird_blob_close, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_close", zif_ibase_blob_close, arginfo_fbird_blob_close, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_open", zif_ibase_blob_open, arginfo_fbird_blob_open, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_open", zif_ibase_blob_open, arginfo_fbird_blob_open, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_get", zif_ibase_blob_get, arginfo_fbird_blob_get, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_get", zif_ibase_blob_get, arginfo_fbird_blob_get, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_echo", zif_ibase_blob_echo, arginfo_fbird_blob_echo, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_echo", zif_ibase_blob_echo, arginfo_fbird_blob_echo, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_blob_import", zif_ibase_blob_import, arginfo_fbird_blob_import, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_blob_import", zif_ibase_blob_import, arginfo_fbird_blob_import, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_errmsg", zif_ibase_errmsg, arginfo_fbird_errmsg, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_errmsg", zif_ibase_errmsg, arginfo_fbird_errmsg, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_errcode", zif_ibase_errcode, arginfo_fbird_errcode, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_errcode", zif_ibase_errcode, arginfo_fbird_errcode, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_add_user", zif_ibase_add_user, arginfo_fbird_add_user, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_add_user", zif_ibase_add_user, arginfo_fbird_add_user, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_modify_user", zif_ibase_modify_user, arginfo_fbird_modify_user, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_modify_user", zif_ibase_modify_user, arginfo_fbird_modify_user, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_delete_user", zif_ibase_delete_user, arginfo_fbird_delete_user, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_delete_user", zif_ibase_delete_user, arginfo_fbird_delete_user, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_service_attach", zif_ibase_service_attach, arginfo_fbird_service_attach, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_service_attach", zif_ibase_service_attach, arginfo_fbird_service_attach, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_service_detach", zif_ibase_service_detach, arginfo_fbird_service_detach, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_service_detach", zif_ibase_service_detach, arginfo_fbird_service_detach, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_backup", zif_ibase_backup, arginfo_fbird_backup, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_backup", zif_ibase_backup, arginfo_fbird_backup, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_restore", zif_ibase_restore, arginfo_fbird_restore, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_restore", zif_ibase_restore, arginfo_fbird_restore, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_maintain_db", zif_ibase_maintain_db, arginfo_fbird_maintain_db, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_maintain_db", zif_ibase_maintain_db, arginfo_fbird_maintain_db, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_db_info", zif_ibase_db_info, arginfo_fbird_db_info, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_db_info", zif_ibase_db_info, arginfo_fbird_db_info, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_server_info", zif_ibase_server_info, arginfo_fbird_server_info, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_server_info", zif_ibase_server_info, arginfo_fbird_server_info, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_wait_event", zif_ibase_wait_event, arginfo_fbird_wait_event, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_wait_event", zif_ibase_wait_event, arginfo_fbird_wait_event, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_set_event_handler", zif_ibase_set_event_handler, arginfo_fbird_set_event_handler, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_set_event_handler", zif_ibase_set_event_handler, arginfo_fbird_set_event_handler, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_free_event_handler", zif_ibase_free_event_handler, arginfo_fbird_free_event_handler, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_free_event_handler", zif_ibase_free_event_handler, arginfo_fbird_free_event_handler, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_get_client_version", zif_ibase_get_client_version, arginfo_fbird_get_client_version, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_get_client_version", zif_ibase_get_client_version, arginfo_fbird_get_client_version, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_get_client_major_version", zif_ibase_get_client_major_version, arginfo_fbird_get_client_major_version, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_get_client_major_version", zif_ibase_get_client_major_version, arginfo_fbird_get_client_major_version, 0)
#endif
#if (PHP_VERSION_ID >= 80400)
	ZEND_RAW_FENTRY("fbird_get_client_minor_version", zif_ibase_get_client_minor_version, arginfo_fbird_get_client_minor_version, 0, NULL, NULL)
#else
	ZEND_RAW_FENTRY("fbird_get_client_minor_version", zif_ibase_get_client_minor_version, arginfo_fbird_get_client_minor_version, 0)
#endif
	ZEND_FE_END
};
