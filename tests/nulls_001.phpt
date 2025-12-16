--TEST--
Test NULLs
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query(
    "CREATE TABLE ITEMS (
        ID INTEGER NOT null,
        CODE1 VARCHAR(32) CHARACTER SET NONE,
        CODE10 VARCHAR(32) CHARACTER SET NONE,
        MAN_ID INTEGER
    )");
    ibase_commit_ret();

    $data = [
        [1, "CODE1 1", "CODE10 1", null],
        [2, null, "CODE10 2", 101],
        [3, "CODE1 3", null, null],
        [4, null, null, 104],
        [5, "CODE1 5", null, 105],
        [6, "CODE1 6", "CODE10 6", null],
        [7, null, "CODE10 7", 107],
        [8, null, null, null],
        [9, "CODE1 8", "CODE10 9", 109],
    ];

    $p = ibase_prepare(
    "INSERT INTO ITEMS (
        ID, CODE1, CODE10, MAN_ID
    ) VALUES (?, ?, ?, ?)");

    foreach ($data as $row) {
        ibase_execute($p, ...$row);
    }

    dump_table_rows("ITEMS");
})();

?>
--EXPECT--
array(4) {
  ["ID"]=>
  int(1)
  ["CODE1"]=>
  string(7) "CODE1 1"
  ["CODE10"]=>
  string(8) "CODE10 1"
  ["MAN_ID"]=>
  NULL
}
array(4) {
  ["ID"]=>
  int(2)
  ["CODE1"]=>
  NULL
  ["CODE10"]=>
  string(8) "CODE10 2"
  ["MAN_ID"]=>
  int(101)
}
array(4) {
  ["ID"]=>
  int(3)
  ["CODE1"]=>
  string(7) "CODE1 3"
  ["CODE10"]=>
  NULL
  ["MAN_ID"]=>
  NULL
}
array(4) {
  ["ID"]=>
  int(4)
  ["CODE1"]=>
  NULL
  ["CODE10"]=>
  NULL
  ["MAN_ID"]=>
  int(104)
}
array(4) {
  ["ID"]=>
  int(5)
  ["CODE1"]=>
  string(7) "CODE1 5"
  ["CODE10"]=>
  NULL
  ["MAN_ID"]=>
  int(105)
}
array(4) {
  ["ID"]=>
  int(6)
  ["CODE1"]=>
  string(7) "CODE1 6"
  ["CODE10"]=>
  string(8) "CODE10 6"
  ["MAN_ID"]=>
  NULL
}
array(4) {
  ["ID"]=>
  int(7)
  ["CODE1"]=>
  NULL
  ["CODE10"]=>
  string(8) "CODE10 7"
  ["MAN_ID"]=>
  int(107)
}
array(4) {
  ["ID"]=>
  int(8)
  ["CODE1"]=>
  NULL
  ["CODE10"]=>
  NULL
  ["MAN_ID"]=>
  NULL
}
array(4) {
  ["ID"]=>
  int(9)
  ["CODE1"]=>
  string(7) "CODE1 8"
  ["CODE10"]=>
  string(8) "CODE10 9"
  ["MAN_ID"]=>
  int(109)
}
