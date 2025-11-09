--TEST--
ibase_trans(): handles
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function() {
    $t = ibase_query("SET TRANSACTION");
    var_dump($t);
})();

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase transaction)
