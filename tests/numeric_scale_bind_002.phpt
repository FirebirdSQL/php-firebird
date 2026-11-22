--TEST--
Binding floats to scaled NUMERIC/DECIMAL and integer columns: full-precision repr must not overflow
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

ibase_query(
    "CREATE TABLE TTEST (
        SMALL_2 NUMERIC(4,2),
        NUM_2   NUMERIC(9,2),
        BIG_2   NUMERIC(18,2),
        SMALL_0 SMALLINT,
        INT_0   INTEGER,
        BIG_0   BIGINT
    )"
);
ibase_commit();

// A float is sent as text with serialize_precision, i.e. up to 17 significant digits:
// 0.1 + 0.2 travels as "0.30000000000000004". Firebird must still round it to the column.
$values = [
    '0.1 + 0.2'  => 0.1 + 0.2,
    '-0.1 - 0.2' => -0.1 - 0.2,
    '1.1 * 3'    => 1.1 * 3,
    '0.125'      => 0.125,
    '-0.125'     => -0.125,
    '1.005'      => 1.005,
    '2.675'      => 2.675,
    '1.5e-7'     => 1.5e-7,
    '99.995'     => 99.995,
    '327.67'     => 327.67,
    '-327.68'    => -327.68,
];

$cols = ['SMALL_2', 'NUM_2', 'BIG_2', 'SMALL_0', 'INT_0', 'BIG_0'];

foreach ($values as $label => $v) {
    foreach ($cols as $col) {
        ibase_query("DELETE FROM TTEST");
        if (!@ibase_query("INSERT INTO TTEST ($col) VALUES (?)", $v)) {
            printf("%-10s %-7s: ERR %s\n", $label, $col, preg_replace('/\s+/', ' ', trim(ibase_errmsg())));
            continue;
        }
        $got = ibase_fetch_row(ibase_query("SELECT $col FROM TTEST"))[0];
        printf("%-10s %-7s: %s\n", $label, $col, $got);
    }
}

?>
--EXPECT--
0.1 + 0.2  SMALL_2: 0.30
0.1 + 0.2  NUM_2  : 0.30
0.1 + 0.2  BIG_2  : 0.30
0.1 + 0.2  SMALL_0: 0
0.1 + 0.2  INT_0  : 0
0.1 + 0.2  BIG_0  : 0
-0.1 - 0.2 SMALL_2: -0.30
-0.1 - 0.2 NUM_2  : -0.30
-0.1 - 0.2 BIG_2  : -0.30
-0.1 - 0.2 SMALL_0: 0
-0.1 - 0.2 INT_0  : 0
-0.1 - 0.2 BIG_0  : 0
1.1 * 3    SMALL_2: 3.30
1.1 * 3    NUM_2  : 3.30
1.1 * 3    BIG_2  : 3.30
1.1 * 3    SMALL_0: 3
1.1 * 3    INT_0  : 3
1.1 * 3    BIG_0  : 3
0.125      SMALL_2: 0.13
0.125      NUM_2  : 0.13
0.125      BIG_2  : 0.13
0.125      SMALL_0: 0
0.125      INT_0  : 0
0.125      BIG_0  : 0
-0.125     SMALL_2: -0.13
-0.125     NUM_2  : -0.13
-0.125     BIG_2  : -0.13
-0.125     SMALL_0: 0
-0.125     INT_0  : 0
-0.125     BIG_0  : 0
1.005      SMALL_2: 1.01
1.005      NUM_2  : 1.01
1.005      BIG_2  : 1.01
1.005      SMALL_0: 1
1.005      INT_0  : 1
1.005      BIG_0  : 1
2.675      SMALL_2: 2.68
2.675      NUM_2  : 2.68
2.675      BIG_2  : 2.68
2.675      SMALL_0: 3
2.675      INT_0  : 3
2.675      BIG_0  : 3
1.5e-7     SMALL_2: 0.00
1.5e-7     NUM_2  : 0.00
1.5e-7     BIG_2  : 0.00
1.5e-7     SMALL_0: 0
1.5e-7     INT_0  : 0
1.5e-7     BIG_0  : 0
99.995     SMALL_2: 100.00
99.995     NUM_2  : 100.00
99.995     BIG_2  : 100.00
99.995     SMALL_0: 100
99.995     INT_0  : 100
99.995     BIG_0  : 100
327.67     SMALL_2: 327.67
327.67     NUM_2  : 327.67
327.67     BIG_2  : 327.67
327.67     SMALL_0: 328
327.67     INT_0  : 328
327.67     BIG_0  : 328
-327.68    SMALL_2: -327.68
-327.68    NUM_2  : -327.68
-327.68    BIG_2  : -327.68
-327.68    SMALL_0: -328
-327.68    INT_0  : -328
-327.68    BIG_0  : -328
