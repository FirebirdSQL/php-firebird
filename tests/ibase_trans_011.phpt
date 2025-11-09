--TEST--
ibase_trans(): transaction control with SQL - commit explicitly
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function() {
    var_dump($t = ibase_query("SET TRANSACTION"));
    var_dump(ibase_query($t, "COMMIT"));
    var_dump(ibase_query($t, "COMMIT"));
})();

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase transaction)
bool(true)

Warning: ibase_query(): Dynamic SQL Error SQL error code = -901 invalid transaction handle (expecting explicit transaction start)%s
bool(false)
