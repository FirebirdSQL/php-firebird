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

$rs = ibase_prepare('SELECT * FROM test1 WHERE 1 = ? AND 2 = ? AND 3 = :x');
var_dump(ibase_num_params($rs));

?>
--EXPECTF--
int(2)

Warning: ibase_prepare(): Dynamic SQL Error SQL error code = -%d Column unknown X At line %d, column %d %s

Fatal error: Uncaught TypeError: ibase_num_params(): Argument #1 ($query) must be of type resource, %a