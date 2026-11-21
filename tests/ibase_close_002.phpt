--TEST--
ibase_close(): Make sure passing a string to the function emits a warning
--SKIPIF--
<?php
include(__DIR__."/skipif.inc");
include(__DIR__."/skipif-php8-or-newer.inc");
?>
--FILE--
<?php

require(__DIR__."/interbase.inc");

// As of 6.1.1-RC3 ibase_close() really closes the connection.

$x = ibase_connect($test_base);
var_dump(ibase_close($x));
var_dump(ibase_close($x));
var_dump(ibase_close());
var_dump(ibase_close('foo'));

?>
--EXPECTF--
bool(true)

Warning: ibase_close(): supplied resource is not a valid Firebird/InterBase link resource %s
bool(false)

Warning: ibase_close(): supplied resource is not a valid Firebird/InterBase link resource %s
bool(false)

Warning: ibase_close() expects parameter 1 to be resource, string given %s
NULL
