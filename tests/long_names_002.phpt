--TEST--
Long names: Firebird 3.0 or older
--SKIPIF--
<?php
require_once("skipif.inc");
skip_if_fb_gt(3.0);
?>
--FILE--
<?php

require_once("interbase.inc");

// FB 2.5, 3.0 identifier len is by byte count not character count
$MAX_LEN = 31;

function test_table(string $table){
	global $MAX_LEN;

	$c = 0;

	$fields = [
		'"'.str_repeat("F", $MAX_LEN).'"',
		'"'.str_repeat("🥰", intdiv($MAX_LEN, 4)).'ppp"', // 7*(utf 4 bytes)+3 padding
	];
	$fields_str = join(" INTEGER, ", $fields)." INTEGER";
	$create_sql = sprintf('CREATE TABLE "%s" (%s)', $table, $fields_str);

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

	test_table(str_repeat('T', $MAX_LEN));
})();

?>
--EXPECT--
Table:TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
array(2) {
  ["FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"]=>
  int(1)
  ["🥰🥰🥰🥰🥰🥰🥰ppp"]=>
  int(2)
}