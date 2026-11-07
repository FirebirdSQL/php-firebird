--TEST--
IBASE_UNIXTIME: TIMESTAMP column round-trip with unix timestamp (integer) input
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");

/** @var string $test_base */
ibase_connect($test_base);
ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);

(function() {
    $ts = 1762436759; // 2025-11-06 13:45:59 UTC

    ibase_query(
        "CREATE TABLE TTEST (
            TS TIMESTAMP
        )"
    );
    ibase_commit();

    ibase_query("INSERT INTO TTEST (TS) VALUES (?)", $ts) or die("ibase_query failed");

    dump_table_rows("TTEST");
    dump_table_rows("TTEST", null, IBASE_UNIXTIME);
})();

?>
--EXPECT--
array(1) {
  ["TS"]=>
  string(19) "2025-11-06 13:45:59"
}
array(1) {
  ["TS"]=>
  int(1762436759)
}
