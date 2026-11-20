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
		echo "    trans: ", ibase_affected_rows($t), "\n";
		echo "    query: ", ibase_query_affected_rows($r), "\n";
		while ($row = ibase_fetch_assoc($r)) {
			echo "    returned: ", json_encode($row), "\n";
		}
		echo "    query after fetch: ", ibase_query_affected_rows($r), "\n";
	}

	echo "prepared UPDATE ... RETURNING\n";
	$q = ibase_prepare($t, "UPDATE PROBE SET NAME = NAME || 'y' WHERE ID <= ? RETURNING ID");
	echo "    query before execute: ", ibase_query_affected_rows($q), "\n";
	$r = ibase_execute($q, 2);
	echo "    query: ", ibase_query_affected_rows($q), "\n";
	while ($row = ibase_fetch_assoc($r)) {
		echo "    returned: ", json_encode($row), "\n";
	}
	ibase_free_result($r);
	echo "    query after free_result: ", ibase_query_affected_rows($q), "\n";
	$r = ibase_execute($q, 1);
	echo "    query after 2nd execute: ", ibase_query_affected_rows($q), "\n";
	ibase_free_result($r);

	echo "DELETE ... RETURNING\n";
	$r = ibase_query($t, "DELETE FROM PROBE WHERE ID > 1 RETURNING ID");
	echo "    query: ", ibase_query_affected_rows($r), "\n";
	while ($row = ibase_fetch_assoc($r)) {
		echo "    returned: ", json_encode($row), "\n";
	}

	echo "MERGE ... RETURNING\n";
	$r = ibase_query($t, <<<'SQL'
		MERGE INTO PROBE
		USING (SELECT 1 ID FROM RDB$DATABASE UNION ALL SELECT 4 FROM RDB$DATABASE) S
		ON PROBE.ID = S.ID
		WHEN MATCHED THEN UPDATE SET NAME = NAME || 'm'
		WHEN NOT MATCHED THEN INSERT (ID, NAME) VALUES (S.ID, 'd')
		RETURNING PROBE.ID, PROBE.NAME
		SQL);
	echo "    trans: ", ibase_affected_rows($t), "\n";
	echo "    query: ", ibase_query_affected_rows($r), "\n";
	while ($row = ibase_fetch_assoc($r)) {
		echo "    returned: ", json_encode($row), "\n";
	}

	echo "INSERT ... SELECT ... RETURNING\n";
	$r = ibase_query($t, "INSERT INTO PROBE SELECT ID + 10, NAME FROM PROBE ORDER BY ID RETURNING ID, NAME");
	echo "    trans: ", ibase_affected_rows($t), "\n";
	echo "    query: ", ibase_query_affected_rows($r), "\n";
	while ($row = ibase_fetch_assoc($r)) {
		echo "    returned: ", json_encode($row), "\n";
	}

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	while ($row = ibase_fetch_assoc($r)) {
		echo "left: ", json_encode($row), "\n";
	}
	ibase_rollback($t);
})($c);

?>
--EXPECT--
UPDATE PROBE SET NAME = NAME || 'x' RETURNING ID, NAME
    trans: 0
    query: 3
    returned: {"ID":1,"NAME":"ax"}
    returned: {"ID":2,"NAME":"bx"}
    returned: {"ID":3,"NAME":"cx"}
    query after fetch: 3
SELECT ID, NAME FROM PROBE ORDER BY ID
    trans: 0
    query: 0
    returned: {"ID":1,"NAME":"ax"}
    returned: {"ID":2,"NAME":"bx"}
    returned: {"ID":3,"NAME":"cx"}
    query after fetch: 0
prepared UPDATE ... RETURNING
    query before execute: 0
    query: 2
    returned: {"ID":1}
    returned: {"ID":2}
    query after free_result: 2
    query after 2nd execute: 1
DELETE ... RETURNING
    query: 2
    returned: {"ID":2}
    returned: {"ID":3}
MERGE ... RETURNING
    trans: 0
    query: 2
    returned: {"ID":1,"NAME":"axyym"}
    returned: {"ID":4,"NAME":"d"}
INSERT ... SELECT ... RETURNING
    trans: 0
    query: 2
    returned: {"ID":11,"NAME":"axyym"}
    returned: {"ID":14,"NAME":"d"}
left: {"ID":1,"NAME":"axyym"}
left: {"ID":4,"NAME":"d"}
left: {"ID":11,"NAME":"axyym"}
left: {"ID":14,"NAME":"d"}
