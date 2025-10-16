--TEST--
Issue #77: Order of INSERT parameters cause "Incorrect values within SQLDA structure"
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");

$db = ibase_connect($test_base);

function test77() {
	ibase_query('CREATE TABLE TTEST (TTYPE VARCHAR(1) NOT NULL, F8 NUMERIC(9,2))');
	ibase_commit();

	[$fields_str, $q_str, $data] = array2sql_parts([
		'TTYPE'=>'1',
		'F8'=>'',
	]);
	ibase_query("INSERT INTO TTEST($fields_str) VALUES ($q_str)", ...array_values($data));
	dump_table_rows("TTEST");

	[$fields_str, $q_str, $data] = array2sql_parts([
		'F8'=>'',
		'TTYPE'=>'2',
	]);
	ibase_query("INSERT INTO TTEST($fields_str) VALUES ($q_str)", ...array_values($data));
	dump_table_rows("TTEST");
}

test77();

?>
--EXPECT--
array(2) {
  [0]=>
  string(1) "1"
  [1]=>
  NULL
}
array(2) {
  [0]=>
  string(1) "1"
  [1]=>
  NULL
}
array(2) {
  [0]=>
  string(1) "2"
  [1]=>
  NULL
}