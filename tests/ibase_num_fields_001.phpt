--TEST--
ibase_num_fields(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

$x = ibase_connect($test_base);

var_dump(ibase_num_fields(ibase_query('SELECT * FROM test1')));

?>
--EXPECTF--
int(2)
