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
		<<<'SQL'
			MERGE INTO PROBE
			USING (SELECT 1 ID FROM RDB$DATABASE UNION ALL SELECT 3 FROM RDB$DATABASE) S
			ON PROBE.ID = S.ID
			WHEN MATCHED THEN UPDATE SET NAME = 'm'
			WHEN NOT MATCHED THEN INSERT (ID, NAME) VALUES (S.ID, 'n')
			SQL,
		"UPDATE OR INSERT INTO PROBE (ID, NAME) VALUES (5, 'u') MATCHING (ID)",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "ibase_query(): ", get_resource_type($r), "\n";
		echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
		echo "ibase_affected_rows(): ", var_export(ibase_affected_rows($t), true), "\n";
		echo "ibase_fetch_assoc(): ", var_export(ibase_fetch_assoc($r), true), "\n";
		echo "ibase_free_result(): ", var_export(ibase_free_result($r), true), "\n";
	}

	echo "prepared UPDATE\n";
	$q = ibase_prepare($t, "UPDATE PROBE SET NAME = ? WHERE ID = 1");
	$r = ibase_execute($q, 'z');
	echo "ibase_execute() returned the query: ", var_export($r === $q, true), "\n";
	echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($q), "\n";

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "SELECT\nibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
	dump_rows($r);

	echo "COMMIT as SQL: ", var_export(ibase_query($t, "COMMIT"), true), "\n";
})($c);

?>
--EXPECT--
CREATE TABLE: Firebird/InterBase query
INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 1
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
INSERT INTO PROBE (ID, NAME) VALUES (2, 'b')
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 1
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
UPDATE PROBE SET NAME = NAME || 'x'
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 2
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
UPDATE PROBE SET NAME = 'y' WHERE ID = 99
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 0
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
DELETE FROM PROBE WHERE ID = 2
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 1
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
MERGE INTO PROBE
USING (SELECT 1 ID FROM RDB$DATABASE UNION ALL SELECT 3 FROM RDB$DATABASE) S
ON PROBE.ID = S.ID
WHEN MATCHED THEN UPDATE SET NAME = 'm'
WHEN NOT MATCHED THEN INSERT (ID, NAME) VALUES (S.ID, 'n')
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 2
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
UPDATE OR INSERT INTO PROBE (ID, NAME) VALUES (5, 'u') MATCHING (ID)
ibase_query(): Firebird/InterBase query
ibase_query_affected_rows(): 1
ibase_affected_rows(): false
ibase_fetch_assoc(): false
ibase_free_result(): true
prepared UPDATE
ibase_execute() returned the query: true
ibase_query_affected_rows(): 1
SELECT
ibase_query_affected_rows(): 0
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(1) "z"
}
array(2) {
  ["ID"]=>
  int(3)
  ["NAME"]=>
  string(1) "n"
}
array(2) {
  ["ID"]=>
  int(5)
  ["NAME"]=>
  string(1) "u"
}
COMMIT as SQL: true
