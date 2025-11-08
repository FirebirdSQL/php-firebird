--TEST--
ibase_trans(): handles
--SKIPIF--
<?php
include("skipif.inc");
// TODO: Further investigation needed.
// On FB2.5 server "invalid transaction handle" happens on fetch.
// Related to tests/ibase_trans_012.phpt
skip_if_fb_lt(3.0);
?>
--FILE--
<?php

require("interbase.inc");

(function() {
    var_dump($t = ibase_trans());
    var_dump(ibase_query($t, "COMMIT"));
    var_dump($t);
    var_dump(ibase_query($t, "SELECT * FROM TEST1"));
})();

?>
--EXPECTF--
resource(12) of type (Firebird/InterBase transaction)
bool(true)
resource(%d) of type (Firebird/InterBase transaction)

Warning: ibase_query(): invalid transaction handle (expecting explicit transaction start)%s
bool(false)
