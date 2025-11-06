--TEST--
Long names: Firebird 4.0 or newer
--SKIPIF--
<?php
require_once("skipif.inc");
skip_if_fb_lt(4.0) || skip_if_fbclient_lt(4.0);
?>
--FILE--
<?php

require_once("interbase.inc");

// FB3: The maximum identifier length is 31 bytes
// FB4: The maximum identifier length is 63 characters character set UTF8 (252 bytes)
$MAX_LEN = 63;

function test_table(string $table){
	global $MAX_LEN;

	$c = 0;

	$fields = [
		str_repeat("F", $MAX_LEN),
		str_repeat("🥰", $MAX_LEN),
	];
	$fields_str = join('" INTEGER,"', $fields);
	$create_sql = sprintf('CREATE TABLE "%s" ("%s" INTEGER)', $table, $fields_str);

	if(ibase_query($create_sql)){
		ibase_commit();
	} else {
		var_dump($create_sql);
		die;
	}

	$data = []; foreach($fields as $f)$data[$f] = ++$c;
	insert_into($table, $data);
	print "Table:$table\n";
	dump_table_rows($table);
}

(function(){
	global $MAX_LEN;

	var_dump($MAX_LEN);

	test_table(str_repeat('T', $MAX_LEN));
	test_table(str_repeat('😂', $MAX_LEN));
})();

?>
--EXPECT--
int(63)
Table:TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
array(2) {
  ["FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"]=>
  int(1)
  ["🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰"]=>
  int(2)
}
Table:😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂😂
array(2) {
  ["FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"]=>
  int(1)
  ["🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰🥰"]=>
  int(2)
}