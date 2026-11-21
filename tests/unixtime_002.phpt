--TEST--
IBASE_UNIXTIME gives the same moment as IBASE_PHPDATE: default zone for zone-less types, the stored moment for TIMESTAMP WITH TIME ZONE, also in legacy mode
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    date_default_timezone_set('Europe/Riga');

    ibase_query("CREATE TABLE TTEST (N INTEGER, D DATE, TS TIMESTAMP, TZ TIMESTAMP WITH TIME ZONE)");
    ibase_commit();

    ibase_query("INSERT INTO TTEST VALUES (1, DATE '2025-01-15', TIMESTAMP '2025-01-15 12:00:00', TIMESTAMP '2025-07-01 12:00:00 America/New_York')");
    ibase_query("INSERT INTO TTEST VALUES (2, DATE '2025-07-15', TIMESTAMP '2025-07-15 12:00:00', TIMESTAMP '2025-10-26 00:30:00 UTC' AT TIME ZONE 'Europe/Riga')");
    ibase_query("INSERT INTO TTEST VALUES (3, NULL, NULL, TIMESTAMP '2025-10-26 01:30:00 UTC' AT TIME ZONE 'Europe/Riga')");
    ibase_commit();

    echo "-- non-legacy, Europe/Riga\n";
    ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);
    $q = ibase_query("SELECT D, TS, TZ FROM TTEST ORDER BY N");
    $o = ibase_query("SELECT D, TS, TZ FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_UNIXTIME)) {
        $obj = ibase_fetch_row($o, IBASE_PHPDATE);
        foreach ($row as $i => $v) {
            if ($v === null) continue;
            // format() rather than getTimestamp(): PHP 7.4 moves a time in the repeated hour on recompute
            echo $v, (string) $v === $obj[$i]->format('U') ? '' : ' MISMATCH ' . $obj[$i]->format('U'), "\n";
        }
    }

    echo "-- non-legacy, UTC\n";
    date_default_timezone_set('UTC');
    $q = ibase_query("SELECT D, TS FROM TTEST WHERE N < 3 ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_UNIXTIME)) {
        echo implode("\n", $row), "\n";
    }

    // Legacy reads zone-less values in the OS zone, which the test cannot rely on.
    // TIMESTAMP WITH TIME ZONE carries its moment and must not depend on it.
    echo "-- legacy, TIMESTAMP WITH TIME ZONE\n";
    ini_set('ibase.legacy_flags', IBASE_LEGACY_UNIXTIME);
    $q = ibase_query("SELECT TZ FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_UNIXTIME)) {
        echo $row[0], "\n";
    }
})();

?>
--EXPECT--
-- non-legacy, Europe/Riga
1736892000
1736935200
1751385600
1752526800
1752570000
1761438600
1761442200
-- non-legacy, UTC
1736899200
1736942400
1752537600
1752580800
-- legacy, TIMESTAMP WITH TIME ZONE
1751385600
1761438600
1761442200
