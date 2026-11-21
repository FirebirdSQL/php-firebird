--TEST--
Fetched DATE, TIME and TIMESTAMP are read in PHP's default zone, with their own timestamp and offset
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    date_default_timezone_set('Europe/Riga');

    ibase_query("CREATE TABLE TTEST (N INTEGER, D DATE, T TIME, TS TIMESTAMP)");
    ibase_commit();

    ibase_query("INSERT INTO TTEST VALUES (1, DATE '2025-01-15', TIME '12:00:00', TIMESTAMP '2025-01-15 12:00:00')");
    ibase_query("INSERT INTO TTEST VALUES (2, DATE '2025-07-15', TIME '23:59:59.1234', TIMESTAMP '2025-07-15 12:00:00')");
    ibase_query("INSERT INTO TTEST VALUES (3, DATE '1900-03-01', TIME '00:00:00', TIMESTAMP '2500-12-31 23:59:59.9999')");
    ibase_commit();

    $f = 'Y-m-d H:i:s.u T P U';

    // TIME has no date, so it gets today's, the same as new DateTime('12:00')
    $same_as_php = function (string $got, string $time) use ($f) {
        $want = (new DateTime($time))->format($f);
        return $got === $want ? "TIME $time: same as new DateTime()" : "TIME $time: got $got, want $want";
    };
    $times = [1 => '12:00:00', 2 => '23:59:59.1234', 3 => '00:00:00'];

    echo "-- IBASE_PHPDATE\n";
    $q = ibase_query("SELECT N, D, T, TS FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_PHPDATE)) {
        foreach ([$row[1], $row[3]] as $v) {
            // the first format() must agree with one after anything that recomputes
            $first = $v->format($f);
            $v->getTimestamp();
            echo $first, $first === $v->format($f) ? '' : ' STALE', "\n";
        }
        echo $same_as_php($row[2]->format($f), $times[$row[0]]), "\n";
    }

    echo "-- string\n";
    ini_set('ibase.dateformat', $f);
    ini_set('ibase.timeformat', $f);
    ini_set('ibase.timestampformat', $f);
    $q = ibase_query("SELECT N, D, T, TS FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q)) {
        echo $row[1], "\n", $row[3], "\n", $same_as_php($row[2], $times[$row[0]]), "\n";
    }

    echo "-- default zone changed at runtime\n";
    date_default_timezone_set('UTC');
    $q = ibase_query("SELECT TS FROM TTEST ORDER BY N");
    while ($row = ibase_fetch_row($q, IBASE_PHPDATE)) {
        echo $row[0]->format($f), "\n";
    }
})();

?>
--EXPECT--
-- IBASE_PHPDATE
2025-01-15 00:00:00.000000 EET +02:00 1736892000
2025-01-15 12:00:00.000000 EET +02:00 1736935200
TIME 12:00:00: same as new DateTime()
2025-07-15 00:00:00.000000 EEST +03:00 1752526800
2025-07-15 12:00:00.000000 EEST +03:00 1752570000
TIME 23:59:59.1234: same as new DateTime()
1900-03-01 00:00:00.000000 RMT +01:36 -2203896994
2500-12-31 23:59:59.999900 EET +02:00 16756754399
TIME 00:00:00: same as new DateTime()
-- string
2025-01-15 00:00:00.000000 EET +02:00 1736892000
2025-01-15 12:00:00.000000 EET +02:00 1736935200
TIME 12:00:00: same as new DateTime()
2025-07-15 00:00:00.000000 EEST +03:00 1752526800
2025-07-15 12:00:00.000000 EEST +03:00 1752570000
TIME 23:59:59.1234: same as new DateTime()
1900-03-01 00:00:00.000000 RMT +01:36 -2203896994
2500-12-31 23:59:59.999900 EET +02:00 16756754399
TIME 00:00:00: same as new DateTime()
-- default zone changed at runtime
2025-01-15 12:00:00.000000 UTC +00:00 1736942400
2025-07-15 12:00:00.000000 UTC +00:00 1752580800
2500-12-31 23:59:59.999900 UTC +00:00 16756761599
