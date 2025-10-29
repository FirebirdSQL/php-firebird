--TEST--
ibase_trans(): handles
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

(function() {
    $t = ibase_trans();
    var_dump(ibase_query($t, "COMMIT"));
    var_dump($t);
    ibase_query($t, "SELECT * FROM TEST1");
})();

?>
--EXPECTF--
bool(true)
resource(%d) of type (Firebird/InterBase transaction)

Warning: ibase_query(): invalid transaction handle (expecting explicit transaction start)%s
