--TEST--
IBASE_PHPDATE returns DateTime objects; a date-less column gets the epoch date
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

    foreach (ibase_fetch_assoc(ibase_query("SELECT * FROM TTEST"), IBASE_PHPDATE) as $name => $v) {
        printf("%-3s %s %s\n", $name, get_class($v), $v->format('Y-m-d H:i:s P'));
    }

    // The format INI entries are for strings; IBASE_PHPDATE must not be affected by them
    ini_set('ibase.dateformat', 'd/m/Y');
    ini_set('ibase.timeformat', 'H.i.s');
    $row = ibase_fetch_assoc(ibase_query("SELECT * FROM TTEST"), IBASE_PHPDATE);
    var_dump($row['D'] instanceof DateTime, $row['D']->format('Y-m-d'));
})();

?>
--EXPECT--
D   DateTime 2025-11-06 00:00:00 +00:00
T   DateTime 1970-01-01 13:45:59 +00:00
TS  DateTime 2025-11-06 13:45:59 +00:00
bool(true)
string(10) "2025-11-06"
