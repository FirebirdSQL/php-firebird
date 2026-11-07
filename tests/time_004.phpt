--TEST--
IBASE_UNIXTIME: ignore IBASE_UNIXTIME flag for TIME_TZ fields
--SKIPIF--
<?php
include("skipif.inc");
// See also: tests/time_002.phpt
skip_if_ext_lt(61);
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");
ibase_connect($test_base);
ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);
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
  string(20) "15:45:59 Europe/Riga"
  ["T2"]=>
  int(1762436759)
}