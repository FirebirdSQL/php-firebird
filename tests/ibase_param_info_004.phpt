--TEST--
ibase_param_info(): Basic test with ibase_query()
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

(function($test_base){
	ibase_connect($test_base);

	$rs = ibase_query('SELECT * FROM TEST1 WHERE 1 = ? OR 2 = ?', 1, 2);
	$count = ibase_num_params($rs);
	var_dump($count);
	for($i = 0; $i < $count; $i++){
		var_dump(ibase_field_info($rs, $i));
	}
})($test_base);

?>
--EXPECTF--
int(2)
array(10) {
  [0]=>
  string(1) "I"
  ["name"]=>
  string(1) "I"
  [1]=>
  string(1) "I"
  ["alias"]=>
  string(1) "I"
  [2]=>
  string(5) "TEST1"
  ["relation"]=>
  string(5) "TEST1"
  [3]=>
  string(1) "4"
  ["length"]=>
  string(1) "4"
  [4]=>
  string(7) "INTEGER"
  ["type"]=>
  string(7) "INTEGER"
}
array(10) {
  [0]=>
  string(1) "C"
  ["name"]=>
  string(1) "C"
  [1]=>
  string(1) "C"
  ["alias"]=>
  string(1) "C"
  [2]=>
  string(5) "TEST1"
  ["relation"]=>
  string(5) "TEST1"
  [3]=>
  string(3) "100"
  ["length"]=>
  string(3) "100"
  [4]=>
  string(7) "VARCHAR"
  ["type"]=>
  string(7) "VARCHAR"
}