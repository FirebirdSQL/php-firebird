--TEST--
IBASE_UNIXTIME: ignore IBASE_UNIXTIME flag for TIME fields
--SKIPIF--
<?php
include("skipif.inc");

// See also: tests/time_001.phpt
skip_if_ext_lt(61);

?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");

/** @var string $test_base */
ibase_connect($test_base);
ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);
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
  string(8) "15:45:59"
  ["T2"]=>
  int(1762443959)
}