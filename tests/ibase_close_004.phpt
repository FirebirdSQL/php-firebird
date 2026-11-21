--TEST--
ibase_close(): Basic test
--SKIPIF--
<?php

include(__DIR__."/skipif.inc");

skip_if_php_lt(8);

?>
--FILE--
<?php

require(__DIR__."/interbase.inc");

set_exception_handler("php_ibase_exception_handler");

// As of 6.1.1-RC3 ibase_close() really closes the connection.

$x = ibase_connect($test_base);
var_dump(ibase_close($x));
var_dump(ibase_close($x));
var_dump(ibase_close());

?>
--EXPECT--
bool(true)
Fatal error: Uncaught TypeError: ibase_close(): supplied resource is not a valid Firebird/InterBase link resource
