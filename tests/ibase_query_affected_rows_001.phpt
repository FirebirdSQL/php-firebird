--TEST--
ibase_query_affected_rows(): single-row INSERT / UPDATE / DELETE ... RETURNING
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
$c = ibase_connect($test_base);

(function($c) {
	ibase_query($c, 'CREATE TABLE PROBE (ID INT, NAME VARCHAR(10))');
	ibase_commit($c);

	$t = ibase_trans(IBASE_DEFAULT, $c);
	ibase_query($t, "INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')");

	$statements = [
		"INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID",
		"UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME",
		"UPDATE PROBE SET NAME = 'x' WHERE ID = 99 RETURNING NAME",
		"DELETE FROM PROBE WHERE ID = 2 RETURNING ID",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "    query: ", ibase_query_affected_rows($r), "\n";
		echo "    returned: ", json_encode(ibase_fetch_assoc($r)), "\n";
	}

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "SELECT\n    query: ", ibase_query_affected_rows($r), "\n";

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	while ($row = ibase_fetch_assoc($r)) {
		echo "left: ", json_encode($row), "\n";
	}
	ibase_rollback($t);
})($c);

?>
--EXPECTF--
INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID
    query: 1
    returned: {"ID":2}
UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME
    query: 1
    returned: {"NAME":"c"}
UPDATE PROBE SET NAME = 'x' WHERE ID = 99 RETURNING NAME
    query: 0
    returned: %s
DELETE FROM PROBE WHERE ID = 2 RETURNING ID
    query: 1
    returned: {"ID":2}
SELECT
    query: 0
left: {"ID":1,"NAME":"a"}
