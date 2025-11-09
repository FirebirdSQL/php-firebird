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
	ini_set("ibase.timeformat", "%H:%M:%S");
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
