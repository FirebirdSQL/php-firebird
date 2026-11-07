--TEST--
ibase_field_info(): fields introduced in FB 4.0, with older client
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbapi_lt(40);
skip_if_fbserver_lt(40);
skip_if_fbclient_gte(40);
?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");
ibase_connect($test_base);
test_fields40();

?>
--EXPECT--
ID/INTEGER/4
NUMERIC_4/VARCHAR/188
DECIMAL_4/VARCHAR/188
DECFLOAT_16/VARCHAR/92
DECFLOAT_34/VARCHAR/168
INT128_FIELD/VARCHAR/188
TIME_TZ/TIME/4
TIMESTAMP_TZ/TIMESTAMP/8
