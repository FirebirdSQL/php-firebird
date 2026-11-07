--TEST--
Binding NULL to a NOT NULL column is refused instead of storing whatever was on the heap
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

/** @var string $test_base */
ibase_connect($test_base);

(function() {
    // Firebird only provides a null indicator for a nullable parameter. Without one there
    // is no way to say "no value", so a NULL has to be refused rather than sent as data.
    $types = ['INTEGER', 'CHAR(10)', 'VARCHAR(20)', 'DOUBLE PRECISION', 'NUMERIC(18,4)',
              'TIMESTAMP', 'BLOB SUB_TYPE TEXT'];

    foreach ($types as $n => $type) {
        ibase_query("CREATE TABLE NN$n (V $type NOT NULL)");
        ibase_query("CREATE TABLE NU$n (V $type)");
    }
    ibase_commit();

    foreach ($types as $n => $type) {
        $bad = @ibase_query("INSERT INTO NN$n (V) VALUES (?)", null);
        $err = strpos(ibase_errmsg(), 'cannot bind NULL to a NOT NULL column') !== false;

        $ok = ibase_query("INSERT INTO NU$n (V) VALUES (?)", null);
        ibase_commit();
        $stored = ibase_fetch_row(ibase_query("SELECT V FROM NU$n"));

        printf("%-18s refused=%s errmsg=%s | nullable ok=%s stored=%s\n",
            $type, var_export($bad === false, true), var_export($err, true),
            var_export($ok !== false, true),
            $stored === false ? 'NO ROW' : var_export($stored[0], true));
    }

    // Nothing reached the NOT NULL tables
    foreach ($types as $n => $type) {
        $c = ibase_fetch_row(ibase_query("SELECT COUNT(*) FROM NN$n"))[0];
        if ($c != 0) echo "LEAKED a row into NN$n ($type)\n";
    }
    echo "no rows leaked\n";
})();

?>
--EXPECT--
INTEGER            refused=true errmsg=true | nullable ok=true stored=NULL
CHAR(10)           refused=true errmsg=true | nullable ok=true stored=NULL
VARCHAR(20)        refused=true errmsg=true | nullable ok=true stored=NULL
DOUBLE PRECISION   refused=true errmsg=true | nullable ok=true stored=NULL
NUMERIC(18,4)      refused=true errmsg=true | nullable ok=true stored=NULL
TIMESTAMP          refused=true errmsg=true | nullable ok=true stored=NULL
BLOB SUB_TYPE TEXT refused=true errmsg=true | nullable ok=true stored=NULL
no rows leaked
