--TEST--
IBASE_UNIXTIME for TIMESTAMP WITH TIME ZONE past 2038 gives the stored moment on 64-bit, also in legacy mode
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
if (PHP_INT_SIZE < 8) die("skip 64-bit only: a 32-bit PHP integer cannot hold this epoch");
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    date_default_timezone_set('Europe/Riga');

    ibase_query("CREATE TABLE TTEST (TZ TIMESTAMP WITH TIME ZONE)");
    ibase_commit();

    ibase_query("INSERT INTO TTEST VALUES (TIMESTAMP '2500-07-01 12:00:00 America/New_York')");
    ibase_commit();

    echo "-- non-legacy\n";
    ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);
    $v = ibase_fetch_row(ibase_query("SELECT TZ FROM TTEST"), IBASE_UNIXTIME)[0];
    $obj = ibase_fetch_row(ibase_query("SELECT TZ FROM TTEST"), IBASE_PHPDATE)[0];
    echo $v, $v === $obj->getTimestamp() ? '' : ' MISMATCH ' . $obj->getTimestamp(), "\n";

    echo "-- legacy\n";
    ini_set('ibase.legacy_flags', IBASE_LEGACY_UNIXTIME);
    echo ibase_fetch_row(ibase_query("SELECT TZ FROM TTEST"), IBASE_UNIXTIME)[0], "\n";
})();

?>
--EXPECT--
-- non-legacy
16740921600
-- legacy
16740921600
