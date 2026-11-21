--TEST--
ibase_commit()/ibase_rollback(): no default link must not crash
--SKIPIF--
<?php
if (!extension_loaded("interbase")) print "skip interbase extension not available";
include("skipif-php7-or-older.inc");
?>
--FILE--
<?php

foreach (['ibase_commit', 'ibase_rollback', 'ibase_commit_ret', 'ibase_rollback_ret'] as $f) {
	foreach ([[], [null]] as $args) {
		try {
			var_dump($f(...$args));
		} catch (TypeError $e) {
			echo $e->getMessage(), "\n";
		}
	}
}

?>
--EXPECT--
ibase_commit(): supplied resource is not a valid Firebird/InterBase link resource
ibase_commit(): supplied resource is not a valid Firebird/InterBase link resource
ibase_rollback(): supplied resource is not a valid Firebird/InterBase link resource
ibase_rollback(): supplied resource is not a valid Firebird/InterBase link resource
ibase_commit_ret(): supplied resource is not a valid Firebird/InterBase link resource
ibase_commit_ret(): supplied resource is not a valid Firebird/InterBase link resource
ibase_rollback_ret(): supplied resource is not a valid Firebird/InterBase link resource
ibase_rollback_ret(): supplied resource is not a valid Firebird/InterBase link resource
