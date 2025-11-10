--TEST--
Get fbclient version
--SKIPIF--
<?php
include("skipif.inc");
skip_if_ext_lt(61);
?>
--FILE--
<?php

var_dump(
    ibase_get_client_version() === (float)ibase_get_client_major_version() + ibase_get_client_minor_version() / 10
);

?>
--EXPECT--
bool(true)
