--TEST--
ibase_num_fields(): Make sure passing an integer or zero args to the function emits a warning
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php8-or-newer.inc");
?>
--FILE--
<?php

var_dump(ibase_num_fields(1));
var_dump(ibase_num_fields());

?>
--EXPECTF--
Warning: ibase_num_fields() expects parameter 1 to be resource, int given in %s on line %d
NULL

Warning: ibase_num_fields() expects exactly 1 parameter, 0 given in %s on line %d
NULL
