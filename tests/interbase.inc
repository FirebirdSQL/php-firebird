<?php

require_once(__DIR__.'/config.inc');
require_once(__DIR__.'/functions.inc');

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

/* Server version on the FB_API_VER scale, the same one FB_API_VER,
   ibase_get_client_version() and IBASE_EFFECTIVE_VER use: 2.5 is 25, 5.0 is 50. */
function get_fb_version(): int
{
	global $host, $user, $password, $fb_version, $fb_version_str;

	if(isset($fb_version)) return $fb_version;

	if($se = ibase_service_attach($host ? $host : "localhost", $user, $password)) {
		$fb_version_str = ibase_server_info($se, IBASE_SVC_SERVER_VERSION);
		if($fb_version_str === false)die("skip cannot ibase_server_info()");
		if(!preg_match('/(\d+)\.(\d+)/', $fb_version_str, $m))die("skip cannot read server version from '$fb_version_str'");
		return $fb_version = (int)$m[1] * 10 + (int)$m[2];
	} else {
		die("skip: cannot connect");
	}
}

register_shutdown_function('cleanup_db');
init_db();
