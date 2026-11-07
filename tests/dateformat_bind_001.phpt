--TEST--
ibase.dateformat / timeformat / timestampformat are used to parse string parameters
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE TTEST (D DATE, T TIME, TS TIMESTAMP)");
    ibase_commit();

    // Formats Firebird would read differently on its own: it takes 06/11/2025 as
    // MM/DD/YYYY, and does not accept a dot-separated time at all.
    ini_set('ibase.dateformat', 'd/m/Y');
    ini_set('ibase.timeformat', 'H.i.s');
    ini_set('ibase.timestampformat', 'd/m/Y H.i.s');

    ibase_query("INSERT INTO TTEST (D, T, TS) VALUES (?, ?, ?)",
        "06/11/2025", "13.45.59", "06/11/2025 13.45.59") or die("insert failed: " . ibase_errmsg());
    ibase_commit();

    // Read back through Firebird so the extension's own output formatting is not involved
    $row = ibase_fetch_row(ibase_query(
        "SELECT CAST(D AS VARCHAR(30)), CAST(T AS VARCHAR(30)), CAST(TS AS VARCHAR(40)) FROM TTEST"));
    var_dump($row[0], $row[1], $row[2]);
})();

?>
--EXPECT--
string(10) "2025-11-06"
string(13) "13:45:59.0000"
string(24) "2025-11-06 13:45:59.0000"
