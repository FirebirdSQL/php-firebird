<?php

/**
 * @generate-function-entries
 * @generate-legacy-arginfo 70000
 */

/*
 * Notes on typing
 *
 * - Resources (links, transactions, queries, results, blobs, events, service
 *   handles) cannot be typed in PHP, so those parameters are left untyped and
 *   functions that return a resource have no native return type.
 *
 * - Many functions take an optional link/transaction as their FIRST argument
 *   and work out what each position means from the number of arguments passed.
 *   Those positions are left untyped.
 *
 * - "= UNKNOWN" is used only on those count-dependent positions: what the
 *   argument means depends on how many were passed, so no single default
 *   value fits. Elsewhere an optional link/name defaults to null, and passing
 *   null is the same as leaving it out (ZPP "r!" / "s!").
 */

/** @return resource|false */
function ibase_connect(string $database = "", string $username = "", string $password = "", string $charset = "", int $buffers = 0, int $dialect = 0, string $role = "", int $sync = 0) { die; }

/** @return resource|false */
function ibase_pconnect(string $database = "", string $username = "", string $password = "", string $charset = "", int $buffers = 0, int $dialect = 0, string $role = "", int $sync = 0) { die; }

/** @param resource $link_identifier */
function ibase_close($link_identifier = null): bool { die; }

/** @param resource $link_identifier */
function ibase_drop_db($link_identifier = null): bool { die; }

/**
 * ibase_query([resource $link_identifier, [resource $trans_identifier,]] string $query, mixed ...$bind_arg)
 * ibase_query(IBASE_CREATE, string $query)
 *
 * @param resource|string|int $link_identifier
 * @param resource|string|mixed $query
 * @return resource|int|bool
 */
function ibase_query($link_identifier, $query = UNKNOWN, mixed ...$bind_arg) { die; }

/** @param resource $result */
function ibase_fetch_row($result, int $fetch_flags = 0): array|false { die; }

/** @param resource $result */
function ibase_fetch_assoc($result, int $fetch_flags = 0): array|false { die; }

/** @param resource $result */
function ibase_fetch_object($result, int $fetch_flags = 0): stdClass|false { die; }

/** @param resource $result */
function ibase_free_result($result): bool { die; }

/** @param resource $result */
function ibase_name_result($result, string $name): bool { die; }

/**
 * ibase_prepare(string $query)
 * ibase_prepare(resource $link_identifier, string $query)
 * ibase_prepare(resource $link_identifier, resource $trans_identifier, string $query)
 *
 * @param resource|string $link_identifier
 * @param resource|string $query
 * @param string $trans_query
 * @return resource|false
 */
function ibase_prepare($link_identifier, $query = UNKNOWN, $trans_query = UNKNOWN) { die; }

/**
 * @param resource $query
 * @return resource|int|bool
 */
function ibase_execute($query, mixed ...$bind_arg) { die; }

/** @param resource $query */
function ibase_free_query($query): bool { die; }

/** @param resource $link_identifier */
function ibase_gen_id(string $generator, int $increment = 1, $link_identifier = null): int|string|false { die; }

/** @param resource $query_result */
function ibase_num_fields($query_result): int { die; }

/** @param resource $query */
function ibase_num_params($query): int { die; }

/**
 * Rows changed by the last statement in the transaction.
 * 0 for ... RETURNING on Firebird 5 (except INSERT ... VALUES); use ibase_query_affected_rows().
 *
 * @param resource $link_identifier connection or transaction
 */
function ibase_affected_rows($link_identifier = null): int|false { die; }

/**
 * Rows changed by the last execution of the query. Each call costs one round trip to the server.
 *
 * @param resource $query
 */
function ibase_query_affected_rows($query): int|false { die; }

/** @param resource $query_result */
function ibase_field_info($query_result, int $field_number): array|false { die; }

/** @param resource $query */
function ibase_param_info($query, int $field_number): array|false { die; }

/**
 * ibase_trans([int $trans_args, [resource $link_identifier, ...]], ...)
 *
 * @return resource|false
 */
