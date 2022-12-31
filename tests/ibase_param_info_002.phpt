--TEST--
ibase_param_info(): Error if called with a single argument
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php8-or-newer.inc");
?>
--FILE--
<?php

var_dump(ibase_param_info(100));

?>
--EXPECTF--
Warning: ibase_param_info() expects exactly 2 parameters, 1 given in %s on line %d
NULL
