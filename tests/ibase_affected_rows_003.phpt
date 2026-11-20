--TEST--
ibase_affected_rows(): after single-row INSERT / UPDATE / DELETE ... RETURNING
--SKIPIF--
<?php
include("skipif.inc");
/*
 * Firebird 5 describes UPDATE / DELETE ... RETURNING as isc_info_sql_stmt_select
 * (see doc/sql.extensions/README.returning, note 3). The extension only reads the
 * row count for insert / update / delete / exec_procedure statements, so
 * ibase_affected_rows() returns 0 for them there. Asking the server for the count on
 * every SELECT would cost a round trip each; use ibase_query_affected_rows() to get it.
 */
if (get_fb_version() >= 50) echo "xfail Firebird 5 describes UPDATE / DELETE ... RETURNING as SELECT";
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
	$statements = [
		"INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')",
		"INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID",
		"UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME",
		"UPDATE PROBE SET NAME = 'd' WHERE ID = 1",
		"DELETE FROM PROBE WHERE ID = 2 RETURNING ID",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "    affected: ", ibase_affected_rows($t), "\n";
		if (is_resource($r)) {
			echo "    returned: ", json_encode(ibase_fetch_assoc($r)), "\n";
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
INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')
    affected: 1
INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID
    affected: 1
    returned: {"ID":2}
UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME
    affected: 1
    returned: {"NAME":"c"}
UPDATE PROBE SET NAME = 'd' WHERE ID = 1
    affected: 1
DELETE FROM PROBE WHERE ID = 2 RETURNING ID
    affected: 1
    returned: {"ID":2}
left: {"ID":1,"NAME":"d"}
