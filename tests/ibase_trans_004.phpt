--TEST--
ibase_trans(): handles
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");

ibase_connect($test_base);

(function() {
    var_dump($t = ibase_trans());
    var_dump(ibase_query($t, "COMMIT"));
    var_dump($t);
    var_dump(ibase_query($t, "SELECT * FROM TEST1"));
})();

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase transaction)
bool(true)
resource(%d) of type (Firebird/InterBase transaction)

Warning: ibase_query(): invalid transaction handle (expecting explicit transaction start)%s
bool(false)
