--TEST--
ibase_num_params(): Basic test
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

require("interbase.inc");

$x = ibase_connect($test_base);

$rs = ibase_prepare('SELECT * FROM test1 WHERE 1 = ? AND 2 = ?');
var_dump(ibase_num_params($rs));

$rs = ibase_prepare('SELECT * FROM test1 WHERE 1 = ? AND 2 = ?');
var_dump(ibase_num_params());

?>
--EXPECTF--
int(2)

Fatal error: Uncaught ArgumentCountError: ibase_num_params() expects exactly 1 argument, 0 given in %a