--TEST--
ibase_query_affected_rows(): rows changed by a statement Firebird 5 describes as SELECT
--SKIPIF--
<?php
include("skipif.inc");
if (get_fb_version() < 50) die("skip multi-row RETURNING needs Firebird 5.0 or newer");
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
	ibase_query($t, "INSERT INTO PROBE (ID, NAME) VALUES (2, 'b')");
	ibase_query($t, "INSERT INTO PROBE (ID, NAME) VALUES (3, 'c')");

	// Firebird 5 describes these as SELECT, so only the per-query count sees them
	$statements = [
		"UPDATE PROBE SET NAME = NAME || 'x' RETURNING ID, NAME",
		"SELECT ID, NAME FROM PROBE ORDER BY ID",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "ibase_affected_rows(): ", ibase_affected_rows($t), "\n";
		echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
		dump_rows($r);
		echo "ibase_query_affected_rows() after fetch: ", ibase_query_affected_rows($r), "\n";
	}

	echo "prepared UPDATE ... RETURNING\n";
	$q = ibase_prepare($t, "UPDATE PROBE SET NAME = NAME || 'y' WHERE ID <= ? RETURNING ID");
	echo "ibase_query_affected_rows() before execute: ", ibase_query_affected_rows($q), "\n";
	$r = ibase_execute($q, 2);
	echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($q), "\n";
	dump_rows($r);
	ibase_free_result($r);
	echo "ibase_query_affected_rows() after free_result: ", ibase_query_affected_rows($q), "\n";
	$r = ibase_execute($q, 1);
	echo "ibase_query_affected_rows() after 2nd execute: ", ibase_query_affected_rows($q), "\n";
	ibase_free_result($r);

	echo "DELETE ... RETURNING\n";
	$r = ibase_query($t, "DELETE FROM PROBE WHERE ID > 1 RETURNING ID");
	echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
	dump_rows($r);

	echo "MERGE ... RETURNING\n";
	$r = ibase_query($t, <<<'SQL'
		MERGE INTO PROBE
		USING (SELECT 1 ID FROM RDB$DATABASE UNION ALL SELECT 4 FROM RDB$DATABASE) S
		ON PROBE.ID = S.ID
		WHEN MATCHED THEN UPDATE SET NAME = NAME || 'm'
		WHEN NOT MATCHED THEN INSERT (ID, NAME) VALUES (S.ID, 'd')
		RETURNING PROBE.ID, PROBE.NAME
		SQL);
	echo "ibase_affected_rows(): ", ibase_affected_rows($t), "\n";
	echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
	dump_rows($r);

	echo "INSERT ... SELECT ... RETURNING\n";
	$r = ibase_query($t, "INSERT INTO PROBE SELECT ID + 10, NAME FROM PROBE ORDER BY ID RETURNING ID, NAME");
	echo "ibase_affected_rows(): ", ibase_affected_rows($t), "\n";
	echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
	dump_rows($r);

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "left:\n";
	dump_rows($r);
	ibase_rollback($t);
})($c);

?>
--EXPECT--
UPDATE PROBE SET NAME = NAME || 'x' RETURNING ID, NAME
ibase_affected_rows(): 0
ibase_query_affected_rows(): 3
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(2) "ax"
}
array(2) {
  ["ID"]=>
  int(2)
  ["NAME"]=>
  string(2) "bx"
}
array(2) {
  ["ID"]=>
  int(3)
  ["NAME"]=>
  string(2) "cx"
}
ibase_query_affected_rows() after fetch: 3
SELECT ID, NAME FROM PROBE ORDER BY ID
ibase_affected_rows(): 0
ibase_query_affected_rows(): 0
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(2) "ax"
}
array(2) {
  ["ID"]=>
  int(2)
  ["NAME"]=>
  string(2) "bx"
}
array(2) {
  ["ID"]=>
  int(3)
  ["NAME"]=>
  string(2) "cx"
}
ibase_query_affected_rows() after fetch: 0
prepared UPDATE ... RETURNING
ibase_query_affected_rows() before execute: 0
ibase_query_affected_rows(): 2
array(1) {
  ["ID"]=>
  int(1)
}
array(1) {
  ["ID"]=>
  int(2)
}
ibase_query_affected_rows() after free_result: 2
ibase_query_affected_rows() after 2nd execute: 1
DELETE ... RETURNING
ibase_query_affected_rows(): 2
array(1) {
  ["ID"]=>
  int(2)
}
array(1) {
  ["ID"]=>
  int(3)
}
MERGE ... RETURNING
ibase_affected_rows(): 0
ibase_query_affected_rows(): 2
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(5) "axyym"
}
array(2) {
  ["ID"]=>
  int(4)
  ["NAME"]=>
  string(1) "d"
}
INSERT ... SELECT ... RETURNING
ibase_affected_rows(): 0
ibase_query_affected_rows(): 2
array(2) {
  ["ID"]=>
  int(11)
  ["NAME"]=>
  string(5) "axyym"
}
array(2) {
  ["ID"]=>
  int(14)
  ["NAME"]=>
  string(1) "d"
}
left:
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(5) "axyym"
}
array(2) {
  ["ID"]=>
  int(4)
  ["NAME"]=>
  string(1) "d"
}
array(2) {
  ["ID"]=>
  int(11)
  ["NAME"]=>
  string(5) "axyym"
}
array(2) {
  ["ID"]=>
  int(14)
  ["NAME"]=>
  string(1) "d"
}
