--TEST--
Longest identifiers Firebird 4.0+ accepts (63 characters)
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
?>
--FILE--
<?php

require_once('config.inc');
ini_set('ibase.default_charset', "UTF8");

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    // Metadata is UTF8 and capped at 63 characters whatever they are, so 63 emoji --
    // 252 bytes -- is a legal name.
    $table  = str_repeat('T', 63);
    $column = str_repeat('F', 63);
    $wide   = str_repeat("\u{1F602}", 63);
    $alias  = str_repeat('A', 63);

    ibase_query("CREATE TABLE \"$table\" ($column INTEGER, \"$wide\" INTEGER)") or die("create: " . ibase_errmsg());
    ibase_commit();
    ibase_query("INSERT INTO \"$table\" VALUES (1, 2)") or die("insert: " . ibase_errmsg());
    ibase_commit();

    $q = ibase_prepare("SELECT $column AS \"$alias\", \"$wide\" FROM \"$table\" WHERE $column = ?");

    // Nothing is truncated on the way back out
    $f = ibase_field_info($q, 0);
    var_dump($f['name'] === $column, $f['alias'] === $alias, $f['relation'] === $table);
    $f = ibase_field_info($q, 1);
    var_dump($f['name'] === $wide, $f['alias'] === $wide, $f['relation'] === $table);

    // ... including as array keys
    $row = ibase_fetch_assoc(ibase_execute($q, 1));
    var_dump(array_keys($row) === [$alias, $wide], array_values($row));

    // Byte lengths, so a truncating regression shows up as a number, not as false
    var_dump(strlen($table), strlen($wide));
})();

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
array(2) {
  [0]=>
  int(1)
  [1]=>
  int(2)
}
int(63)
int(252)
