--TEST--
Bug #46247 (ibase_set_event_handler() is allowing to pass callback without event)
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

require("interbase.inc");

$db = ibase_connect($test_base);

function test() { }

ibase_set_event_handler(NULL, 'test', 1);

?>
--EXPECTF--
Fatal error: Uncaught TypeError: ibase_set_event_handler(): supplied argument is not a valid InterBase link resource in %a
