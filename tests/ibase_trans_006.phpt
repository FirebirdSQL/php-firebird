--TEST--
ibase_trans(): handles
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

(function() {
    $t = ibase_query("SET TRANSACTION");
    ibase_rollback($t);
    var_dump($t);
    ibase_query($t, "SELECT * FROM TEST1");
})();

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase transaction)

Warning: ibase_query(): invalid transaction handle (expecting explicit transaction start)%s
