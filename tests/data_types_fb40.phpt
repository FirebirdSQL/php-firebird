--TEST--
DATA: fields introduced with FB 4.0
--SKIPIF--
<?php
include("skipif.inc");
skip_if_fbeffective_lt(40);
skip_if_fbserver_lt(40);
?>
--FILE--
<?php

require_once('config.inc');
ini_set('ibase.default_charset', "UTF8");

require("interbase.inc");
require("common.inc");

if (function_exists("mb_internal_encoding")) {
	mb_internal_encoding("UTF-8");
}

/** @var string $test_base */
ibase_connect($test_base);

(function() {
	ibase_query(file_get_contents(__DIR__."/001-FIELDS40.sql"));
	ibase_commit();

	$data = [
		0 => [
			'ID'           => 0,
			'NUMERIC_4'    => "3.1415926535897932384626433832795028841",
			'DECIMAL_4'    => "3.1415926535897932384626433832795028841",
			'DECFLOAT_16'  => "3.141592653589793",
			'DECFLOAT_34'  => "3.141592653589793238462643383279502",
			'INT128_FIELD' => "0",
			'TIME_TZ'      => "15:45:59 Europe/Berlin",
			'TIMESTAMP_TZ' => "2025-11-06 15:45:59 Europe/Berlin",
		],
		1 => [
			'ID'           => 1,
			'NUMERIC_4'    => "9.9999999999999999999999999999999999999",
			'DECIMAL_4'    => "9.9999999999999999999999999999999999999",
			'DECFLOAT_16'  => "9.999999999999999",
			'DECFLOAT_34'  => "9.999999999999999999999999999999999",
			'INT128_FIELD' => "170141183460469231731687303715884105727",
			'TIME_TZ'      => "00:00:00 UTC",
			'TIMESTAMP_TZ' => "2025-01-01 00:00:00 UTC",
		],
		2 => [
			'ID'           => 2,
			'NUMERIC_4'    => "-9.9999999999999999999999999999999999999",
			'DECIMAL_4'    => "-9.9999999999999999999999999999999999999",
			'DECFLOAT_16'  => "-9.999999999999999",
			'DECFLOAT_34'  => "-9.999999999999999999999999999999999",
			'INT128_FIELD' => "-170141183460469231731687303715884105728",
			'TIME_TZ'      => "23:59:59 UTC",
			'TIMESTAMP_TZ' => "2024-12-31 23:59:59 UTC",
		],
		3 => [
			'ID'           => 3,
			'NUMERIC_4'    => "0.0000000000000000000000000000000000000",
			'DECIMAL_4'    => "0.0000000000000000000000000000000000000",
			'DECFLOAT_16'  => "0",
			'DECFLOAT_34'  => "0",
			'INT128_FIELD' => "0",
			'TIME_TZ'      => "12:00:00 Asia/Tokyo",
			'TIMESTAMP_TZ' => "2025-06-15 12:00:00 Asia/Tokyo",
		],
		4 => [
			'ID'           => 4,
			'NUMERIC_4'    => null,
			'DECIMAL_4'    => null,
			'DECFLOAT_16'  => null,
			'DECFLOAT_34'  => null,
			'INT128_FIELD' => null,
			'TIME_TZ'      => null,
			'TIMESTAMP_TZ' => null,
		],
		5 => [
			'ID'           => 5,
			'NUMERIC_4'    => "2.7182818284590452353602874713526624977",
			'DECIMAL_4'    => "2.7182818284590452353602874713526624977",
			'DECFLOAT_16'  => "2.718281828459045",
			'DECFLOAT_34'  => "2.718281828459045235360287471352662",
			'INT128_FIELD' => "271828182845904523536028747135266",
			'TIME_TZ'      => "08:30:00 America/New_York",
			'TIMESTAMP_TZ' => "2025-03-15 08:30:00 America/New_York",
		],
		6 => [
			'ID'           => 6,
			'NUMERIC_4'    => "1.6180339887498948482045868343656381177",
			'DECIMAL_4'    => "1.6180339887498948482045868343656381177",
			'DECFLOAT_16'  => "1.618033988749895",
			'DECFLOAT_34'  => "1.618033988749894848204586834365638",
			'INT128_FIELD' => "161803398874989484820458683436563",
			'TIME_TZ'      => "18:00:00 Europe/London",
			'TIMESTAMP_TZ' => "2025-08-01 18:00:00 Europe/London",
		],
		7 => [
			'ID'           => 7,
			'NUMERIC_4'    => "-1.4142135623730950488016887242096980785",
			'DECIMAL_4'    => "-1.4142135623730950488016887242096980785",
			'DECFLOAT_16'  => "-1.414213562373095",
			'DECFLOAT_34'  => "-1.414213562373095048801688724209698",
			'INT128_FIELD' => "-141421356237309504880168872420969",
			'TIME_TZ'      => "06:15:45 America/Los_Angeles",
			'TIMESTAMP_TZ' => "2024-04-10 06:15:45 America/Los_Angeles",
		],
		8 => [
			'ID'           => 8,
			'NUMERIC_4'    => "0.0000000000000000000000000000000000001",
			'DECIMAL_4'    => "0.0000000000000000000000000000000000001",
			'DECFLOAT_16'  => "1.234567890123456",
			'DECFLOAT_34'  => "1.234567890123456789012345678901234",
			'INT128_FIELD' => "99999999999999999999999999999999999999",
			'TIME_TZ'      => "03:14:15 Australia/Sydney",
			'TIMESTAMP_TZ' => "2025-06-28 03:14:15 Australia/Sydney",
		],
		9 => [
			'ID'           => 9,
			'NUMERIC_4'    => "-0.0000000000000000000000000000000000001",
			'DECIMAL_4'    => "-0.0000000000000000000000000000000000001",
			'DECFLOAT_16'  => "-1.234567890123456",
			'DECFLOAT_34'  => "-1.234567890123456789012345678901234",
			'INT128_FIELD' => "-99999999999999999999999999999999999999",
			'TIME_TZ'      => "20:30:10 America/Chicago",
			'TIMESTAMP_TZ' => "2024-10-31 20:30:10 America/Chicago",
		],
	];

	foreach ($data as $id => $item) {
		// $item["ID"] = $id;
		// $time_lit = $item['TIME_TZ'] !== null ? "'" . $item['TIME_TZ'] . "'" : 'NULL';
		// $ts_lit   = $item['TIMESTAMP_TZ'] !== null ? "'" . $item['TIMESTAMP_TZ'] . "'" : 'NULL';
		$sql =
		"INSERT INTO FIELDS40 (
			ID, NUMERIC_4, DECIMAL_4, DECFLOAT_16, DECFLOAT_34, INT128_FIELD, TIME_TZ, TIMESTAMP_TZ
		) VALUES (
			?, ?, ?, ?, ?, ?, ?, ?
		)";

		if (false === ibase_query($sql, ...array_values($item))) {
			print "$id insert failed\n";
			print_r($item);
		}
	}

	foreach ($data as $id => $item) {
		if (false === ($q = ibase_query("SELECT * FROM FIELDS40 WHERE ID = ?", $id))) {
			print "$id select failed\n";
			print_r($item);
			continue;
		}

		if (false === ($row = ibase_fetch_assoc($q))) {
			print "$id fetch failed\n";
			continue;
		}

		foreach ($item as $k => $v) {
			if (!array_key_exists($k, $row)) {
				print_r($row);
				die("$id missing column: $k");
			}

			if ($row[$k] !== $v) {
				print "$id column differs: $k\n";
				print "data="; var_dump($v);
				print "received from database="; var_dump($row[$k]);
			}
		}
	}

	// defaults
	$id = 10;
	if (false === ibase_query("INSERT INTO FIELDS40 (ID) VALUES (?)", $id)) {
		print "$id insert failed (defaults)\n";
	}

	if (false === ($q = ibase_query("SELECT * FROM FIELDS40 WHERE ID = ?", $id))) {
		print "$id select failed\n";
		die;
	}

	if (false === ($row = ibase_fetch_assoc($q))) {
		print "$id fetch failed\n";
		die;
	}

	var_dump($row);
})();

?>
--EXPECT--
array(8) {
  ["ID"]=>
  int(10)
  ["NUMERIC_4"]=>
  string(39) "3.1415926535897932384626433832795028841"
  ["DECIMAL_4"]=>
  string(39) "3.1415926535897932384626433832795028841"
  ["DECFLOAT_16"]=>
  string(17) "3.141592653589793"
  ["DECFLOAT_34"]=>
  string(35) "3.141592653589793238462643383279502"
  ["INT128_FIELD"]=>
  string(40) "-170141183460469231731687303715884105727"
  ["TIME_TZ"]=>
  string(22) "15:45:59 Europe/Berlin"
  ["TIMESTAMP_TZ"]=>
  string(33) "2025-11-06 15:45:59 Europe/Berlin"
}
