--TEST--
Issue #112: Test for string buffer overflow
--SKIPIF--
<?php include(__DIR__."/skipif.inc"); ?>
--FILE--
<?php

require(__DIR__."/interbase.inc");
require(__DIR__."/common.inc");

ibase_connect($test_base);

(function() {
    ibase_query(
        "CREATE TABLE TTEST (
            TS TIMESTAMP,
            VARV VARCHAR(32765)
        )"
    );
    ibase_commit();

    // Max
    $len = 2**15 - 3;
    $data = str_repeat("!", $len);
    ibase_query("INSERT INTO TTEST (VARV) VALUES (?)", $data) or die("ibase_query failed");

    $r = ibase_fetch_object(ibase_query("SELECT * FROM TTEST"));
    printf("Got back %d bytes\n", strlen($r->VARV));
    assert($len == strlen($r->VARV));
    assert($r->VARV === $data);

    // Overflowing ISC_SHORT
    $data = str_repeat("!", 2**15);
    var_dump(ibase_query("INSERT INTO TTEST (VARV) VALUES (?)", $data));
})();

?>
--EXPECTF--
Got back 32765 bytes

Warning: ibase_query(): Parameter 1: string buffer overflow (32768 bytes, max 32765) in %s on line %d
bool(false)
