--TEST--
BIGINT / NUMERIC(>=10) direct integer binding round-trips values above INT32_MAX
--SKIPIF--
<?php
include("skipif.inc");
if (PHP_INT_SIZE < 8) print "skip: only for 64-bit PHP";
?>
--FILE--
<?php

// Regression test: lval in BIND_BUF was declared ISC_LONG (32-bit on some platforms).
// The old code avoided writing through it for SQL_INT64 by falling through to a
// convert_to_string() + SQL_TEXT fallback, which was correct but slow (PHP string
// allocation + Firebird-side string parse). Fixing lval to ISC_INT64 enables the
// direct integer path. This test verifies the new path produces identical results.
//
// NUMERIC/DECIMAL storage in Firebird:
//   precision <= 4  -> SQL_SHORT  (not affected)
//   precision <= 9  -> SQL_LONG   (not affected)
//   precision >= 10 -> SQL_INT64  (affected)

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query(
        "CREATE TABLE BIGINT_BIND_TEST (
            ID          INTEGER NOT NULL PRIMARY KEY,
            BIG         BIGINT,
            NUM_10_2    NUMERIC(10, 2),
            DEC_12_4    DECIMAL(12, 4)
        )");
    ibase_commit();

    $rows = [
        // [id, big, num_10_2, dec_12_4]
        // one above INT32_MAX
        [1,  2147483648,            21474836.48,   214748.3648],
        // well above INT32_MAX
        [2,  9876543210,            98765432.10,   987654.3210],
        // INT64_MAX
        [3,  PHP_INT_MAX,           null,          null],
        // negative: one below INT32_MIN
        [4, -2147483649,           -21474836.49,  -214748.3649],
        // INT64_MIN: -9223372036854775808 as a PHP literal becomes a float because
        // the positive literal 9223372036854775808 overflows PHP_INT_MAX first;
        // use the PHP_INT_MIN constant to keep it as an integer.
        [5,  PHP_INT_MIN,           null,          null],
        // round-trip zero to confirm baseline
        [6,  0,                     0.00,          0.0000],
    ];

    $ins = ibase_prepare(
        "INSERT INTO BIGINT_BIND_TEST (ID, BIG, NUM_10_2, DEC_12_4) VALUES (?, ?, ?, ?)"
    );
    foreach ($rows as [$id, $big, $num, $dec]) {
        if (false === ibase_execute($ins, $id, $big, $num, $dec)) {
            echo "INSERT $id failed: " . ibase_errmsg() . "\n";
        }
    }
    ibase_free_query($ins);
    ibase_commit();

    $sel = ibase_prepare(
        "SELECT ID, BIG, NUM_10_2, DEC_12_4 FROM BIGINT_BIND_TEST ORDER BY ID"
    );
    $q = ibase_execute($sel);
    dump_rows($q);
    ibase_free_result($q);
    ibase_free_query($sel);
})();
?>
--EXPECT--
array(4) {
  ["ID"]=>
  int(1)
  ["BIG"]=>
  int(2147483648)
  ["NUM_10_2"]=>
  string(11) "21474836.48"
  ["DEC_12_4"]=>
  string(11) "214748.3648"
}
array(4) {
  ["ID"]=>
  int(2)
  ["BIG"]=>
  int(9876543210)
  ["NUM_10_2"]=>
  string(11) "98765432.10"
  ["DEC_12_4"]=>
  string(11) "987654.3210"
}
array(4) {
  ["ID"]=>
  int(3)
  ["BIG"]=>
  int(9223372036854775807)
  ["NUM_10_2"]=>
  NULL
  ["DEC_12_4"]=>
  NULL
}
array(4) {
  ["ID"]=>
  int(4)
  ["BIG"]=>
  int(-2147483649)
  ["NUM_10_2"]=>
  string(12) "-21474836.49"
  ["DEC_12_4"]=>
  string(12) "-214748.3649"
}
array(4) {
  ["ID"]=>
  int(5)
  ["BIG"]=>
  int(-9223372036854775808)
  ["NUM_10_2"]=>
  NULL
  ["DEC_12_4"]=>
  NULL
}
array(4) {
  ["ID"]=>
  int(6)
  ["BIG"]=>
  int(0)
  ["NUM_10_2"]=>
  string(4) "0.00"
  ["DEC_12_4"]=>
  string(6) "0.0000"
}
