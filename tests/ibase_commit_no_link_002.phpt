--TEST--
ibase_commit()/ibase_rollback(): no default link must not crash (PHP 7)
--SKIPIF--
<?php
if (!extension_loaded("interbase")) print "skip interbase extension not available";
require(__DIR__."/functions.inc");

// See also tests/ibase_commit_no_link_001.phpt
skip_if_php_gte(8);
?>
--FILE--
<?php

foreach (['ibase_commit', 'ibase_rollback', 'ibase_commit_ret', 'ibase_rollback_ret'] as $f) {
	foreach ([[], [null]] as $args) {
		var_dump($f(...$args));
	}
}

?>
--EXPECTF--
Warning: ibase_commit(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_commit(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_rollback(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_rollback(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_commit_ret(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_commit_ret(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_rollback_ret(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)

Warning: ibase_rollback_ret(): supplied resource is not a valid Firebird/InterBase link resource in %s on line %d
bool(false)
