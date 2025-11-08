--TEST--
ibase_trans(): handles
--SKIPIF--
<?php
include("skipif.inc");
// On FB2.5 server "invalid transaction handle" happens on fetch.
// TODO: Further investigation needed.
// Related to tests/ibase_trans_013.phpt
skip_if_fb_gt(2.5);
?>
--FILE--
<?php

require("interbase.inc");

(function() {
    var_dump($t = ibase_query("SET TRANSACTION"));
    var_dump(ibase_rollback($t));
    var_dump($t);
    var_dump($q = ibase_query($t, "SELECT * FROM TEST1"));
	var_dump(ibase_fetch_assoc($q));
})();

?>
--EXPECTF--
resource(%d) of type (Firebird/InterBase transaction)
bool(true)
resource(%d) of type (Firebird/InterBase transaction)
resource(%d) of type (interbase %s)

Warning: ibase_fetch_assoc(): Dynamic SQL Error SQL error code = -901 invalid transaction handle (expecting explicit transaction start) %s
bool(false)
