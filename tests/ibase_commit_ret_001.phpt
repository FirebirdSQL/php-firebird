--TEST--
ibase_commit_ret(): Make sure the method can be invoked with zero arguments
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

ibase_connect($test_base);

ibase_query('INSERT INTO test1 VALUES (100, 2)');

var_dump(ibase_commit_ret());

?>
--EXPECTF--
bool(true)
