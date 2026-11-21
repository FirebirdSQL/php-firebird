<?php

function test_use_after_ibase_free_query() {
	$batch_count = 5;

	ibase_query("DELETE FROM TEST1");
	$query = ibase_prepare("INSERT INTO TEST1 (I, C) VALUES (?, ?)");
	for($i = 1; $i <= $batch_count; $i++) {
		ibase_execute($query, $i, "ROW 1 (batch $i)");
		ibase_execute($query, $i, "ROW 2 (batch $i)");
	}
	ibase_free_query($query);

	$query = ibase_prepare("SELECT * FROM TEST1 WHERE I = ?");
	$res = array();
	for($i = 1; $i <= $batch_count; $i++) {
		$res[$i] = ibase_execute($query, $i);
	}
	ibase_free_query($query);

	foreach ($res as $batch=>$r) {
		print "---- Batch $batch ----\n";
		dump_rows($r);
		ibase_free_result($r);
	}
}

function test_ibase_trans_014_015() {
	global $test_base;

	$x = ibase_connect($test_base);
	var_dump(ibase_trans($x));
	var_dump(ibase_trans(1));
	var_dump(ibase_close());
	var_dump(ibase_close($x));
}

function test_time_unixtime(): void {
	ibase_query("CREATE TABLE TTEST (
		ID INTEGER,
		T1 TIME DEFAULT '15:45:59',
		T2 TIMESTAMP DEFAULT '2025-11-06 15:45:59'
	)");
	ibase_commit();
	ibase_query("INSERT INTO TTEST (ID) VALUES (1)");
	dump_table_rows("TTEST");
	dump_table_rows("TTEST", null, IBASE_UNIXTIME);
}

function test_time_tz_unixtime(): void {
	ibase_query("CREATE TABLE TTEST (
		ID INTEGER,
		T1 TIME WITH TIME ZONE DEFAULT '15:45:59 Europe/Riga',
		T2 TIMESTAMP WITH TIME ZONE DEFAULT '2025-11-06 15:45:59 Europe/Riga'
	)");
	ibase_commit();
	ibase_query("INSERT INTO TTEST (ID) VALUES (1)");
	dump_table_rows("TTEST");
	dump_table_rows("TTEST", null, IBASE_UNIXTIME);
}

function test_fields25(): void {
	ibase_query(file_get_contents(__DIR__."/001-FIELDS25.sql"));
	ibase_commit();

	ibase_query("INSERT INTO FIELDS25 (ID) VALUES (1)");
	$q = ibase_query("SELECT * FROM FIELDS25");
	$num_fields = ibase_num_fields($q);
	for($i = 0; $i < $num_fields; $i++){
		$info = ibase_field_info($q, $i);
		printf("%s/%s/%d\n", $info["name"], $info["type"], $info["length"]);
	}
}

function test_fields40(): void {
	ibase_query(file_get_contents(__DIR__."/001-FIELDS40.sql"));
	ibase_commit();

	ibase_query("INSERT INTO FIELDS40 (ID) VALUES (DEFAULT)");
	$q = ibase_query("SELECT * FROM FIELDS40");
	$num_fields = ibase_num_fields($q);
	for($i = 0; $i < $num_fields; $i++){
		$info = ibase_field_info($q, $i);
		printf("%s/%s/%d\n", $info["name"], $info["type"], $info["length"]);
	}
}

function test_field_data40(): void {
	ibase_query(file_get_contents(__DIR__."/001-FIELDS40.sql"));
	ibase_commit();
	ibase_query("INSERT INTO FIELDS40 (ID) VALUES (1)");
	dump_table_rows("FIELDS40");
}

function test_query_affected_rows_single_returning($c): void {
	ibase_query($c, 'CREATE TABLE PROBE (ID INT, NAME VARCHAR(10))');
	ibase_commit($c);

	$t = ibase_trans(IBASE_DEFAULT, $c);
	ibase_query($t, "INSERT INTO PROBE (ID, NAME) VALUES (1, 'a')");

	$statements = [
		"INSERT INTO PROBE (ID, NAME) VALUES (2, 'b') RETURNING ID",
		"UPDATE PROBE SET NAME = 'c' WHERE ID = 2 RETURNING NAME",
		"UPDATE PROBE SET NAME = 'x' WHERE ID = 99 RETURNING NAME",
		"DELETE FROM PROBE WHERE ID = 2 RETURNING ID",
	];
	foreach ($statements as $sql) {
		echo $sql, "\n";
		$r = ibase_query($t, $sql);
		echo "ibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";
		dump_rows($r);
	}

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "SELECT\nibase_query_affected_rows(): ", ibase_query_affected_rows($r), "\n";

	$r = ibase_query($t, "SELECT ID, NAME FROM PROBE ORDER BY ID");
	echo "left:\n";
	dump_rows($r);
	ibase_rollback($t);
}
