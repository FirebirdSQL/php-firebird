--TEST--
ibase.legacy_flags defaults to every legacy behaviour being enabled
--SKIPIF--
<?php if (!extension_loaded("interbase")) die("skip interbase extension not available"); ?>
--FILE--
<?php

// Deliberately does not load tests/config.inc, which turns the legacy flags off for the
// rest of the suite: this checks the value the extension actually ships with.
var_dump(ini_get('ibase.legacy_flags'));

var_dump(IBASE_LEGACY_NONE, IBASE_LEGACY_ALL, IBASE_LEGACY_UNIXTIMESTAMP);

// Every flag must read as set under the default
var_dump((int)ini_get('ibase.legacy_flags') & IBASE_LEGACY_UNIXTIMESTAMP ? true : false);

?>
--EXPECT--
string(2) "-1"
int(0)
int(-1)
int(1)
bool(true)
