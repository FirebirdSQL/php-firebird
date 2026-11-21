--TEST--
ibase_blob_import(): a read error from a directory stops the import
--SKIPIF--
<?php
include(__DIR__."/skipif.inc");
if (PHP_OS_FAMILY === 'Windows') die("skip directories cannot be opened as files on Windows");
?>
--FILE--
<?php

require(__DIR__."/interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

$f = fopen(sys_get_temp_dir(), "r");
$id = ibase_blob_import($f);
fclose($f);

var_dump(is_string($id));
var_dump(ibase_blob_info($id)['length']);

?>
--EXPECTF--
Notice: ibase_blob_import(): %cead of %d bytes failed with errno=%d %s in %s on line %d
bool(true)
int(0)
