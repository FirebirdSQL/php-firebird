--TEST--
Procedures
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("interbase.inc");

(function(){
	ibase_query(
	"CREATE OR ALTER PROCEDURE GET_5_RECORDS(ARG INTEGER)
	RETURNS (N INTEGER, RESULT INTEGER)
	AS
	DECLARE VARIABLE I INTEGER;
	BEGIN
		I = 1;
		WHILE (:I <= 5) DO BEGIN
			N = :I;
			RESULT = :ARG + :I;
			I = :I + 1;
			SUSPEND;
		END
	END");

	$query = ibase_prepare("EXECUTE PROCEDURE GET_5_RECORDS(?)");
	dump_rows(ibase_execute($query, 1));
	dump_rows(ibase_execute($query, 10));

	print "------------------\n";

	$query = ibase_prepare("SELECT * FROM GET_5_RECORDS(?)");
	dump_rows(ibase_execute($query, 1));
	dump_rows(ibase_execute($query, 10));
})();

?>
--EXPECT--
array(2) {
  ["N"]=>
  int(1)
  ["RESULT"]=>
  int(2)
}
array(2) {
  ["N"]=>
  int(1)
  ["RESULT"]=>
  int(11)
}
------------------
array(2) {
  ["N"]=>
  int(1)
  ["RESULT"]=>
  int(2)
}
array(2) {
  ["N"]=>
  int(2)
  ["RESULT"]=>
  int(3)
}
array(2) {
  ["N"]=>
  int(3)
  ["RESULT"]=>
  int(4)
}
array(2) {
  ["N"]=>
  int(4)
  ["RESULT"]=>
  int(5)
}
array(2) {
  ["N"]=>
  int(5)
  ["RESULT"]=>
  int(6)
}
array(2) {
  ["N"]=>
  int(1)
  ["RESULT"]=>
  int(11)
}
array(2) {
  ["N"]=>
  int(2)
  ["RESULT"]=>
  int(12)
}
array(2) {
  ["N"]=>
  int(3)
  ["RESULT"]=>
  int(13)
}
array(2) {
  ["N"]=>
  int(4)
  ["RESULT"]=>
  int(14)
}
array(2) {
  ["N"]=>
  int(5)
  ["RESULT"]=>
  int(15)
}