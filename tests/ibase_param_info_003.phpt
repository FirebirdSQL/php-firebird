--TEST--
ibase_param_info(): Error if called with a single argument
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

var_dump(ibase_param_info(100));

?>
--EXPECTF--
Fatal error: Uncaught ArgumentCountError: ibase_param_info() expects exactly 2 arguments, 1 given in %a
