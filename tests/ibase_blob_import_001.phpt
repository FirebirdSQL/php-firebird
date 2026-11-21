--TEST--
ibase_blob_import(): a read error from a write-only file stops the import
--SKIPIF--
<?php include(__DIR__."/skipif.inc"); ?>
--FILE--
<?php

require(__DIR__."/interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

$name = tempnam(sys_get_temp_dir(), "ibase_blob_import_001");
$f = fopen($name, "w");
$id = ibase_blob_import($f);
fclose($f);
unlink($name);

var_dump(is_string($id));
var_dump(ibase_blob_info($id)['length']);

?>
--EXPECTF--
Notice: ibase_blob_import(): %cead of %d bytes failed with errno=%d %s in %s on line %d
bool(true)
int(0)
