--TEST--
ibase_num_fields(): Make sure passing zero arguments to the function throws an error
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

var_dump(ibase_num_fields());

?>
--EXPECTF--
Fatal error: Uncaught ArgumentCountError: ibase_num_fields() expects exactly 1 argument, 0 given in %a