function ibase_trans(mixed ...$trans_args) { die; }

/** @param resource $link_identifier */
function ibase_commit($link_identifier = null): bool { die; }

/** @param resource $link_identifier */
function ibase_rollback($link_identifier = null): bool { die; }

/** @param resource $link_identifier */
function ibase_commit_ret($link_identifier = null): bool { die; }

/** @param resource $link_identifier */
function ibase_rollback_ret($link_identifier = null): bool { die; }

/**
 * ibase_blob_info([resource $link_identifier,] string $blob_id)
 *
 * @param resource|string $link_identifier
 * @param string $blob_id
 */
function ibase_blob_info($link_identifier, $blob_id = UNKNOWN): array|false { die; }

/**
 * @param resource $link_identifier
 * @return resource|false
 */
function ibase_blob_create($link_identifier = null) { die; }

/**
 * @param resource $blob_handle
 * @param string $data
 */
function ibase_blob_add($blob_handle, $data): ?bool { die; }

/** @param resource $blob_handle */
function ibase_blob_cancel($blob_handle): bool { die; }

/** @param resource $blob_handle */
function ibase_blob_close($blob_handle): string|bool { die; }

/**
 * ibase_blob_open([resource $link_identifier,] string $blob_id)
 *
 * @param resource|string $link_identifier
 * @param string $blob_id
 * @return resource|false
 */
function ibase_blob_open($link_identifier, $blob_id = UNKNOWN) { die; }

/** @param resource $blob_handle */
function ibase_blob_get($blob_handle, int $len): string|false { die; }

/**
 * ibase_blob_echo([resource $link_identifier,] string $blob_id)
 *
 * @param resource|string $link_identifier
 * @param string $blob_id
 */
function ibase_blob_echo($link_identifier, $blob_id = UNKNOWN): bool { die; }

/**
 * ibase_blob_import([resource $link_identifier,] resource $file)
 *
 * @param resource $link_identifier
 * @param resource $file
 */
function ibase_blob_import($link_identifier, $file = UNKNOWN): string|false { die; }

function ibase_errmsg(): string|false { die; }

function ibase_errcode(): int|false { die; }

/** @param resource $service_handle */
function ibase_add_user($service_handle, string $user_name, string $password, ?string $first_name = null, ?string $middle_name = null, ?string $last_name = null): bool { die; }

/** @param resource $service_handle */
function ibase_modify_user($service_handle, string $user_name, string $password, ?string $first_name = null, ?string $middle_name = null, ?string $last_name = null): bool { die; }

/** @param resource $service_handle */
function ibase_delete_user($service_handle, string $user_name): bool { die; }

/** @return resource|false */
function ibase_service_attach(?string $host = null, ?string $dba_username = null, ?string $dba_password = null) { die; }

/** @param resource $service_handle */
function ibase_service_detach($service_handle): bool { die; }

/** @param resource $service_handle */
function ibase_backup($service_handle, string $source_db, string $dest_file, int $options = 0, bool $verbose = false): string|bool { die; }

/** @param resource $service_handle */
function ibase_restore($service_handle, string $source_file, string $dest_db, int $options = 0, bool $verbose = false): string|bool { die; }

/** @param resource $service_handle */
function ibase_maintain_db($service_handle, string $db, int $action, int $argument = 0): bool { die; }

/** @param resource $service_handle */
function ibase_db_info($service_handle, string $db, int $action, int $argument = 0): string|bool { die; }

/** @param resource $service_handle */
function ibase_server_info($service_handle, int $action): string|array|bool { die; }

/**
 * ibase_wait_event([resource $link_identifier,] string $event, string ...$event)
 *
 * @param resource|string $link_identifier
 */
function ibase_wait_event($link_identifier, mixed ...$event): string|false { die; }

/**
 * ibase_set_event_handler([resource $link_identifier,] callable $handler, string $event, string ...$event)
 *
 * @param resource|callable $link_identifier
 * @param callable|string $handler
 * @return resource|false
 */
