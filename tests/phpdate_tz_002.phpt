--TEST--
Fetched TIMESTAMP WITH TIME ZONE in the DST fall-back hour keeps its own UTC instant
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
if (PHP_VERSION_ID < 80000) echo "xfail PHP 7 moves a time in the repeated hour on recompute";
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    date_default_timezone_set('Europe/Riga');

    ibase_query("CREATE TABLE TTEST (N INTEGER, TS TIMESTAMP WITH TIME ZONE)");
    ibase_commit();

    // both are 03:30 local, only the stored UTC instant tells them apart
    ibase_query("INSERT INTO TTEST VALUES (1, TIMESTAMP '2025-10-26 00:30:00 UTC' AT TIME ZONE 'Europe/Riga')");
    ibase_query("INSERT INTO TTEST VALUES (2, TIMESTAMP '2025-10-26 01:30:00 UTC' AT TIME ZONE 'Europe/Riga')");
    ibase_commit();

    $f = 'Y-m-d H:i:s.u T P U';

    echo "-- IBASE_PHPDATE\n";
    $q = ibase_query("SELECT TS FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_PHPDATE)) {
        // the first format() must agree with one after anything that recomputes
        $first = $row[0]->format($f);
        $row[0]->getTimestamp();
        echo $first, $first === $row[0]->format($f) ? '' : ' STALE', "\n";
    }

    echo "-- string\n";
    ini_set('ibase.timestampformat_tz', $f);
    $q = ibase_query("SELECT TS FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q)) {
        echo $row[0], "\n";
    }
})();

?>
--EXPECT--
-- IBASE_PHPDATE
2025-10-26 03:30:00.000000 EEST +03:00 1761438600
2025-10-26 03:30:00.000000 EET +02:00 1761442200
-- string
2025-10-26 03:30:00.000000 EEST +03:00 1761438600
2025-10-26 03:30:00.000000 EET +02:00 1761442200
