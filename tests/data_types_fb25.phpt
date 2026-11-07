--TEST--
DATA: basic fields
--SKIPIF--
<?php
include("skipif.inc");
if (PHP_INT_SIZE < 8) print "skip: only for 64-bit PHP";
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
	ibase_query(file_get_contents(__DIR__."/001-FIELDS25.sql"));
	ibase_commit();

	$str_len = static function(string $value): int {
		return function_exists("mb_strlen") ? mb_strlen($value, "UTF-8") : strlen($value);
	};

	$data = [
		0 => [
			'ID'               => 0,
			'SMALLINT_1'       => 100,
			'INTEGER_FIELD'    => 12345,
			'BIGINT_FIELD'     => 1000000,
			'NUMERIC_1'        => "1.2345",
			'DECIMAL_1'        => "1.2345",
			'NUMERIC_2'        => "1.23456789",
			'DECIMAL_2'        => "1.23456789",
			'NUMERIC_3'        => "1.1234567890123456",
			'DECIMAL_3'        => "1.1234567890123456",
			'FLOAT_FIELD'      => 1.23,
			'DOUBLE_FIELD'     => 1.23456789,
			'CHAR_FIXED'       => 'A',
			'VARCHAR_FIELD'    => 'First row text',
			'CHAR_UTF8'        => 'テA',
			'VARCHAR_UTF8'     => 'Glāžšķūņi',
			'DATE_FIELD'       => '2024-01-15',
			'TIME_FIELD'       => '08:30:00',
			'TIMESTAMP_FIELD'  => '2024-01-15 08:30:00',
			'BINARY_FIXED'     => "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10",
			'VARBINARY_FIELD'  => "\xDE\xAD\xBE\xEF",
			'BLOB_TEXT'        => 'First blob text',
			'BLOB_BINARY'      => "\x01\x02\x03",
		],
		1 => [
			'ID'               => 1,
			'SMALLINT_1'       => -200,
			'INTEGER_FIELD'    => -54321,
			'BIGINT_FIELD'     => -9999999,
			'NUMERIC_1'        => "1.9000",
			'DECIMAL_1'        => "9.9999",
			'NUMERIC_2'        => "9.90000000",
			'DECIMAL_2'        => "9.99999999",
			'NUMERIC_3'        => "9.9999999999999999",
			'DECIMAL_3'        => "9.9999999999999999",
			'FLOAT_FIELD'      => 9.99,
			'DOUBLE_FIELD'     => 9.99999999,
			'CHAR_FIXED'       => 'AB',
			'VARCHAR_FIELD'    => 'Second row text',
			'CHAR_UTF8'        => 'テAB',
			'VARCHAR_UTF8'     => 'Rīga',
			'DATE_FIELD'       => '2024-02-20',
			'TIME_FIELD'       => '14:45:30',
			'TIMESTAMP_FIELD'  => '2024-02-20 14:45:30',
			'BINARY_FIXED'     => "\xAA\xBB\xCC\xDD\xEE\xFF\x00\x11\x22\x33\x44\x55\x66\x77\x88\x99",
			'VARBINARY_FIELD'  => "\xCA\xFE",
			'BLOB_TEXT'        => 'Second blob text',
			'BLOB_BINARY'      => "\xAA\xBB",
		],
		2 => [
			'ID'               => 2,
			'SMALLINT_1'       => 0,
			'INTEGER_FIELD'    => 0,
			'BIGINT_FIELD'     => 0,
			'NUMERIC_1'        => "0.0000",
			'DECIMAL_1'        => "0.0000",
			'NUMERIC_2'        => "0.00000000",
			'DECIMAL_2'        => "0.00000000",
			'NUMERIC_3'        => "0.0000000000000000",
			'DECIMAL_3'        => "0.0",
			'FLOAT_FIELD'      => 0.0,
			'DOUBLE_FIELD'     => 0.0,
			'CHAR_FIXED'       => 'ABC',
			'VARCHAR_FIELD'    => 'All zeros row',
			'CHAR_UTF8'        => 'テABC',
			'VARCHAR_UTF8'     => 'Liepāja',
			'DATE_FIELD'       => '2024-03-01',
			'TIME_FIELD'       => '00:00:00',
			'TIMESTAMP_FIELD'  => '2024-03-01 00:00:00',
			'BINARY_FIXED'     => "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
			'VARBINARY_FIELD'  => "\x00\x00",
			'BLOB_TEXT'        => 'Third blob text',
			'BLOB_BINARY'      => "\x00",
		],
		3 => [
			'ID'               => 3,
			'SMALLINT_1'       => 32767,
			'INTEGER_FIELD'    => 2147483647,
			'BIGINT_FIELD'     => 9223372036854775807,
			'NUMERIC_1'        => "1.1",
			'DECIMAL_1'        => "9.9990",
			'NUMERIC_2'        => "9.99999990",
			'DECIMAL_2'        => "9.99999990",
			'NUMERIC_3'        => "9.999999999999990",
			'DECIMAL_3'        => "9.999999999999990",
			'FLOAT_FIELD'      => 3.14,
			'DOUBLE_FIELD'     => 3.14159265,
			'CHAR_FIXED'       => 'ABCD',
			'VARCHAR_FIELD'    => 'Max int values row',
			'CHAR_UTF8'        => 'テABCD',
			'VARCHAR_UTF8'     => 'Ventspils',
			'DATE_FIELD'       => '2024-04-10',
			'TIME_FIELD'       => '06:15:45',
			'TIMESTAMP_FIELD'  => '2024-04-10 06:15:45',
			'BINARY_FIXED'     => "\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF",
			'VARBINARY_FIELD'  => "\xFF\xFE\xFD",
			'BLOB_TEXT'        => 'Fourth blob text',
			'BLOB_BINARY'      => "\xFF\xFE",
		],
		4 => [
			'ID'               => 4,
			'SMALLINT_1'       => -32768,
			'INTEGER_FIELD'    => -2147483648,
			'BIGINT_FIELD'     => "-9223372036854775808",
			'NUMERIC_1'        => "-1.3",
			'DECIMAL_1'        => null,
			'NUMERIC_2'        => null,
			'DECIMAL_2'        => null,
			'NUMERIC_3'        => null,
			'DECIMAL_3'        => null,
			'FLOAT_FIELD'      => -3.14,
			'DOUBLE_FIELD'     => -3.14159265,
			'CHAR_FIXED'       => 'ABCDE',
			'VARCHAR_FIELD'    => 'Min int values row',
			'CHAR_UTF8'        => 'テABCDE',
			'VARCHAR_UTF8'     => 'Jelgava',
			'DATE_FIELD'       => '2024-05-22',
			'TIME_FIELD'       => '18:00:00',
			'TIMESTAMP_FIELD'  => '2024-05-22 18:00:00',
			'BINARY_FIXED'     => "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F",
			'VARBINARY_FIELD'  => "\x80\x81",
			'BLOB_TEXT'        => 'Fifth blob text',
			'BLOB_BINARY'      => "\x80",
		],
		5 => [
			'ID'               => 5,
			'SMALLINT_1'       => 512,
			'INTEGER_FIELD'    => 100000,
			'BIGINT_FIELD'     => 123456789,
			'NUMERIC_1'        => "3.1415",
			'DECIMAL_1'        => "3.1415",
			'NUMERIC_2'        => null,
			'DECIMAL_2'        => "3.14159265",
			'NUMERIC_3'        => "3.141592653589793",
			'DECIMAL_3'        => "3.141592653589793",
			'FLOAT_FIELD'      => 3.14,
			'DOUBLE_FIELD'     => 3.14159265,
			'CHAR_FIXED'       => 'ABCDEF',
			'VARCHAR_FIELD'    => 'Pi values row',
			'CHAR_UTF8'        => 'テABCDEF',
			'VARCHAR_UTF8'     => 'Valmiera',
			'DATE_FIELD'       => '2024-06-05',
			'TIME_FIELD'       => '03:14:15',
			'TIMESTAMP_FIELD'  => '2024-06-05 03:14:15',
			'BINARY_FIXED'     => "\x3E\x3D\x3C\x3B\x3A\x39\x38\x37\x36\x35\x34\x33\x32\x31\x30\x2F",
			'VARBINARY_FIELD'  => "\x3E\x3D\x3C",
			'BLOB_TEXT'        => 'Sixth blob text',
			'BLOB_BINARY'      => "\x3E\x3D",
		],
		6 => [
			'ID'               => 6,
			'SMALLINT_1'       => -1024,
			'INTEGER_FIELD'    => -777777,
			'BIGINT_FIELD'     => -987654321,
			'NUMERIC_1'        => "2.7182",
			'DECIMAL_1'        => "-0",
			'NUMERIC_2'        => "2.71828182",
			'DECIMAL_2'        => "2.71828182",
			'NUMERIC_3'        => "2.718281828459045",
			'DECIMAL_3'        => "2.718281828459045",
			'FLOAT_FIELD'      => 2.72,
			'DOUBLE_FIELD'     => 2.71828183,
			'CHAR_FIXED'       => 'ABCDEFG',
			'VARCHAR_FIELD'    => 'Euler number row',
			'CHAR_UTF8'        => 'テABCDEFG',
			'VARCHAR_UTF8'     => 'Cēsis',
			'DATE_FIELD'       => '2024-07-19',
			'TIME_FIELD'       => '11:11:11',
			'TIMESTAMP_FIELD'  => '2024-07-19 11:11:11',
			'BINARY_FIXED'     => "\xAB\xCD\xEF\x01\x23\x45\x67\x89\xAB\xCD\xEF\x01\x23\x45\x67\x89",
			'VARBINARY_FIELD'  => "\xAB\xCD",
			'BLOB_TEXT'        => 'Seventh blob text',
			'BLOB_BINARY'      => "\xAB",
		],
		7 => [
			'ID'               => 7,
			'SMALLINT_1'       => 8000,
			'INTEGER_FIELD'    => 8888888,
			'BIGINT_FIELD'     => 8000000000000,
			'NUMERIC_1'        => "1.6180",
			'DECIMAL_1'        => "1.6180",
			'NUMERIC_2'        => "1.61803398",
			'DECIMAL_2'        => "1.61803398",
			'NUMERIC_3'        => "1.618033988749895",
			'DECIMAL_3'        => null,
			'FLOAT_FIELD'      => 1.62,
			'DOUBLE_FIELD'     => 1.61803399,
			'CHAR_FIXED'       => 'ABCDEFGH',
			'VARCHAR_FIELD'    => 'Golden ratio row',
			'CHAR_UTF8'        => 'テABCDEFGH',
			'VARCHAR_UTF8'     => 'Sigulda',
			'DATE_FIELD'       => '2024-08-08',
			'TIME_FIELD'       => '16:20:00',
			'TIMESTAMP_FIELD'  => '2024-08-08 16:20:00',
			'BINARY_FIXED'     => "\x08\x18\x28\x38\x48\x58\x68\x78\x88\x98\xA8\xB8\xC8\xD8\xE8\xF8",
			'VARBINARY_FIELD'  => "\x08\x88",
			'BLOB_TEXT'        => 'Eighth blob text',
			'BLOB_BINARY'      => "\x08\x80",
		],
		8 => [
			'ID'               => 8,
			'SMALLINT_1'       => -9999,
			'INTEGER_FIELD'    => -1,
			'BIGINT_FIELD'     => -42,
			'NUMERIC_1'        => "0.0001",
			'DECIMAL_1'        => "0.0001",
			'NUMERIC_2'        => "0.00000001",
			'DECIMAL_2'        => "0.00000001",
			'NUMERIC_3'        => "0.000000000000001",
			'DECIMAL_3'        => "0.000000000000001",
			'FLOAT_FIELD'      => 0.01,
			'DOUBLE_FIELD'     => 0.000000001,
			'CHAR_FIXED'       => 'ABCDEFGHI',
			'VARCHAR_FIELD'    => 'Small values row',
			'CHAR_UTF8'        => 'テABCDEFGHI',
			'VARCHAR_UTF8'     => 'Tukums',
			'DATE_FIELD'       => '2024-09-30',
			'TIME_FIELD'       => '23:59:59',
			'TIMESTAMP_FIELD'  => '2024-09-30 23:59:59',
			'BINARY_FIXED'     => "\x09\x19\x29\x39\x49\x59\x69\x79\x89\x99\xA9\xB9\xC9\xD9\xE9\xF9",
			'VARBINARY_FIELD'  => "\x09\x99",
			'BLOB_TEXT'        => 'Ninth blob text',
			'BLOB_BINARY'      => "\x09",
		],
		9 => [
			'ID'               => 9,
			'SMALLINT_1'       => 1234,
			'INTEGER_FIELD'    => 9999999,
			'BIGINT_FIELD'     => 99999999999999,
			'NUMERIC_1'        => "1.777",
			'DECIMAL_1'        => "7.7777",
			'NUMERIC_2'        => "7.77777777",
			'DECIMAL_2'        => "7.77777777",
			'NUMERIC_3'        => "7.77777777777777",
			'DECIMAL_3'        => "7.77777777777777",
			'FLOAT_FIELD'      => 7.77,
			'DOUBLE_FIELD'     => 7.77777777,
			'CHAR_FIXED'       => 'ABCDEFGHIJ',
			'VARCHAR_FIELD'    => null,
			'CHAR_UTF8'        => 'テABCDEFGHJ',
			'VARCHAR_UTF8'     => 'Ogre',
			'DATE_FIELD'       => '2024-10-31',
			'TIME_FIELD'       => '20:30:10',
			'TIMESTAMP_FIELD'  => '2024-10-31 20:30:10',
			'BINARY_FIXED'     => "\x0A\x1A\x2A\x3A\x4A\x5A\x6A\x7A\x8A\x9A\xAA\xBA\xCA\xDA\xEA\xFA",
			'VARBINARY_FIELD'  => "\x0A\xAA",
			'BLOB_TEXT'        => 'Tenth blob text',
			'BLOB_BINARY'      => "\x0A\xFA",
		],
	];

	$sql =
	"INSERT INTO FIELDS25 (
		ID, SMALLINT_1, INTEGER_FIELD, BIGINT_FIELD,
		NUMERIC_1, DECIMAL_1, NUMERIC_2, DECIMAL_2, NUMERIC_3, DECIMAL_3,
		FLOAT_FIELD, DOUBLE_FIELD,
		CHAR_FIXED, VARCHAR_FIELD, CHAR_UTF8, VARCHAR_UTF8,
		DATE_FIELD, TIME_FIELD, TIMESTAMP_FIELD,
		BINARY_FIXED, VARBINARY_FIELD,
		BLOB_TEXT, BLOB_BINARY
	) VALUES (
		?, ?, ?, ?,
		?, ?, ?, ?, ?, ?,
		?, ?,
		?, ?, ?, ?,
		?, ?, ?,
		?, ?,
		?, ?
	)";

	foreach ($data as $id => $item) {
		$item["ID"] = $id;
		if (false === ibase_query($sql, ...array_values($item))) {
			print "$id insert failed\n";
			print_r($item);
		}
	}

	foreach ($data as $id => $item) {
		if (false === ($q = ibase_query("SELECT * FROM FIELDS25 WHERE ID = ?", $id))) {
			print "$id select failed\n";
			print_r($item);
			continue;
		}

		if (false === ($row = ibase_fetch_assoc($q, IBASE_FETCH_BLOBS))) {
			print "$id fetch failed\n";
			continue;
		}

		foreach ($item as $k => $v) {
			if (!array_key_exists($k, $row)) {
				print_r($row);
				die("$id missing column: $k");
			}

			if ($row[$k] != $v) {
				if (strpos($k, "CHAR_") === 0) {
					$pad = str_repeat(" ", $str_len($row[$k]) - $str_len($v));
					if ($row[$k] === "$v$pad") {
						continue;
					}
				}

				if (strpos($k, "FLOAT_") === 0) {
					if(abs($row[$k] - $v) <= abs($v / 1E7)){
						continue;
					}
				}


				print "$id column differs: $k\n";
				print "data="; var_dump($v);
				print "received from database="; var_dump($row[$k]);
			}

			// if ($row[$k] !== $v) {
			// 	if (strpos($k, "CHAR_") === 0) {
			// 		$pad = str_repeat(" ", mb_strlen($row[$k]) - mb_strlen($v));
			// 		if ($row[$k] == "$v$pad") {
			// 			continue;
			// 		}
			// 	}

			// 	print "$id column differs (strict): $k\n";
			// 	print "data="; var_dump($v);
			// 	print "received from database="; var_dump($row[$k]);
			// }
		}
	}

	// defaults
	$id = 10;
	if (false === ibase_query("INSERT INTO FIELDS25 (ID) VALUES (?)", $id)) {
		print "$id insert failed (defaults)\n";
	}

	if (false === ($q = ibase_query("SELECT * FROM FIELDS25 WHERE ID = ?", $id))) {
		print "$id select failed\n";
		die;
	}

	if (false === ($row = ibase_fetch_assoc($q, IBASE_FETCH_BLOBS))) {
		print "$id fetch failed\n";
		die;
	}

	$row['BINARY_FIXED'] = "x'" . strtoupper(bin2hex($row['BINARY_FIXED'])) . "'";
	$row['VARBINARY_FIELD'] = "x'" . strtoupper(bin2hex($row['VARBINARY_FIELD'])) . "'";
	var_dump($row);
})();

