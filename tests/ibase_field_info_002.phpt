--TEST--
ibase_field_info(): fields introduced in FB 3.0
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fb_lt(3);
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query(file_get_contents(__DIR__."/001-FIELDS30.sql"));
    ibase_commit();

    ibase_query("INSERT INTO FIELDS30 (ID) VALUES (1)");
    $q = ibase_query("SELECT * FROM FIELDS30");
    $num_fields = ibase_num_fields($q);
    for($i = 0; $i < $num_fields; $i++){
        $info = ibase_field_info($q, $i);
        printf("%s/%s/%d\n", $info["name"], $info["type"], $info["length"]);
    }
})();

?>
--EXPECT--
ID/INTEGER/4
BOOL_FIELD/BOOLEAN/1
