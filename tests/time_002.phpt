--TEST--
IBASE_UNIXTIME: return negative unix timestamp (IBASE_LEGACY_UNIXTIMESTAMP) for TIME_TZ fields
--SKIPIF--
<?php
include("skipif.inc");
// See also: tests/time_004.phpt
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");
ibase_connect($test_base);
ini_set('ibase.legacy_flags', IBASE_LEGACY_UNIXTIMESTAMP);
test_time_tz_unixtime();

?>
--EXPECTF--
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  string(20) "15:45:59 Europe/Riga"
  ["T2"]=>
  string(31) "2025-11-06 15:45:59 Europe/Riga"
}
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  int(-%d)
  ["T2"]=>
  int(1762436759)
}