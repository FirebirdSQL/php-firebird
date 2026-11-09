--TEST--
ibase_affected_rows(): no usable connection must not crash
--SKIPIF--
<?php if (!extension_loaded("interbase")) print "skip interbase extension not available"; ?>
--FILE--
<?php

/*
 * ibase_affected_rows() reached ib_link->tr_list without checking that the link
 * was actually fetched, so every case below segfaulted. The most reachable one
 * is the first: a connect fails, the caller does not notice, and then asks how
 * many rows were affected.
 *
 * None of this needs a server, so there is deliberately no connect check in
 * --SKIPIF--. The connect here is expected to fail, and fails fast whether or
 * not anything is listening.
 */

function attempt(string $label, callable $fn): void
{
	printf("%-24s ", $label);
	try {
		var_dump(@$fn());
	} catch (\Throwable $e) {
		echo "threw ", get_class($e), "\n";
	}
}

// A connection that was never established.
$link = @ibase_connect('127.0.0.1/3050:/nonexistent/no_such_db.fdb', 'nobody', 'nobody');
printf("%-24s ", 'failed connect');
var_dump($link);

// The crash: no default link exists, because the connect above failed.
attempt('no default link', fn() => ibase_affected_rows());

// The caller passes the failed connect's return value straight through.
attempt('false as link', fn() => ibase_affected_rows($link));

// A live resource, but of the wrong type.
$stream = fopen('php://memory', 'r');
attempt('wrong resource type', fn() => ibase_affected_rows($stream));
fclose($stream);
attempt('closed resource', fn() => ibase_affected_rows($stream));

// Not a resource at all.
attempt('null', fn() => ibase_affected_rows(null));
attempt('int', fn() => ibase_affected_rows(42));

echo "survived\n";

?>
--EXPECTF--
failed connect           bool(false)
no default link          %s
false as link            %s
wrong resource type      %s
closed resource          %s
null                     %s
int                      %s
survived
