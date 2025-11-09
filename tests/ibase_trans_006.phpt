--TEST--
ibase_trans(): handles
--SKIPIF--
<?php
include("skipif.inc");
// On FB2.5 server "invalid transaction handle" happens on fetch.
// See also: tests/ibase_trans_013.phpt
// skip_if_fb_lt(3.0);
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function() {
    var_dump($t = ibase_query("SET TRANSACTION"));
    var_dump(ibase_rollback($t));
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
