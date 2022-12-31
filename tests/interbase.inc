<?php

require('config.inc');
require('functions.inc');

/* we need just the generated name, not the file itself */
unlink($test_base = tempnam(sys_get_temp_dir(),"php_ibase_test"));

function init_db()
{
	global $test_base, $user, $password;

	$test_db = ibase_query(IBASE_CREATE,
		sprintf("CREATE SCHEMA '%s' USER '%s' PASSWORD '%s' DEFAULT CHARACTER SET %s",$test_base,
		$user, $password, ($charset = ini_get('ibase.default_charset')) ? $charset : 'NONE'));
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

	$r = ibase_connect($test_base);
	ibase_drop_db($r);
}

register_shutdown_function('cleanup_db');
init_db();

?>
