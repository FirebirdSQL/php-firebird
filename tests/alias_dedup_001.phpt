--TEST--
Repeated column aliases are made unique for ibase_fetch_assoc() keys
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    // Four columns with the same alias, plus unnamed literals, which Firebird itself
    // calls CONSTANT. An array can hold each name once, so all but the first get a suffix.
    $r = ibase_query(
        "SELECT 1 AS ID, 2 AS ID, 3 AS ID, 4 AS ID, 5, 6 FROM RDB\$DATABASE");
    $row = ibase_fetch_assoc($r);
    var_dump(array_keys($row));
    var_dump(array_values($row));

    // A name that only collides after suffixing still has to come out unique
    $r = ibase_query("SELECT 1 AS ID, 2 AS ID_01, 3 AS ID, 4 AS ID FROM RDB\$DATABASE");
    var_dump(array_keys(ibase_fetch_assoc($r)));
})();

?>
--EXPECT--
array(6) {
  [0]=>
  string(2) "ID"
  [1]=>
  string(5) "ID_01"
  [2]=>
  string(5) "ID_02"
  [3]=>
  string(5) "ID_03"
  [4]=>
  string(8) "CONSTANT"
  [5]=>
  string(11) "CONSTANT_01"
}
array(6) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(4)
  [4]=>
  int(5)
  [5]=>
  int(6)
}
array(4) {
  [0]=>
  string(2) "ID"
  [1]=>
  string(5) "ID_01"
  [2]=>
  string(5) "ID_02"
  [3]=>
  string(5) "ID_03"
}
