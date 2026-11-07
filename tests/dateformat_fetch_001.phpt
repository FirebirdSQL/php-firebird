--TEST--
ibase.dateformat / timeformat / timestampformat are used to format fetched values
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

    ibase_query("INSERT INTO TTEST (D, T, TS) VALUES
        (DATE '2025-11-06', TIME '13:45:59', TIMESTAMP '2025-11-06 13:45:59')");
    ibase_commit();

    foreach ([
        ['Y-m-d', 'H:i:s', 'Y-m-d H:i:s'],            // the shipped defaults
        // 6 November: d/m/Y and m/d/Y must not produce the same string
        ['d/m/Y', 'H.i.s', 'd/m/Y H.i.s'],
        ['m/d/Y', 'h:i:s A', 'm/d/Y h:i:s A'],
        ['j.n.Y', 'G:i', 'j.n.Y G:i'],                // no leading zeros
        ['Y.d.m', 's-i-H', 'Y.d.m s-i-H'],            // deliberately scrambled
        ['d-M-Y', 'H\hi\m', 'l jS F Y'],
        ['D, d M Y', 'g:ia', 'c'],
    ] as [$d, $t, $ts]) {
        ini_set('ibase.dateformat', $d);
        ini_set('ibase.timeformat', $t);
        ini_set('ibase.timestampformat', $ts);

        print_r(ibase_fetch_assoc(ibase_query("SELECT * FROM TTEST")));
    }
})();

?>
--EXPECT--
Array
(
    [D] => 2025-11-06
    [T] => 13:45:59
    [TS] => 2025-11-06 13:45:59
)
Array
(
    [D] => 06/11/2025
    [T] => 13.45.59
    [TS] => 06/11/2025 13.45.59
)
Array
(
    [D] => 11/06/2025
    [T] => 01:45:59 PM
    [TS] => 11/06/2025 01:45:59 PM
)
Array
(
    [D] => 6.11.2025
    [T] => 13:45
    [TS] => 6.11.2025 13:45
)
Array
(
    [D] => 2025.06.11
    [T] => 59-45-13
    [TS] => 2025.06.11 59-45-13
)
Array
(
    [D] => 06-Nov-2025
    [T] => 13h45m
    [TS] => Thursday 6th November 2025
)
Array
(
    [D] => Thu, 06 Nov 2025
    [T] => 1:45pm
    [TS] => 2025-11-06T13:45:59+00:00
)
