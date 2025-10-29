--TEST--
ibase_trans(): transaction control with SQL - commit default transaction
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

(function() {
    var_dump(ibase_query("COMMIT"));
    var_dump(ibase_query("COMMIT"));
})();

?>
--EXPECT--
bool(true)
bool(true)
