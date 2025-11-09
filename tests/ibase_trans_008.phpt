--TEST--
ibase_trans(): transaction control with SQL
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function() {
    $queries = [
        "SET TRANSACTION",
        "DELETE FROM TEST1",
        "COMMIT RETAIN",
        ["INSERT INTO TEST1 (I, C) VALUES (?, ?)", [1, "test1(1)"]],
        "SAVEPOINT sp_name",
        ["INSERT INTO TEST1 (I, C) VALUES (?, ?)", [2, "test1(2)"]],
        "ROLLBACK TO SAVEPOINT sp_name",
        "COMMIT",
    ];
    ibase_query_bulk($queries);
    dump_table_rows("TEST1");
})();

?>
--EXPECT--
array(2) {
  ["I"]=>
  int(1)
  ["C"]=>
  string(8) "test1(1)"
}