?>
--EXPECTF--
array(23) {
  ["ID"]=>
  int(10)
  ["SMALLINT_1"]=>
  int(-32768)
  ["INTEGER_FIELD"]=>
  int(-2147483648)
  ["BIGINT_FIELD"]=>
  int(-9223372036854775808)
  ["NUMERIC_1"]=>
  string(6) "3.1415"
  ["DECIMAL_1"]=>
  string(6) "3.1415"
  ["NUMERIC_2"]=>
  string(10) "3.14159265"
  ["DECIMAL_2"]=>
  string(10) "3.14159265"
  ["NUMERIC_3"]=>
  string(18) "3.1415926535897930"
  ["DECIMAL_3"]=>
  string(18) "3.1415926535897930"
  ["FLOAT_FIELD"]=>
  float(3.1400001049%d)
  ["DOUBLE_FIELD"]=>
  float(3.14)
  ["CHAR_FIXED"]=>
  string(10) "ABCDE     "
  ["VARCHAR_FIELD"]=>
  string(20) "Default varchar text"
  ["CHAR_UTF8"]=>
  string(16) "テスト       "
  ["VARCHAR_UTF8"]=>
  string(26) "Glāžšķūņu rūķīši"
  ["DATE_FIELD"]=>
  string(10) "2025-11-10"
  ["TIME_FIELD"]=>
  string(8) "15:16:59"
  ["TIMESTAMP_FIELD"]=>
  string(19) "2025-11-10 15:16:59"
  ["BINARY_FIXED"]=>
  string(35) "x'000102030405060708090A0B0C0D0E0F'"
  ["VARBINARY_FIELD"]=>
  string(11) "x'DEADBEEF'"
  ["BLOB_TEXT"]=>
  string(20) "Hello from text blob"
  ["BLOB_BINARY"]=>
  string(22) "Hello from binary blob"
}