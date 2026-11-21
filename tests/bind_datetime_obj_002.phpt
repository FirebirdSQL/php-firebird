--TEST--
Binding DateTime and DateTimeImmutable objects to TIME and TIMESTAMP WITH TIME ZONE keeps the object's zone
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

    // Firebird's own text of what it received
    $p = ibase_prepare("SELECT CAST(CAST(? AS TIME WITH TIME ZONE) AS VARCHAR(40)),
        CAST(CAST(? AS TIMESTAMP WITH TIME ZONE) AS VARCHAR(60)) FROM RDB\$DATABASE");

    foreach (['Europe/Riga', '+02:00', '-05:30', 'EET', 'EEST', 'EST', 'Z'] as $zone) {
        $tz = new DateTimeZone($zone);
        $d = new DateTime('2025-01-15 12:00:00.5', $tz);
        $i = new DateTimeImmutable('2025-07-15 12:00:00', $tz);
        printf("%-11s %s | %s\n", $zone, ...ibase_fetch_row(ibase_execute($p, $d, $d)));
        printf("%-11s %s | %s\n", $zone, ...ibase_fetch_row(ibase_execute($p, $i, $i)));
    }
})();

?>
--EXPECT--
Europe/Riga 12:00:00.5000 Europe/Riga | 2025-01-15 12:00:00.5000 Europe/Riga
Europe/Riga 12:00:00.0000 Europe/Riga | 2025-07-15 12:00:00.0000 Europe/Riga
+02:00      12:00:00.5000 +02:00 | 2025-01-15 12:00:00.5000 +02:00
+02:00      12:00:00.0000 +02:00 | 2025-07-15 12:00:00.0000 +02:00
-05:30      12:00:00.5000 -05:30 | 2025-01-15 12:00:00.5000 -05:30
-05:30      12:00:00.0000 -05:30 | 2025-07-15 12:00:00.0000 -05:30
EET         12:00:00.5000 +02:00 | 2025-01-15 12:00:00.5000 +02:00
EET         12:00:00.0000 +02:00 | 2025-07-15 12:00:00.0000 +02:00
EEST        12:00:00.5000 +03:00 | 2025-01-15 12:00:00.5000 +03:00
EEST        12:00:00.0000 +03:00 | 2025-07-15 12:00:00.0000 +03:00
EST         12:00:00.5000 -05:00 | 2025-01-15 12:00:00.5000 -05:00
EST         12:00:00.0000 -05:00 | 2025-07-15 12:00:00.0000 -05:00
Z           12:00:00.5000 +00:00 | 2025-01-15 12:00:00.5000 +00:00
Z           12:00:00.0000 +00:00 | 2025-07-15 12:00:00.0000 +00:00
