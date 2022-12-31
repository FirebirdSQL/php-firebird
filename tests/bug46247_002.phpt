--TEST--
Bug #46247 (ibase_set_event_handler() is allowing to pass callback without event)
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

$db = ibase_connect($test_base);

ibase_set_event_handler('foo', 1);
ibase_set_event_handler($db, 'foo', 1);

?>
--EXPECTF--
Warning: ibase_set_event_handler(): Callback argument foo is not a callable function in %s on line %d

Warning: ibase_set_event_handler(): Callback argument foo is not a callable function in %s on line %d
