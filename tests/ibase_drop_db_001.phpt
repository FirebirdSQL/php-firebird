--TEST--
ibase_drop_db(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("config.inc");

unlink($file = tempnam(sys_get_temp_dir(),"php_ibase_test"));
if(!empty($host))$file = "$host:$file";

$db = ibase_query(IBASE_CREATE,
		sprintf("CREATE SCHEMA '%s' USER '%s' PASSWORD '%s' DEFAULT CHARACTER SET %s",$file,
		$user, $password, ($charset = ini_get('ibase.default_charset')) ? $charset : 'NONE'));

var_dump($db);
var_dump(ibase_drop_db($db));

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase link)
bool(true)
