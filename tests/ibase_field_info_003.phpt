--TEST--
ibase_field_info(): fields introduced in FB 4.0
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fb_lt(4);
skip_if_fbclient_lt(4);
?>
--FILE--
<?php

require("interbase.inc");
ibase_connect($test_base);

(function(){
    ibase_query(file_get_contents(__DIR__."/001-FIELDS40.sql"));
    ibase_commit();

    ibase_query("INSERT INTO FIELDS40 (ID) VALUES (DEFAULT)");
    $q = ibase_query("SELECT * FROM FIELDS40");
    $num_fields = ibase_num_fields($q);
    for($i = 0; $i < $num_fields; $i++){
        $info = ibase_field_info($q, $i);
        printf("%s/%s/%d\n", $info["name"], $info["type"], $info["length"]);
    }
})();

?>
--EXPECT--
ID/INTEGER/4
NUMERIC_4/VARCHAR/188
DECIMAL_4/VARCHAR/188
DECFLOAT_16/VARCHAR/92
DECFLOAT_34/VARCHAR/168
INT128_FIELD/VARCHAR/188
TIME_TZ/TIME WITH TIME ZONE/8
TIMESTAMP_TZ/TIMESTAMP WITH TIME ZONE/12
