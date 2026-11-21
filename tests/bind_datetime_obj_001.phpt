--TEST--
Binding DateTime and DateTimeImmutable objects to DATE, TIME and TIMESTAMP
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    date_default_timezone_set('Asia/Tokyo');

    // Firebird's own text of what it received
    $p = ibase_prepare("SELECT CAST(CAST(? AS DATE) AS VARCHAR(40)),
        CAST(CAST(? AS TIME) AS VARCHAR(40)),
        CAST(CAST(? AS TIMESTAMP) AS VARCHAR(40)) FROM RDB\$DATABASE");

    // The object's own wall clock is bound, whatever its zone and the default zone
    foreach ([
        new DateTime('2025-01-15 12:34:56.123456', new DateTimeZone('Europe/Riga')),
        new DateTimeImmutable('2025-07-15 23:59:59', new DateTimeZone('-05:30')),
        new DateTime('0001-01-01 00:00:00'),
        new DateTime('9999-12-31 23:59:59.9999'),
    ] as $d) {
        echo implode(" | ", ibase_fetch_row(ibase_execute($p, $d, $d, $d))), "\n";
    }

    // Not initialized
    $u = new class extends DateTime { function __construct() {} };
    var_dump(ibase_execute($p, $u, $u, $u));
    echo ibase_errmsg(), "\n";

    // Out of Firebird's year range, which it does not check in encoded values
    $y = (new DateTime('9999-12-31'))->modify('+1 day');
    var_dump(ibase_execute($p, $y, '12:00', '2025-01-01'));
    echo ibase_errmsg(), "\n";
    $z = new DateTimeImmutable('0000-12-31');
    var_dump(ibase_execute($p, '2025-01-01', '12:00', $z));
    echo ibase_errmsg(), "\n";

    // TIME ignores the date part, so the year does not matter there
    echo implode(" | ", ibase_fetch_row(ibase_execute($p, '2025-01-01', $y, '2025-01-01'))), "\n";
})();

?>
--EXPECTF--
2025-01-15 | 12:34:56.1234 | 2025-01-15 12:34:56.1234
2025-07-15 | 23:59:59.0000 | 2025-07-15 23:59:59.0000
0001-01-01 | 00:00:00.0000 | 0001-01-01 00:00:00.0000
9999-12-31 | 23:59:59.9999 | 9999-12-31 23:59:59.9999

Warning: ibase_execute(): Parameter 1: DateTime object is not initialized in %s on line %d
bool(false)
Parameter 1: DateTime object is not initialized

Warning: ibase_execute(): Parameter 1: year 10000 is out of range 1..9999 in %s on line %d
bool(false)
Parameter 1: year 10000 is out of range 1..9999

Warning: ibase_execute(): Parameter 3: year 0 is out of range 1..9999 in %s on line %d
bool(false)
Parameter 3: year 0 is out of range 1..9999
2025-01-01 | 00:00:00.0000 | 2025-01-01 00:00:00.0000
