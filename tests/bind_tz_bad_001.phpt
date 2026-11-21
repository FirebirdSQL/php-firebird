--TEST--
Binding TIME and TIMESTAMP WITH TIME ZONE with an unknown zone fails with Firebird's error, with or without ibase.*format_tz
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE TTEST (T TIME WITH TIME ZONE, TS TIMESTAMP WITH TIME ZONE)");
    ibase_commit();

    $p = ibase_prepare("INSERT INTO TTEST (T, TS) VALUES (?, ?)");

    $try = function (string $t, string $ts) use ($p) {
        echo ibase_execute($p, $t, $ts) === false ? "false" : "ok", "\n";
    };

    // PHP cannot parse the zone, so the string goes to Firebird, which rejects it
    foreach ([['H:i:s e', 'Y-m-d H:i:s e'], ['', '']] as [$tf, $tsf]) {
        echo "-- format '$tsf'\n";
        ini_set('ibase.timeformat_tz', $tf);
        ini_set('ibase.timestampformat_tz', $tsf);
        $try("13:45:59 Mars/Olympus", "2025-11-06 13:45:59 Europe/Riga");
        $try("13:45:59 Europe/Riga", "2025-11-06 13:45:59 Mars/Olympus");
        // the statement is still usable after a failed execute
        $try("13:45:59 Europe/Riga", "2025-11-06 13:45:59 Europe/Riga");
    }
    ibase_commit();

    ini_set('ibase.timeformat_tz', 'H:i:s e');
    ini_set('ibase.timestampformat_tz', 'Y-m-d H:i:s e');
    $q = ibase_query("SELECT T, TS FROM TTEST");
    while ($row = ibase_fetch_row($q)) {
        echo implode(" | ", $row), "\n";
    }
})();

?>
--EXPECTF--
-- format 'Y-m-d H:i:s e'

Warning: ibase_execute(): Dynamic SQL Error SQL error code = -303 Invalid time zone region: Mars/Olympus  in %s on line %d
false

Warning: ibase_execute(): Dynamic SQL Error SQL error code = -303 Invalid time zone region: Mars/Olympus  in %s on line %d
false
ok
-- format ''

Warning: ibase_execute(): Dynamic SQL Error SQL error code = -303 Invalid time zone region: Mars/Olympus  in %s on line %d
false

Warning: ibase_execute(): Dynamic SQL Error SQL error code = -303 Invalid time zone region: Mars/Olympus  in %s on line %d
false
ok
13:45:59 Europe/Riga | 2025-11-06 13:45:59 Europe/Riga
13:45:59 Europe/Riga | 2025-11-06 13:45:59 Europe/Riga
