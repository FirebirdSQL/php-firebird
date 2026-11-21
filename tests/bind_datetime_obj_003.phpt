--TEST--
Binding DateTime and DateTimeImmutable objects in the EEST zone to TIME and TIMESTAMP WITH TIME ZONE keeps +03:00
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
if (PHP_VERSION_ID < 80000) die("skip PHP 7 reports EEST as +02:00");
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    // A default zone different from the zone bound below
    date_default_timezone_set('Asia/Tokyo');

    // Firebird's own text of what it received
    $p = ibase_prepare("SELECT CAST(CAST(? AS TIME WITH TIME ZONE) AS VARCHAR(40)),
        CAST(CAST(? AS TIMESTAMP WITH TIME ZONE) AS VARCHAR(60)) FROM RDB\$DATABASE");

    $tz = new DateTimeZone('EEST');
    $d = new DateTime('2025-01-15 12:00:00.5', $tz);
    $i = new DateTimeImmutable('2025-07-15 12:00:00', $tz);
    printf("%-11s %s | %s\n", 'EEST', ...ibase_fetch_row(ibase_execute($p, $d, $d)));
    printf("%-11s %s | %s\n", 'EEST', ...ibase_fetch_row(ibase_execute($p, $i, $i)));
})();

?>
--EXPECT--
EEST        12:00:00.5000 +03:00 | 2025-01-15 12:00:00.5000 +03:00
EEST        12:00:00.0000 +03:00 | 2025-07-15 12:00:00.0000 +03:00
