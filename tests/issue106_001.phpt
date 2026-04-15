--TEST--
Issue #106: _php_ibase_bind() loses NULL flag when handling SQL_TEXT fallthrough
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");

$db = ibase_connect($test_base);

function test106() {
	// Nullable VARCHAR column: the bind fallthrough path sets SQL_TEXT,
	// which must preserve the nullable bit (sqltype & 1).
	ibase_query('CREATE TABLE test106 (ID INTEGER, VAL VARCHAR(50))');
	ibase_commit();

	$insert = ibase_prepare('INSERT INTO test106 (ID, VAL) VALUES (?, ?)');

	// First execute: non-null value triggers the SQL_TEXT fallthrough path
	ibase_execute($insert, 1, 'hello');

	// Second execute: NULL must be passed correctly;
	// if the nullable bit was lost in the first call, this silently inserts
	// the old value instead of NULL.
	ibase_execute($insert, 2, null);

	// Third execute: non-null again to verify the prepared statement still works
	ibase_execute($insert, 3, 'world');

	ibase_commit();

	$res = ibase_query('SELECT ID, VAL FROM test106 ORDER BY ID');
	while ($row = ibase_fetch_row($res)) {
		$val = $row[1] === null ? 'NULL' : "'" . $row[1] . "'";
		echo "ID={$row[0]} VAL=$val\n";
	}
	ibase_free_result($res);
	ibase_free_query($insert);
}

test106();

?>
--EXPECT--
ID=1 VAL='hello'
ID=2 VAL=NULL
ID=3 VAL='world'
