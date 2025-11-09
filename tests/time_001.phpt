--TEST--
Test unixtimestamp
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query("CREATE TABLE TTEST (
        ID INTEGER,
        T1 TIME DEFAULT '15:45:59',
        T2 TIMESTAMP DEFAULT '2025-11-06 15:45:59'
    )");
    ibase_commit();
    ibase_query("INSERT INTO TTEST (ID) VALUES (1)");
    dump_table_rows("TTEST");
    dump_table_rows("TTEST", null, IBASE_UNIXTIME);
})();

?>
--EXPECTF--
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  string(8) "15:45:59"
  ["T2"]=>
  string(19) "2025-11-06 15:45:59"
}
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  int(-%d)
  ["T2"]=>
  int(1762436759)
}