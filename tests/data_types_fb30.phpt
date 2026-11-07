--TEST--
DATA: fields introduced with FB 3.0
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbserver_lt(30);
skip_if_fbeffective_lt(30);
?>
--FILE--
<?php

require_once('config.inc');
ini_set('ibase.default_charset', "UTF8");

require("interbase.inc");
require("common.inc");

if (function_exists("mb_internal_encoding")) {
	mb_internal_encoding("UTF-8");
}

/** @var string $test_base */
ibase_connect($test_base);

(function() {
	ibase_query(file_get_contents(__DIR__."/001-FIELDS30.sql"));
	ibase_commit();

	$data = [
		0 => [ 'ID' => 0, 'BOOL_FIELD' => null ],
		1 => [ 'ID' => 1, 'BOOL_FIELD' => true ],
		2 => [ 'ID' => 2, 'BOOL_FIELD' => false ],
		3 => [ 'ID' => 3, 'BOOL_FIELD' => null ],
		4 => [ 'ID' => 4, 'BOOL_FIELD' => false ],
		5 => [ 'ID' => 5, 'BOOL_FIELD' => null ],
		6 => [ 'ID' => 6, 'BOOL_FIELD' => true ],
	];

	$sql = "INSERT INTO FIELDS30 (ID, BOOL_FIELD) VALUES (?, ?)";

	foreach ($data as $id => $item) {
		$item["ID"] = $id;
		if (false === ibase_query($sql, ...array_values($item))) {
			print "item="; var_dump($item);
			die("ibase_query failed");
		}
	}

	foreach ($data as $id => $item) {
		$q = ibase_query("SELECT * FROM FIELDS30 WHERE ID = ?", $id) or die("ibase_query failed");
		$row = ibase_fetch_assoc($q, IBASE_FETCH_BLOBS) or die("ibase_fetch_assoc failed");
		foreach ($item as $k => $v) {
			if (!array_key_exists($k, $row)) {
				print_r($row);
				die("$id missing column: $k");
			}

			if ($row[$k] !== $v) {
				print "$id column differs: $k\n";
				print "data="; var_dump($v);
				print "received from database="; var_dump($row[$k]);
			}
		}
	}
})();

?>
--EXPECT--
