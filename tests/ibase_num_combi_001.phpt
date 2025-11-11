--TEST--
ibase_num_fields() / ibase_num_params(): combined
--SKIPIF--
<?php
include("skipif.inc");
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query("DELETE FROM TEST1");

    $p = ibase_prepare("INSERT INTO TEST1 (I, C) VALUES (?, ?)");
    var_dump(ibase_num_fields($p));
    var_dump(ibase_num_params($p));

    print "--------\n";

    $p = ibase_prepare("INSERT INTO TEST1 (I, C) VALUES (?, ?) RETURNING I, C");
    var_dump(ibase_num_fields($p));
    var_dump(ibase_num_params($p));
})();

?>
--EXPECT--
int(0)
int(2)
--------
int(2)
int(2)
