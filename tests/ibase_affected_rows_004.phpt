--TEST--
ibase_affected_rows(): after multi-row UPDATE / DELETE ... RETURNING (Firebird 5+)
--SKIPIF--
<?php
include("skipif.inc");
if (get_fb_version() < 50) die("skip multi-row RETURNING needs Firebird 5.0 or newer");
?>
--XFAIL--
Firebird 5 describes UPDATE / DELETE ... RETURNING as isc_info_sql_stmt_select
(see doc/sql.extensions/README.returning, note 3), whether they change one row or many.
The extension only reads the row count for insert / update / delete / exec_procedure
statements, so ibase_affected_rows() returns 0 here. Asking the server for the count on
every SELECT would cost a round trip each; use ibase_query_affected_rows() to get it.
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

	$statements = [
		"UPDATE PROBE SET NAME = NAME || 'x' RETURNING ID, NAME",
		"DELETE FROM PROBE WHERE ID > 1 RETURNING ID",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "    affected: ", ibase_affected_rows($t), "\n";
		while ($row = ibase_fetch_assoc($r)) {
			echo "    returned: ", json_encode($row), "\n";
		}
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
    affected: 3
    returned: {"ID":1,"NAME":"ax"}
    returned: {"ID":2,"NAME":"bx"}
    returned: {"ID":3,"NAME":"cx"}
DELETE FROM PROBE WHERE ID > 1 RETURNING ID
    affected: 2
    returned: {"ID":2}
    returned: {"ID":3}
left: {"ID":1,"NAME":"ax"}
