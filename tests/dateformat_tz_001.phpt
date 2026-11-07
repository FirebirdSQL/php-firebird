--TEST--
ibase.timeformat_tz / timestampformat_tz round-trip TIME and TIMESTAMP WITH TIME ZONE
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fb_lt(4);
skip_if_fbclient_lt(4);
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE TTEST (T TIME WITH TIME ZONE, TS TIMESTAMP WITH TIME ZONE)");
    ibase_commit();

    ini_set('ibase.timeformat_tz', 'H:i:s e');
    ini_set('ibase.timestampformat_tz', 'Y-m-d H:i:s e');

    ibase_query("INSERT INTO TTEST (T, TS) VALUES (?, ?)",
        "13:45:59 Europe/Riga", "2025-11-06 13:45:59 Europe/Riga") or die("insert failed: " . ibase_errmsg());
    ibase_commit();

    print_r(ibase_fetch_assoc(ibase_query("SELECT * FROM TTEST")));

    // A different spelling of the same instants
    ini_set('ibase.timeformat_tz', 'H.i.s P');
    ini_set('ibase.timestampformat_tz', 'd/m/Y H.i.s P');
    print_r(ibase_fetch_assoc(ibase_query("SELECT * FROM TTEST")));
})();

?>
--EXPECT--
Array
(
    [T] => 13:45:59 Europe/Riga
    [TS] => 2025-11-06 13:45:59 Europe/Riga
)
Array
(
    [T] => 13.45.59 +02:00
    [TS] => 06/11/2025 13.45.59 +02:00
)
