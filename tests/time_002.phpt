--TEST--
Test unixtimestamp
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fb_lt(4) || skip_if_fbclient_lt(4);
?>
--FILE--
<?php

require("interbase.inc");

(function(){
    ibase_query("CREATE TABLE TTEST (
        ID INTEGER,
        T1 TIME WITH TIME ZONE DEFAULT '15:45:59 Europe/Riga',
        T2 TIMESTAMP WITH TIME ZONE DEFAULT '2025-11-06 15:45:59 Europe/Riga'
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
  string(20) "15:45:59 Europe/Riga"
  ["T2"]=>
  string(31) "2025-11-06 15:45:59 Europe/Riga"
}
array(3) {
  ["ID"]=>
  int(1)
  ["T1"]=>
  int(-%d)
  ["T2"]=>
  int(1762436759)
}