--TEST--
Binding a date string that PHP parses outside years 1..9999 fails with Firebird's range error instead of storing garbage
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ini_set('ibase.dateformat', 'Y-m-d');
    ini_set('ibase.timestampformat', 'Y-m-d H:i:s');

    // PHP parses year 0 and rolls overflowing days into years 0 and 10000, but Firebird's encoded values are not range checked,
    // so the string must go to Firebird as text
    $p = ibase_prepare("SELECT CAST(CAST(? AS DATE) AS VARCHAR(40)), CAST(CAST(? AS TIMESTAMP) AS VARCHAR(40)) FROM RDB\$DATABASE");
    foreach ([['0000-01-01', '2025-01-01 12:00:00'], ['2025-01-01', '0000-01-01 12:00:00'],
        ['9999-12-32', '2025-01-01 12:00:00'], ['0001-01-00', '2025-01-01 12:00:00']] as [$d, $ts]) {
        $r = ibase_execute($p, $d, $ts);
        var_dump($r === false ? false : ibase_fetch_row($r));
    }
})();

?>
--EXPECTF--
%AWarning: ibase_%s(): Dynamic SQL Error SQL error code = -303 value exceeds the range for valid dates %Ain %s on line %d
bool(false)
%AWarning: ibase_%s(): Dynamic SQL Error SQL error code = -303 value exceeds the range for valid timestamps %Ain %s on line %d
bool(false)
%AWarning: ibase_%s(): Dynamic SQL Error SQL error code = -303 value exceeds the range for valid dates %Ain %s on line %d
bool(false)
%AWarning: ibase_%s(): Dynamic SQL Error SQL error code = -303 value exceeds the range for valid dates %Ain %s on line %d
bool(false)
