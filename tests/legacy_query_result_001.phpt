--TEST--
ibase.legacy_flags without IBASE_LEGACY_QUERY_RESULT: every statement returns its handle
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");

ini_set('ibase.legacy_flags', IBASE_LEGACY_ALL & ~IBASE_LEGACY_QUERY_RESULT);

/** @var string $test_base */
$c = ibase_connect($test_base);

(function($c) {
	$r = ibase_query($c, 'CREATE TABLE PROBE (ID INT, NAME VARCHAR(10))');
	echo "CREATE TABLE: ", get_resource_type($r), "\n";
	ibase_commit($c);

	$t = ibase_trans(IBASE_DEFAULT, $c);
	$statements = [
		"INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')",
		"INSERT INTO PROBE (ID, NAME) VALUES (2, 'b')",
		"UPDATE PROBE SET NAME = NAME || 'x'",
		"UPDATE PROBE SET NAME = 'y' WHERE ID = 99",
		"DELETE FROM PROBE WHERE ID = 2",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "    returned: ", get_resource_type($r), "\n";
		echo "    query affected: ", ibase_query_affected_rows($r), "\n";
		echo "    trans affected: ", var_export(ibase_affected_rows($t), true), "\n";
		echo "    fetch: ", var_export(ibase_fetch_assoc($r), true), "\n";
		echo "    free: ", var_export(ibase_free_result($r), true), "\n";
	}

	echo "prepared UPDATE\n";
	$q = ibase_prepare($t, "UPDATE PROBE SET NAME = ? WHERE ID = 1");
	$r = ibase_execute($q, 'z');
	echo "    returned the query: ", var_export($r === $q, true), "\n";
	echo "    query affected: ", ibase_query_affected_rows($q), "\n";

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "SELECT\n    query affected: ", ibase_query_affected_rows($r), "\n";
	while ($row = ibase_fetch_assoc($r)) {
		echo "    row: ", json_encode($row), "\n";
	}

	echo "COMMIT as SQL: ", var_export(ibase_query($t, "COMMIT"), true), "\n";
})($c);

?>
--EXPECT--
CREATE TABLE: Firebird/InterBase query
INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')
    returned: Firebird/InterBase query
    query affected: 1
    trans affected: false
    fetch: false
    free: true
INSERT INTO PROBE (ID, NAME) VALUES (2, 'b')
    returned: Firebird/InterBase query
    query affected: 1
    trans affected: false
    fetch: false
    free: true
UPDATE PROBE SET NAME = NAME || 'x'
    returned: Firebird/InterBase query
    query affected: 2
    trans affected: false
    fetch: false
    free: true
UPDATE PROBE SET NAME = 'y' WHERE ID = 99
    returned: Firebird/InterBase query
    query affected: 0
    trans affected: false
    fetch: false
    free: true
DELETE FROM PROBE WHERE ID = 2
    returned: Firebird/InterBase query
    query affected: 1
    trans affected: false
    fetch: false
    free: true
prepared UPDATE
    returned the query: true
    query affected: 1
SELECT
    query affected: 0
    row: {"ID":1,"NAME":"z"}
COMMIT as SQL: true
