--TEST--
InterBase: use after ibase_free_query()
--SKIPIF--
<?php
include("skipif.inc");
if(defined('IBASE_VER') && (IBASE_VER >= 61)) print "Skip IBASE_VER < 6.1";
?>
--FILE--
<?php

// Related to the "test execute procedure" part of tests/006.phpt. This
// ilustrates incorrect use of ibase_free_query(). If you follow the same logic
// as in 006.phpt, you would expect 5 batches with 2 rows printed with
// incremented I but it doesn't of course.

require("interbase.inc");
require("common.inc");
test_use_after_ibase_free_query();

?>
--EXPECT--
---- Batch 1 ----
array(2) {
  ["I"]=>
  int(5)
  ["C"]=>
  string(15) "ROW 1 (batch 5)"
}
array(2) {
  ["I"]=>
  int(5)
  ["C"]=>
  string(15) "ROW 2 (batch 5)"
}
---- Batch 2 ----
---- Batch 3 ----
---- Batch 4 ----
---- Batch 5 ----