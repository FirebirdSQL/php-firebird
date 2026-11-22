--TEST--
Fetching scaled NUMERIC values: negative fractions and type min/max
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

// Scaled columns are fetched as strings on 32-bit and 64-bit alike
$cases = [
    'NUMERIC(4,2)'   => ['-0.05', '-0.01', '0.05', '-1.00', '-327.68', '327.67'],
    'NUMERIC(9,2)'   => ['-0.05', '-0.01', '0.05', '-1.00', '-21474836.48', '21474836.47'],
    'NUMERIC(18,2)'  => ['-0.05', '-0.01', '0.05', '-1.00', '-92233720368547758.08', '92233720368547758.07'],
    'NUMERIC(18,18)' => ['-0.000000000000000001', '-0.100000000000000000', '-9.223372036854775808', '9.223372036854775807'],
];

foreach ($cases as $type => $values) {
    foreach ($values as $v) {
        $got = ibase_fetch_row(ibase_query("SELECT CAST('$v' AS $type) FROM RDB\$DATABASE"))[0];
        printf("%s %s: %s\n", $type, $v, $got === $v ? 'OK' : 'FAIL, got ' . var_export($got, true));
    }
}

?>
--EXPECT--
NUMERIC(4,2) -0.05: OK
NUMERIC(4,2) -0.01: OK
NUMERIC(4,2) 0.05: OK
NUMERIC(4,2) -1.00: OK
NUMERIC(4,2) -327.68: OK
NUMERIC(4,2) 327.67: OK
NUMERIC(9,2) -0.05: OK
NUMERIC(9,2) -0.01: OK
NUMERIC(9,2) 0.05: OK
NUMERIC(9,2) -1.00: OK
NUMERIC(9,2) -21474836.48: OK
NUMERIC(9,2) 21474836.47: OK
NUMERIC(18,2) -0.05: OK
NUMERIC(18,2) -0.01: OK
NUMERIC(18,2) 0.05: OK
NUMERIC(18,2) -1.00: OK
NUMERIC(18,2) -92233720368547758.08: OK
NUMERIC(18,2) 92233720368547758.07: OK
NUMERIC(18,18) -0.000000000000000001: OK
NUMERIC(18,18) -0.100000000000000000: OK
NUMERIC(18,18) -9.223372036854775808: OK
NUMERIC(18,18) 9.223372036854775807: OK
