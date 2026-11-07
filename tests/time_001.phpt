--TEST--
IBASE_UNIXTIME: return negative unix timestamp (IBASE_LEGACY_UNIXTIMESTAMP) for TIME fields
--SKIPIF--
<?php
include("skipif.inc");

// See also: tests/time_003.phpt

?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");
ibase_connect($test_base);
ini_set('ibase.legacy_flags', IBASE_LEGACY_UNIXTIMESTAMP);
test_time_unixtime();

?>
--EXPECTF--
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  string(8) "15:45:59"
  ["T2"]=>
  string(19) "2025-11-06 15:45:59"
}
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  int(-%d)
  ["T2"]=>
  int(1762436759)
}