--TEST--
Binding TIME and TIMESTAMP WITH TIME ZONE strings with an offset or an abbreviation keeps that offset, not PHP's default zone
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
    // A default zone different from every zone bound below
    date_default_timezone_set('Asia/Tokyo');

    ini_set('ibase.timeformat_tz', 'H:i:s e');
    ini_set('ibase.timestampformat_tz', 'Y-m-d H:i:s e');

    // Firebird's own text of what it received
    $p = ibase_prepare("SELECT CAST(CAST(? AS TIME WITH TIME ZONE) AS VARCHAR(40)),
        CAST(CAST(? AS TIMESTAMP WITH TIME ZONE) AS VARCHAR(60)) FROM RDB\$DATABASE");

    foreach (['Europe/Riga', '+02:00', '-05:30', 'EET', 'EEST', 'EST', 'Z'] as $zone) {
        $row = ibase_fetch_row(ibase_execute($p, "12:00:00 $zone", "2025-01-15 12:00:00 $zone"));
        printf("%-11s %s | %s\n", $zone, ...$row);
    }
})();

?>
--EXPECT--
Europe/Riga 12:00:00.0000 Europe/Riga | 2025-01-15 12:00:00.0000 Europe/Riga
+02:00      12:00:00.0000 +02:00 | 2025-01-15 12:00:00.0000 +02:00
-05:30      12:00:00.0000 -05:30 | 2025-01-15 12:00:00.0000 -05:30
EET         12:00:00.0000 +02:00 | 2025-01-15 12:00:00.0000 +02:00
EEST        12:00:00.0000 +03:00 | 2025-01-15 12:00:00.0000 +03:00
EST         12:00:00.0000 -05:00 | 2025-01-15 12:00:00.0000 -05:00
Z           12:00:00.0000 +00:00 | 2025-01-15 12:00:00.0000 +00:00
