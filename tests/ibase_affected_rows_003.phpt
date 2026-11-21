--TEST--
ibase_affected_rows(): after single-row ... RETURNING
--SKIPIF--
<?php
include("skipif.inc");
/*
 * Firebird 5 describes ... RETURNING statements as isc_info_sql_stmt_select
 * (see doc/sql.extensions/README.returning, note 3). The extension only reads the
 * row count for insert / update / delete / exec_procedure statements, so
 * ibase_affected_rows() returns 0 for them there. Asking the server for the count on
 * every SELECT would cost a round trip each; use ibase_query_affected_rows() to get it.
 */
if (get_fb_version() >= 50) echo "xfail Firebird 5 describes ... RETURNING as SELECT";
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
		echo "ibase_affected_rows(): ", ibase_affected_rows($t), "\n";
		if (is_resource($r)) {
			dump_rows($r);
		}
	}

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "left:\n";
	dump_rows($r);
	ibase_rollback($t);
})($c);

?>
--EXPECT--
INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')
ibase_affected_rows(): 1
INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID
ibase_affected_rows(): 1
array(1) {
  ["ID"]=>
  int(2)
}
UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME
ibase_affected_rows(): 1
array(1) {
  ["NAME"]=>
  string(1) "c"
}
UPDATE PROBE SET NAME = 'd' WHERE ID = 1
ibase_affected_rows(): 1
DELETE FROM PROBE WHERE ID = 2 RETURNING ID
ibase_affected_rows(): 1
array(1) {
  ["ID"]=>
  int(2)
}
left:
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(1) "d"
}
