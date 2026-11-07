--TEST--
Sub-second precision survives both binding and fetching
--SKIPIF--
<?php
include("skipif.inc");
// Sub-second precision needs IUtil::decodeTime(). Without it -- headers older than 3.0,
// or a runtime client with no OO API -- the isc_* fallback has no field for it.
skip_if_fbeffective_lt(30);
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE TFRAC (T TIME, TS TIMESTAMP)");
    ibase_commit();

    ini_set('ibase.timeformat', 'H:i:s.u');
    ini_set('ibase.timestampformat', 'Y-m-d H:i:s.u');

    ibase_query("INSERT INTO TFRAC (T, TS) VALUES (?, ?)",
        "13:45:59.123400", "2025-11-06 13:45:59.123400") or die("insert failed: " . ibase_errmsg());
    ibase_commit();

    // What Firebird itself stored, in its own 1/10000 s notation
    $row = ibase_fetch_row(ibase_query(
        "SELECT CAST(T AS VARCHAR(30)), CAST(TS AS VARCHAR(40)) FROM TFRAC"));
    var_dump($row[0], $row[1]);

    // ... and what the extension formats it back to, in microseconds
    $row = ibase_fetch_row(ibase_query("SELECT T, TS FROM TFRAC"));
    var_dump($row[0], $row[1]);

    $row = ibase_fetch_row(ibase_query("SELECT T, TS FROM TFRAC"), IBASE_PHPDATE);
    var_dump($row[0]->format('H:i:s.u'), $row[1]->format('Y-m-d H:i:s.u'));

    // Firebird cannot store a finer unit than 1/10000 s, the rest is truncated
    ibase_query("DELETE FROM TFRAC");
    ibase_query("INSERT INTO TFRAC (T, TS) VALUES (?, ?)",
        "13:45:59.123456", "2025-11-06 13:45:59.123456") or die("insert failed: " . ibase_errmsg());
    ibase_commit();
    $row = ibase_fetch_row(ibase_query("SELECT T, TS FROM TFRAC"));
    var_dump($row[0], $row[1]);
})();

?>
--EXPECT--
string(13) "13:45:59.1234"
string(24) "2025-11-06 13:45:59.1234"
string(15) "13:45:59.123400"
string(26) "2025-11-06 13:45:59.123400"
string(15) "13:45:59.123400"
string(26) "2025-11-06 13:45:59.123400"
string(15) "13:45:59.123400"
string(26) "2025-11-06 13:45:59.123400"
