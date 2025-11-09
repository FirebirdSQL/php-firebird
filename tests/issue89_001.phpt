--TEST--
Issue #89: Passing result from ibase_prepare() to ibase_fetch_*() causes segfault.
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function() {
    $res = ibase_prepare('SELECT * FROM TEST1');
    var_dump(ibase_fetch_assoc($res));
})();

?>
--EXPECT--
bool(false)
