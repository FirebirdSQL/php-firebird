--TEST--
INF/NAN bound to FLOAT or DOUBLE PRECISION are rejected, not stored
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");
require("common.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    ibase_query("CREATE TABLE TTEST (D DOUBLE PRECISION, F FLOAT)");
    ibase_commit();

    // Firebird raises an arithmetic exception for any operation that would produce one of
    // these, so a column must never hold one. Binding the raw IEEE754 bits would get past
    // the engine's checks entirely, so they go over as a string for Firebird to refuse.
    foreach (['INF' => INF, '-INF' => -INF, 'NAN' => NAN] as $label => $v) {
        foreach (['D', 'F'] as $col) {
            $ok = @ibase_query("INSERT INTO TTEST ($col) VALUES (?)", $v);
            printf("%-5s -> %-2s  %s\n", $label, $col, $ok ? 'ACCEPTED' : 'rejected');
            $ok ? ibase_commit() : ibase_rollback();
        }
    }

    // An ordinary value on the same path still works
    var_dump(ibase_query("INSERT INTO TTEST (D) VALUES (?)", 1.5) !== false);
    ibase_commit();

    dump_table_rows("TTEST");
})();

?>
--EXPECT--
INF   -> D   rejected
INF   -> F   rejected
-INF  -> D   rejected
-INF  -> F   rejected
NAN   -> D   rejected
NAN   -> F   rejected
bool(true)
array(2) {
  ["D"]=>
  float(1.5)
  ["F"]=>
  NULL
}
