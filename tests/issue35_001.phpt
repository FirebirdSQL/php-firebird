--TEST--
Issue #35: ibase_prepare() fails to find table with SQL that has double quotes on table identifiers
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");

$db = ibase_connect($test_base);

function test35() {
	ibase_query('CREATE TABLE "test" (ID INTEGER, CLIENT_NAME VARCHAR(10))');
	ibase_commit();
	$p = ibase_prepare('INSERT INTO "test" (ID, CLIENT_NAME) VALUES (?, ?)');
	ibase_execute($p, 1, "Some name");
	$q = ibase_query('SELECT * FROM "test"');
	while($r = ibase_fetch_object($q)){
		var_dump($r);
	}
}

test35();

?>
--EXPECTF--
object(stdClass)#1 (2) {
  ["ID"]=>
  int(1)
  ["CLIENT_NAME"]=>
  string(9) "Some name"
}