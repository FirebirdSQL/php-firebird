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

unlink($file = tempnam('/tmp',"php_ibase_test"));

$db = ibase_query(IBASE_CREATE,
		sprintf("CREATE SCHEMA '%s' USER '%s' PASSWORD '%s' DEFAULT CHARACTER SET %s",$file,
		$user, $password, ($charset = ini_get('ibase.default_charset')) ? $charset : 'NONE'));

var_dump($db);
var_dump(ibase_drop_db($db));
var_dump(ibase_drop_db(NULL));

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase link)
bool(true)

Fatal error: Uncaught TypeError: ibase_drop_db(): Argument #1 ($link_identifier) must be of type resource, null given in %a
