--TEST--
ibase_query_affected_rows(): single-row ... RETURNING (Firebird 5+)
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbserver_lt(50);
?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");

/** @var string $test_base */
test_query_affected_rows_single_returning(ibase_connect($test_base));

?>
--EXPECT--
INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID
ibase_query_affected_rows(): 1
array(1) {
  ["ID"]=>
  int(2)
}
UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME
ibase_query_affected_rows(): 1
array(1) {
  ["NAME"]=>
  string(1) "c"
}
UPDATE PROBE SET NAME = 'x' WHERE ID = 99 RETURNING NAME
ibase_query_affected_rows(): 0
DELETE FROM PROBE WHERE ID = 2 RETURNING ID
ibase_query_affected_rows(): 1
array(1) {
  ["ID"]=>
  int(2)
}
SELECT
ibase_query_affected_rows(): 0
left:
array(2) {
  ["ID"]=>
  int(1)
  ["NAME"]=>
  string(1) "a"
}
