--TEST--
ibase_drop_db(): Make sure passing null to the function throws an error.
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

require("config.inc");

unlink($file = tempnam(sys_get_temp_dir(),"php_ibase_test"));
if(!empty($host))$file = "$host:$file";

$db = ibase_query(IBASE_CREATE,
		sprintf("CREATE SCHEMA '%s' DEFAULT CHARACTER SET %s",$file,
		($charset = ini_get('ibase.default_charset')) ? $charset : 'NONE'));

var_dump($db);
var_dump(ibase_drop_db($db));
var_dump(ibase_drop_db(NULL));

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase link)
bool(true)

Warning: ibase_drop_db(): invalid database handle (no active connection) %s
bool(false)
