--TEST--
Fetched TIME and TIMESTAMP WITH TIME ZONE carry their own timestamp and UTC offset
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

    ibase_query("CREATE TABLE TTEST (N INTEGER, TS TIMESTAMP WITH TIME ZONE, T TIME WITH TIME ZONE)");
    ibase_commit();

    // winter and summer mixed in one result
    ibase_query("INSERT INTO TTEST VALUES (1, TIMESTAMP '2025-01-15 12:00:00 Europe/Riga', TIME '12:00:00 UTC')");
    ibase_query("INSERT INTO TTEST VALUES (2, TIMESTAMP '2025-07-15 12:00:00 Europe/Riga', TIME '12:00:00 +05:00')");
    ibase_query("INSERT INTO TTEST VALUES (3, TIMESTAMP '2025-01-15 12:00:00.5 Europe/Riga', TIME '12:00:00.25 Europe/Riga')");
    ibase_query("INSERT INTO TTEST VALUES (4, TIMESTAMP '2025-07-15 12:00:00 +05:00', NULL)");
    ibase_commit();

    $f = 'Y-m-d H:i:s.u T P U';

    // TIME has no date, so it gets today's, the same as new DateTime('12:00') in its zone
    $same_as_php = function (string $got, string $time, string $zone) use ($f) {
        $want = (new DateTime($time, new DateTimeZone($zone)))->format($f);
        return $got === $want ? "TIME $time $zone: same as new DateTime()" : "TIME $time $zone: got $got, want $want";
    };
    $times = [1 => ['12:00:00', 'UTC'], 2 => ['12:00:00', '+05:00'], 3 => ['12:00:00.25', 'Europe/Riga']];

    echo "-- IBASE_PHPDATE\n";
    $q = ibase_query("SELECT N, TS, T FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_PHPDATE)) {
        // the first format() must agree with one after anything that recomputes
        $first = $row[1]->format($f);
        $row[1]->getTimestamp();
        echo $first, $first === $row[1]->format($f) ? '' : ' STALE', "\n";
        if ($row[2]) {
            echo $same_as_php($row[2]->format($f), ...$times[$row[0]]), "\n";
        }
    }

    echo "-- string\n";
    ini_set('ibase.timestampformat_tz', $f);
    ini_set('ibase.timeformat_tz', $f);
    $q = ibase_query("SELECT N, TS, T FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q)) {
        echo $row[1], "\n";
        if ($row[2]) {
            echo $same_as_php($row[2], ...$times[$row[0]]), "\n";
        }
    }
})();

?>
--EXPECT--
-- IBASE_PHPDATE
2025-01-15 12:00:00.000000 EET +02:00 1736935200
TIME 12:00:00 UTC: same as new DateTime()
2025-07-15 12:00:00.000000 EEST +03:00 1752570000
TIME 12:00:00 +05:00: same as new DateTime()
2025-01-15 12:00:00.500000 EET +02:00 1736935200
TIME 12:00:00.25 Europe/Riga: same as new DateTime()
2025-07-15 12:00:00.000000 GMT+0500 +05:00 1752562800
-- string
2025-01-15 12:00:00.000000 EET +02:00 1736935200
TIME 12:00:00 UTC: same as new DateTime()
2025-07-15 12:00:00.000000 EEST +03:00 1752570000
TIME 12:00:00 +05:00: same as new DateTime()
2025-01-15 12:00:00.500000 EET +02:00 1736935200
TIME 12:00:00.25 Europe/Riga: same as new DateTime()
2025-07-15 12:00:00.000000 GMT+0500 +05:00 1752562800
