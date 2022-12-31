--TEST--
ibase_num_fields(): Make sure passing an integer to the function throws an error.
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

var_dump(ibase_num_fields(1));

?>
--EXPECTF--
Fatal error: Uncaught TypeError: ibase_num_fields(): Argument #1 ($query_result) must be of type resource, int given in %a
