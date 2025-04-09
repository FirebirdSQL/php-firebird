--TEST--
ibase_trans(): Check order of link identifier and trans args
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

$db = ibase_connect($test_base);
$tr = ibase_trans(IBASE_READ, $db) or die("Could not create transaction");
@ibase_query($tr, "INSERT INTO test1 VALUES(1, 2)") or die("Could not insert");
ibase_commit($tr) or die("Could not commit transaction");
print "Finished OK\n";

unset($db);

?>
--EXPECT--
Could not insert
