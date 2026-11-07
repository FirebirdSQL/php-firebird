--TEST--
IBASE_LEGACY_UNIXTIMESTAMP: ibase.legacy_flags restores pre-refactor mktime()-based (system timezone) conversion for IBASE_UNIXTIME
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    $ts = 1762436759; // 2025-11-06 13:45:59 UTC

    ibase_query(
        "CREATE TABLE TTEST (
            TS TIMESTAMP
        )"
    );
    ibase_commit();

    ibase_query("INSERT INTO TTEST (TS) VALUES (?)", $ts) or die("ibase_query failed");

    // Pacific/Kiritimati (UTC+14, no DST) makes the legacy offset deterministic regardless
    // of the host's own configured timezone.
    putenv('TZ=Pacific/Kiritimati');

    // IBASE_LEGACY_NONE: timestamp derived via DateTime, respecting date.timezone (forced to
    // UTC by tests/config.inc) rather than the process's system timezone.
    ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);
    $res = ibase_query("SELECT TS FROM TTEST");
    $row = ibase_fetch_assoc($res, IBASE_UNIXTIME);
    var_dump($row['TS'] === $ts);

    // Legacy behaviour: mktime() against the process's system timezone, exactly like
    // versions before the DateTime-based rewrite -- ignoring date.timezone.
    ini_set('ibase.legacy_flags', IBASE_LEGACY_UNIXTIMESTAMP);
    $res = ibase_query("SELECT TS FROM TTEST");
    $row = ibase_fetch_assoc($res, IBASE_UNIXTIME);
    var_dump($row['TS'] === $ts - 14 * 3600);

    // IBASE_LEGACY_ALL is the shipped default and must behave like the flag set above
    ini_set('ibase.legacy_flags', IBASE_LEGACY_ALL);
    $res = ibase_query("SELECT TS FROM TTEST");
    $row = ibase_fetch_assoc($res, IBASE_UNIXTIME);
    var_dump($row['TS'] === $ts - 14 * 3600);

    ini_set('ibase.legacy_flags', IBASE_LEGACY_NONE);
    putenv('TZ');
})();

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
