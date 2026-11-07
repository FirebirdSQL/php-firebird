--TEST--
ibase.reuse_connections controls whether ibase_connect() hands back an open link
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base, $user, $password */
$args = [$test_base, $user, $password, ''];

// On by default: two calls with the same arguments share one attachment
var_dump(ini_get('ibase.reuse_connections'));
$c1 = ibase_connect(...$args);
$c2 = ibase_connect(...$args);
var_dump($c1 === $c2);

// Off: every call gets its own, and none of them is the one opened above
ini_set('ibase.reuse_connections', '0');
$c3 = ibase_connect(...$args);
$c4 = ibase_connect(...$args);
var_dump($c3 === $c4, $c3 === $c1);

// Closing one leaves the others alone
ibase_close($c3);
var_dump((bool) @ibase_query($c1, 'SELECT 1 FROM RDB$DATABASE'));
var_dump((bool) @ibase_query($c4, 'SELECT 1 FROM RDB$DATABASE'));

// Back on: reuse finds $c1 again, but never a link opened while it was off
ini_set('ibase.reuse_connections', '1');
$c5 = ibase_connect(...$args);
var_dump($c5 === $c1, $c5 === $c4);

ibase_close($c4);
ibase_close($c1);

?>
--EXPECT--
string(1) "1"
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
bool(true)
bool(false)
