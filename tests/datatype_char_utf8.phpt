--TEST--
Check for data type CHAR(1) and UTF8
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php
	require("interbase.inc");

	$db = ibase_connect($test_base, );

    ibase_query(
    	"CREATE TABLE test_dt (
            v_char_utf8_1 CHAR(1) CHARACTER SET UTF8,
            v_char_utf8_10 CHAR(10) CHARACTER SET UTF8,
            v_varchar_utf8_1 VARCHAR(1) CHARACTER SET UTF8
        );");
    ibase_commit();

	ibase_query("insert into test_dt (v_char_utf8_1, v_char_utf8_10, v_varchar_utf8_1) values ('€', '  A   €   ', '€')");

    $sql = 'select * from test_dt';
    $query = ibase_query($sql);
    while(($row = ibase_fetch_assoc($query))) {
    	var_dump($row);
    }

    ibase_free_result($query);
    ibase_close();

?>
--EXPECTF--
array(3) {
  ["V_CHAR_UTF8_1"]=>
  string(3) "€"
  ["V_CHAR_UTF8_10"]=>
  string(12) "  A   €   "
  ["V_VARCHAR_UTF8_1"]=>
  string(3) "€"
}