function ibase_set_event_handler($link_identifier, $handler, mixed ...$event) { die; }

/** @param resource $event */
function ibase_free_event_handler($event): bool { die; }

function ibase_get_client_version(): int { die; }

function ibase_get_client_major_version(): int { die; }

function ibase_get_client_minor_version(): int { die; }

/*
 * Firebird aliases. As Firebird and InterBase are developed independently,
 * Firebird users should use these so future InterBase-specific changes do not
 * affect their code.
 */

/**
 * @return resource|false
 * @alias ibase_connect
 */
function fbird_connect(string $database = "", string $username = "", string $password = "", string $charset = "", int $buffers = 0, int $dialect = 0, string $role = "", int $sync = 0) { die; }

/**
 * @return resource|false
 * @alias ibase_pconnect
 */
function fbird_pconnect(string $database = "", string $username = "", string $password = "", string $charset = "", int $buffers = 0, int $dialect = 0, string $role = "", int $sync = 0) { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_close
 */
function fbird_close($link_identifier = null): bool { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_drop_db
 */
function fbird_drop_db($link_identifier = null): bool { die; }

/**
 * @param resource|string|int $link_identifier
 * @param resource|string|mixed $query
 * @return resource|int|bool
 * @alias ibase_query
 */
function fbird_query($link_identifier, $query = UNKNOWN, mixed ...$bind_arg) { die; }

/**
 * @param resource $result
 * @alias ibase_fetch_row
 */
function fbird_fetch_row($result, int $fetch_flags = 0): array|false { die; }

/**
 * @param resource $result
 * @alias ibase_fetch_assoc
 */
function fbird_fetch_assoc($result, int $fetch_flags = 0): array|false { die; }

/**
 * @param resource $result
 * @alias ibase_fetch_object
 */
function fbird_fetch_object($result, int $fetch_flags = 0): stdClass|false { die; }

/**
 * @param resource $result
 * @alias ibase_free_result
 */
function fbird_free_result($result): bool { die; }

/**
 * @param resource $result
 * @alias ibase_name_result
 */
function fbird_name_result($result, string $name): bool { die; }

/**
 * @param resource|string $link_identifier
 * @param resource|string $query
 * @param string $trans_query
 * @return resource|false
 * @alias ibase_prepare
 */
function fbird_prepare($link_identifier, $query = UNKNOWN, $trans_query = UNKNOWN) { die; }

/**
 * @param resource $query
 * @return resource|int|bool
 * @alias ibase_execute
 */
function fbird_execute($query, mixed ...$bind_arg) { die; }

/**
 * @param resource $query
 * @alias ibase_free_query
 */
function fbird_free_query($query): bool { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_gen_id
 */
function fbird_gen_id(string $generator, int $increment = 1, $link_identifier = null): int|string|false { die; }

/**
 * @param resource $query_result
 * @alias ibase_num_fields
 */
function fbird_num_fields($query_result): int { die; }

/**
 * @param resource $query
 * @alias ibase_num_params
 */
function fbird_num_params($query): int { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_affected_rows
 */
function fbird_affected_rows($link_identifier = null): int|false { die; }

/**
 * @param resource $query
 * @alias ibase_query_affected_rows
 */
function fbird_query_affected_rows($query): int|false { die; }

/**
 * @param resource $query_result
 * @alias ibase_field_info
 */
function fbird_field_info($query_result, int $field_number): array|false { die; }

/**
 * @param resource $query
 * @alias ibase_param_info
 */
function fbird_param_info($query, int $field_number): array|false { die; }

/**
 * @return resource|false
 * @alias ibase_trans
 */
function fbird_trans(mixed ...$trans_args) { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_commit
 */
function fbird_commit($link_identifier = null): bool { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_rollback
 */
function fbird_rollback($link_identifier = null): bool { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_commit_ret
 */
function fbird_commit_ret($link_identifier = null): bool { die; }

/**
 * @param resource $link_identifier
 * @alias ibase_rollback_ret
 */
function fbird_rollback_ret($link_identifier = null): bool { die; }

/**
 * @param resource|string $link_identifier
 * @param string $blob_id
 * @alias ibase_blob_info
 */
function fbird_blob_info($link_identifier, $blob_id = UNKNOWN): array|false { die; }

/**
 * @param resource $link_identifier
 * @return resource|false
 * @alias ibase_blob_create
 */
function fbird_blob_create($link_identifier = null) { die; }

/**
 * @param resource $blob_handle
 * @param string $data
 * @alias ibase_blob_add
 */
function fbird_blob_add($blob_handle, $data): ?bool { die; }

/**
 * @param resource $blob_handle
 * @alias ibase_blob_cancel
 */
function fbird_blob_cancel($blob_handle): bool { die; }

/**
 * @param resource $blob_handle
 * @alias ibase_blob_close
 */
function fbird_blob_close($blob_handle): string|bool { die; }

/**
 * @param resource|string $link_identifier
 * @param string $blob_id
 * @return resource|false
 * @alias ibase_blob_open
 */
function fbird_blob_open($link_identifier, $blob_id = UNKNOWN) { die; }

/**
 * @param resource $blob_handle
 * @alias ibase_blob_get
 */
function fbird_blob_get($blob_handle, int $len): string|false { die; }

/**
 * @param resource|string $link_identifier
 * @param string $blob_id
 * @alias ibase_blob_echo
 */
function fbird_blob_echo($link_identifier, $blob_id = UNKNOWN): bool { die; }

/**
 * @param resource $link_identifier
 * @param resource $file
 * @alias ibase_blob_import
 */
function fbird_blob_import($link_identifier, $file = UNKNOWN): string|false { die; }

/** @alias ibase_errmsg */
function fbird_errmsg(): string|false { die; }

/** @alias ibase_errcode */
function fbird_errcode(): int|false { die; }

/**
 * @param resource $service_handle
 * @alias ibase_add_user
 */
function fbird_add_user($service_handle, string $user_name, string $password, ?string $first_name = null, ?string $middle_name = null, ?string $last_name = null): bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_modify_user
 */
function fbird_modify_user($service_handle, string $user_name, string $password, ?string $first_name = null, ?string $middle_name = null, ?string $last_name = null): bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_delete_user
 */
function fbird_delete_user($service_handle, string $user_name): bool { die; }

/**
 * @return resource|false
 * @alias ibase_service_attach
 */
function fbird_service_attach(?string $host = null, ?string $dba_username = null, ?string $dba_password = null) { die; }

/**
 * @param resource $service_handle
 * @alias ibase_service_detach
 */
function fbird_service_detach($service_handle): bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_backup
 */
function fbird_backup($service_handle, string $source_db, string $dest_file, int $options = 0, bool $verbose = false): string|bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_restore
 */
function fbird_restore($service_handle, string $source_file, string $dest_db, int $options = 0, bool $verbose = false): string|bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_maintain_db
 */
function fbird_maintain_db($service_handle, string $db, int $action, int $argument = 0): bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_db_info
 */
function fbird_db_info($service_handle, string $db, int $action, int $argument = 0): string|bool { die; }

/**
 * @param resource $service_handle
 * @alias ibase_server_info
 */
function fbird_server_info($service_handle, int $action): string|array|bool { die; }

/**
 * @param resource|string $link_identifier
 * @alias ibase_wait_event
 */
function fbird_wait_event($link_identifier, mixed ...$event): string|false { die; }

/**
 * @param resource|callable $link_identifier
 * @param callable|string $handler
 * @return resource|false
 * @alias ibase_set_event_handler
 */
function fbird_set_event_handler($link_identifier, $handler, mixed ...$event) { die; }

/**
 * @param resource $event
 * @alias ibase_free_event_handler
 */
function fbird_free_event_handler($event): bool { die; }

/** @alias ibase_get_client_version */
function fbird_get_client_version(): int { die; }

/** @alias ibase_get_client_major_version */
function fbird_get_client_major_version(): int { die; }

/** @alias ibase_get_client_minor_version */
function fbird_get_client_minor_version(): int { die; }
