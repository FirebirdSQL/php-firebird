--TEST--
Check for data type INT128 (Firebird 4.0 or above)
--SKIPIF--
<?php
include("skipif.inc");
if(get_fb_version() < 4.0)print "skip FB < 4.0";
?>
--FILE--
<?php

    require("interbase.inc");

    $db = ibase_connect($test_base);

    ibase_query(
        "create table test_dt (
            v_int128    int128 not null
         )");
    ibase_commit();

    ibase_query("insert into test_dt (v_int128) values (1234)");
    ibase_query("insert into test_dt (v_int128) values (-170141183460469231731687303715884105728)");
    ibase_query("insert into test_dt (v_int128) values (170141183460469231731687303715884105727)");

    $sql = 'select * from test_dt';
    $query = ibase_query($sql);
    while(($row = ibase_fetch_assoc($query))) {
    	var_dump($row);
    }

    ibase_free_result($query);
    ibase_close();

?>
--EXPECTF--
array(1) {
  ["V_INT128"]=>
  string(4) "1234"
}
array(1) {
  ["V_INT128"]=>
  string(40) "-170141183460469231731687303715884105728"
}
array(1) {
  ["V_INT128"]=>
  string(39) "170141183460469231731687303715884105727"
}
