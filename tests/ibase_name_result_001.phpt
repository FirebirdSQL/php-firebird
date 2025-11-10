--TEST--
ibase_name_result(): basic test
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    $rowc = 2;
    ibase_query("DELETE FROM TEST1");
    $p = ibase_prepare("INSERT INTO TEST1 (i, c) VALUES (?, ?)");
    for($i = 1; $i <= $rowc; $i++){
        ibase_execute($p, $i, "row$i");
    }
    print "---- init ----\n";
    dump_table_rows("TEST1");

    $q = ibase_query("SELECT * FROM TEST1 FOR UPDATE");
    ibase_name_result($q, "curs");

    $p = ibase_prepare("UPDATE TEST1 SET i = ?, c = ? WHERE CURRENT OF curs");
    for ($i = 1; ibase_fetch_row($q); ++$i) {
        ibase_execute($p, $i*2, "row$i/".($i * 2));
    }

    print "---- after update ----\n";
    dump_table_rows("TEST1");
})();

?>
--EXPECT--
---- init ----
array(2) {
  ["I"]=>
  int(1)
  ["C"]=>
  string(4) "row1"
}
array(2) {
  ["I"]=>
  int(2)
  ["C"]=>
  string(4) "row2"
}
---- after update ----
array(2) {
  ["I"]=>
  int(2)
  ["C"]=>
  string(6) "row1/2"
}
array(2) {
  ["I"]=>
  int(4)
  ["C"]=>
  string(6) "row2/4"
}