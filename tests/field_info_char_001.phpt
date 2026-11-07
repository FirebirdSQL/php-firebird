--TEST--
ibase_field_info() reports CHAR for a CHAR column, which is fetched as VARCHAR
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE FIELDSTEST (C CHAR(10), V VARCHAR(10), I INTEGER)");
    ibase_commit();

    $q = ibase_prepare("SELECT C, V, I FROM FIELDSTEST");
    foreach ([0, 1, 2] as $i) {
        var_dump(ibase_field_info($q, $i)['type']);
    }

    // The declared type is reported, but the value still comes back space-padded to the
    // declared character count -- that is what reading CHAR as VARCHAR buys us.
    ibase_query("INSERT INTO FIELDSTEST VALUES ('AB', 'AB', 1)");
    ibase_commit();
    $row = ibase_fetch_row(ibase_query("SELECT C, V FROM FIELDSTEST"));
    var_dump($row[0], $row[1]);
})();

?>
--EXPECT--
string(4) "CHAR"
string(7) "VARCHAR"
string(7) "INTEGER"
string(10) "AB        "
string(2) "AB"
