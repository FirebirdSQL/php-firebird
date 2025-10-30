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
