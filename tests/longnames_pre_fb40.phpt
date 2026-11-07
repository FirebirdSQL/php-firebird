--TEST--
Longest identifiers before Firebird 4.0 (31 bytes)
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbserver_gte(40);
?>
--FILE--
<?php

require_once('config.inc');
ini_set('ibase.default_charset', "UTF8");

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    // Metadata is UNICODE_FSS and capped at 31 bytes, so a 2-byte character fits 15
    // times. Characters outside the BMP cannot be used in an identifier at all here.
    $table  = str_repeat('T', 31);
    $column = str_repeat('F', 31);
    $wide   = str_repeat("\u{00E4}", 15);
    $alias  = str_repeat('A', 31);

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
int(31)
int(30)
