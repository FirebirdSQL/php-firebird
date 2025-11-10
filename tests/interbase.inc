<?php

require_once('config.inc');
require_once('functions.inc');

/* we need just the generated name, not the file itself */
unlink($test_base = tempnam(sys_get_temp_dir(),"php_ibase_test"));
if(!empty($host))$test_base = "$host:$test_base";

function init_db()
{
	global $test_base, $user, $password;

	$sql = sprintf("CREATE SCHEMA '%s' USER '%s' PASSWORD '%s' DEFAULT CHARACTER SET %s",
		$test_base, $user, $password,
		($charset = ini_get('ibase.default_charset')) ? $charset : 'NONE'
	);

	$test_db = ibase_query(IBASE_CREATE, $sql);

	$tr = ibase_trans($test_db);
	ibase_query($tr,"create table test1 (i integer, c varchar(100))");
	ibase_commit_ret($tr);
	ibase_query($tr,"insert into test1(i, c) values(1, 'test table not created with isql')");
	ibase_commit($tr);
	ibase_close($test_db);
}

function cleanup_db()
{
	global $test_base;

	if($r = ibase_connect($test_base)){
		ibase_drop_db($r);
	}
}

function get_fb_version(): float
{
	global $host, $user, $password, $fb_version, $fb_version_str;

	if(isset($fb_version)) return $fb_version;

	if($se = ibase_service_attach($host ? $host : "localhost", $user, $password)) {
		$fb_version_str = ibase_server_info($se, IBASE_SVC_SERVER_VERSION);
		if($fb_version_str === false)die("skip cannot ibase_server_info()");
		$server_version  = explode(" ", $fb_version_str);
		$vers = (float)$server_version[count($server_version) - 1];
		return $fb_version = $vers;
	} else {
		die("skip: cannot connect");
	}
}

register_shutdown_function('cleanup_db');
init_db();
