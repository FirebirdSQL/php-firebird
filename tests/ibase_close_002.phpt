--TEST--
ibase_close(): Make sure passing a string to the function emits a warning
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php8-or-newer.inc");
?>
--FILE--
<?php

require("interbase.inc");

$x = ibase_connect($test_base);
var_dump(ibase_close($x));
var_dump(ibase_close($x));
var_dump(ibase_close());
var_dump(ibase_close('foo'));

?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

Warning: ibase_close() expects parameter 1 to be resource, string given in %s on line %d
NULL
