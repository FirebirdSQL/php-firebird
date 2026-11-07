--TEST--
A string parameter that does not match the configured format is left for Firebird to parse
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE TTEST (D DATE, TS TIMESTAMP)");
    ibase_commit();

    ini_set('ibase.dateformat', 'Y-m-d');
    ini_set('ibase.timestampformat', 'Y-m-d H:i:s');

    // No time part, so it does not match ibase.timestampformat -- Firebird accepts it
    var_dump(ibase_query("INSERT INTO TTEST (TS) VALUES (?)", "2025-11-06") !== false);
    ibase_commit();
    var_dump(ibase_fetch_row(ibase_query("SELECT CAST(TS AS VARCHAR(40)) FROM TTEST"))[0]);

    // Matches neither the format nor anything Firebird understands
    var_dump(@ibase_query("INSERT INTO TTEST (D) VALUES (?)", "garbage"));
    var_dump(str_contains(ibase_errmsg(), "conversion error from string"));
})();

?>
--EXPECT--
bool(true)
string(24) "2025-11-06 00:00:00.0000"
bool(false)
bool(true)
