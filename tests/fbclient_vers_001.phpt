--TEST--
Get fbclient version
--SKIPIF--
<?php
include("skipif.inc");
skip_if_ext_lt(61);
?>
--FILE--
<?php

// Same scale as FB_API_VER: major * 10 + minor, so 2.5 is 25 and 5.0 is 50.
var_dump(
    ibase_get_client_version() === ibase_get_client_major_version() * 10 + ibase_get_client_minor_version()
);

// IBASE_EFFECTIVE_VER is the lower of what we were built against and what we are running
var_dump(
    IBASE_EFFECTIVE_VER === min(FB_API_VER, ibase_get_client_version())
);

?>
--EXPECT--
bool(true)
bool(true)
