--TEST--
Binding integers to scaled NUMERIC/DECIMAL columns applies the column scale
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query(
        "CREATE TABLE TTEST (
            NUM_2 NUMERIC(10,2),
            NUM_4 NUMERIC(18,4),
            DEC_3 DECIMAL(9,3),
            SMALL_2 NUMERIC(4,2)
        )"
    );
    ibase_commit();

    // NUMERIC/DECIMAL are described as SQL_SHORT/LONG/INT64 with a negative scale and the
    // raw wire value is pre-scaled. Binding a PHP int must mean the value itself (5 => 5.00),
    // not the scaled representation (which would yield 0.05).
    ibase_query("INSERT INTO TTEST (NUM_2, NUM_4, DEC_3, SMALL_2) VALUES (?, ?, ?, ?)",
        5, 5, 5, 5) or die("ibase_query failed");

    ibase_query("INSERT INTO TTEST (NUM_2, NUM_4, DEC_3, SMALL_2) VALUES (?, ?, ?, ?)",
        -12345, -12345, -12345, -12) or die("ibase_query failed");

    // Booleans take the same path as integers, so they scale the same way (true => 1.00)
    ibase_query("INSERT INTO TTEST (NUM_2, NUM_4, DEC_3, SMALL_2) VALUES (?, ?, ?, ?)",
        true, false, true, false) or die("ibase_query failed");

    // Strings are left for Firebird to parse; these must come out the same either way
    ibase_query("INSERT INTO TTEST (NUM_2, NUM_4, DEC_3, SMALL_2) VALUES (?, ?, ?, ?)",
        '22.33', '22.33', '-7.5', '.5') or die("ibase_query failed");

    ibase_query("INSERT INTO TTEST (NUM_2, NUM_4, DEC_3, SMALL_2) VALUES (?, ?, ?, ?)",
        ' 5 ', '5e3', '22.3355', '1e-2') or die("ibase_query failed");

    dump_table_rows("TTEST");
})();

?>
--EXPECT--
array(4) {
  ["NUM_2"]=>
  string(4) "5.00"
  ["NUM_4"]=>
  string(6) "5.0000"
  ["DEC_3"]=>
  string(5) "5.000"
  ["SMALL_2"]=>
  string(4) "5.00"
}
array(4) {
  ["NUM_2"]=>
  string(9) "-12345.00"
  ["NUM_4"]=>
  string(11) "-12345.0000"
  ["DEC_3"]=>
  string(10) "-12345.000"
  ["SMALL_2"]=>
  string(6) "-12.00"
}
array(4) {
  ["NUM_2"]=>
  string(4) "1.00"
  ["NUM_4"]=>
  string(6) "0.0000"
  ["DEC_3"]=>
  string(5) "1.000"
  ["SMALL_2"]=>
  string(4) "0.00"
}
array(4) {
  ["NUM_2"]=>
  string(5) "22.33"
  ["NUM_4"]=>
  string(7) "22.3300"
  ["DEC_3"]=>
  string(6) "-7.500"
  ["SMALL_2"]=>
  string(4) "0.50"
}
array(4) {
  ["NUM_2"]=>
  string(4) "5.00"
  ["NUM_4"]=>
  string(9) "5000.0000"
  ["DEC_3"]=>
  string(6) "22.336"
  ["SMALL_2"]=>
  string(4) "0.01"
}
