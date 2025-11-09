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
        ["INSERT INTO TEST1 (I, C) VALUES (?, ?)", [1, "test2(1)"]],
        "SAVEPOINT sp_name",
        ["INSERT INTO TEST1 (I, C) VALUES (?, ?)", [2, "test2(2)"]],
    ];

    print "---- current status\n";
    ibase_query_bulk($queries);
    dump_table_rows("TEST1");

    print "---- now rollback\n";
    ibase_query_bulk([
        "ROLLBACK TO SAVEPOINT sp_name",
        "COMMIT",
    ]);
    dump_table_rows("TEST1");
})();

?>
--EXPECT--
---- current status
array(2) {
  ["I"]=>
  int(1)
  ["C"]=>
  string(8) "test2(1)"
}
array(2) {
  ["I"]=>
  int(2)
  ["C"]=>
  string(8) "test2(2)"
}
---- now rollback
array(2) {
  ["I"]=>
  int(1)
  ["C"]=>
  string(8) "test2(1)"
}