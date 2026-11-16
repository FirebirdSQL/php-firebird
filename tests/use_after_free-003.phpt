--TEST--
InterBase: use after ibase_free_result() and ibase_free_query()
--SKIPIF--
<?php
include("skipif.inc");
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

require("interbase.inc");

$x = ibase_connect($test_base);

function try_call(string $label, callable $f) {
	try {
		var_dump($f());
	} catch (TypeError $e) {
		print "$label: " . $e->getMessage() . "\n";
	}
}

print "--- fetch after ibase_free_result() ---\n";
$r = ibase_query($x, "SELECT i FROM test1");
var_dump(ibase_free_result($r));
try_call("fetch", fn() => ibase_fetch_assoc($r));

print "--- ibase_free_result() twice ---\n";
$r = ibase_query($x, "SELECT i FROM test1");
var_dump(ibase_free_result($r));
try_call("free_result", fn() => ibase_free_result($r));

print "--- ibase_free_result() on ibase_execute() result closes only the cursor ---\n";
$q = ibase_prepare($x, "SELECT i FROM test1");
$r = ibase_execute($q);
var_dump(ibase_free_result($r));
var_dump(ibase_fetch_assoc($r));
$r = ibase_execute($q);
var_dump(ibase_fetch_assoc($r));
var_dump(ibase_free_result($r));
var_dump(ibase_free_result($r));
ibase_free_query($q);

print "--- execute after ibase_free_query() ---\n";
$q = ibase_prepare($x, "SELECT i FROM test1 WHERE i = ?");
var_dump(ibase_free_query($q));
try_call("execute", fn() => ibase_execute($q, 1));

print "--- ibase_free_query() twice ---\n";
$q = ibase_prepare($x, "SELECT i FROM test1 WHERE i = ?");
var_dump(ibase_free_query($q));
try_call("free_query", fn() => ibase_free_query($q));

print "--- connection still works ---\n";
var_dump(ibase_fetch_assoc(ibase_query($x, "SELECT i FROM test1")));

?>
--EXPECT--
--- fetch after ibase_free_result() ---
bool(true)
fetch: ibase_fetch_assoc(): supplied resource is not a valid Firebird/InterBase query resource
--- ibase_free_result() twice ---
bool(true)
free_result: ibase_free_result(): supplied resource is not a valid Firebird/InterBase query resource
--- ibase_free_result() on ibase_execute() result closes only the cursor ---
bool(true)
bool(false)
array(1) {
  ["I"]=>
  int(1)
}
bool(true)
bool(true)
--- execute after ibase_free_query() ---
bool(true)
execute: ibase_execute(): supplied resource is not a valid Firebird/InterBase query resource
--- ibase_free_query() twice ---
bool(true)
free_query: ibase_free_query(): supplied resource is not a valid Firebird/InterBase query resource
--- connection still works ---
array(1) {
  ["I"]=>
  int(1)
}
