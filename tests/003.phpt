--TEST--
InterBase: misc sql types (may take a while)
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

    require("interbase.inc");
    /** @var string $test_base */
    ibase_connect($test_base);

    /* To prevent unwanted roundings set PHP precision to 18 */
    ini_set('precision',"18");

    /* Check if PHP precision is set correctly */
    if(ini_get('precision') < 18) {
        echo "PHP precision check fail\n";
        echo "Precision set in php.ini: " . ini_get('precision') . "\n";
        echo "Precision required: 18\n";
    }

    ibase_query(
    	"create table test3 (
            iter		integer not null,
            v_char		char(1000),
            v_date      timestamp,
   	 		v_decimal4_2  		decimal(4,2),
   	 		v_decimal4_0  		decimal(4,0),
   	 		v_decimal7_2  		decimal(7,2),
   	 		v_decimal7_0  		decimal(7,0),
   	 		v_numeric15_15  	numeric(15,15),
                        v_decimal18_3           decimal(18,3),
   	 		v_numeric15_0  	numeric(15,0),
            v_double  	double precision,
            v_float     float,
            v_integer   integer,
            v_smallint  smallint,
            v_varchar   varchar(10000)
            )");
    ibase_commit();

	/* should fail, but gracefully */
	@ibase_query("insert into test3 (iter) values (?)", null);

    ini_set("ibase.timestampformat", "m/d/Y H:i:s");

    /* Values the old rand_*() helpers produced; char/varchar hold that many letters A-Z */
    $rows = [
        /* char, date,                 dec4_2,   dec4_0,  dec7_2,     dec7_0,    num15_15,        dec18_3,              num15_0,          double,                  float,       integer,     smallint, varchar */
        [22,  '08/17/1930 18:44:34', '-7.43',  '1788',  '18683.10', '-171994', '1.1547546835',  '0',                 '0',              '1249877.2028661055',    '22743.56',  '15825197',  -29,    1540],
        [316, '01/22/1916 22:35:50', '-6.15',  '37',    '-108.16',  '100313',  '1.1697504841',  '-110593885119.172', '-1554652312152', '198062.775100',         '-1790.8',   '2',         4,      8408],
        [638, '02/16/1935 13:33:04', '1.18',   '1',     '89328.10', '135747',  '1.1449718840',  '1323540189.854',    '72940138',       '1447236.170025516',     '-1079596',  '-5',        -10596, 4421],
        [959, '04/04/1950 16:48:05', '4.92',   '1009',  '-7808.46', '6',       '5.1533125736',  '-15821.161',        '-776241126635',  '4879842891931.99960',   '980063.1',  '69399',     -17,    1811],
        [810, '03/12/2001 02:20:41', '24.10',  '4',     '57472.17', '15653',   '2.1386706106',  '10665112054.184',   '266127218476',   '794369742927870057',    '306.1',     '62',        -11751, 3666],
        [433, '12/02/1990 20:36:58', '-22.77', '-5271', '-1.85',    '1958',    '1.1991760874',  '2082916.382',       '-129942328761',  '33730.76328221',        '1489874',   '11142',     -199,   4101],
        [172, '09/16/1964 22:52:41', '13.12',  '12',    '129.12',   '5764',    '-1.1967970807', '-1649955.116',      '89889287710',    '8678.2046912515',       '129071.1',  '437560',    -1495,  6674],
        [561, '11/16/1954 14:53:29', '59.13',  '36',    '17647.88', '1087400', '8.1172477069',  '1082799.197',       '1712241146164',  '-1756321384178.21099',  '-1833.12',  '6838',      -5885,  7097],
        [245, '12/08/1952 13:51:00', '-14.30', '5',     '87.66',    '1024',    '1.1777482894',  '595.206',           '1268',           '-45396268928507598.2',  '1035.738',  '135649088', 1,      6091],
        [175, '06/15/1918 11:46:01', '17.13',  '1310',  '17.10',    '50058',   '1.2010576286',  '-164617811512.800', '16',             '3594168803732.910',     '-96640.69', '889914549', 11,     3420],
    ];

    foreach($rows as $iter => [$char_len, $v_date, $v_decimal4_2, $v_decimal4_0, $v_decimal7_2, $v_decimal7_0,
        $v_numeric15_15, $v_decimal18_3, $v_numeric15_0, $v_double, $v_float, $v_integer, $v_smallint, $varchar_len]){
    	$v_char = str_pad('', $char_len, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ');
    	$v_varchar = str_pad('', $varchar_len, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ');

    	ibase_query(
    	"insert into test3 (iter, v_char,v_date,v_decimal4_2, v_decimal4_0, v_decimal7_2, v_decimal7_0,v_numeric15_15, v_decimal18_3, v_numeric15_0,v_double,v_float,v_integer,v_smallint,v_varchar)
    	values ($iter, '$v_char','$v_date',$v_decimal4_2, $v_decimal4_0, $v_decimal7_2, $v_decimal7_0,$v_numeric15_15, $v_decimal18_3, $v_numeric15_0,$v_double,$v_float,$v_integer,$v_smallint,'$v_varchar')");
    	$sel = ibase_query("select * from test3 where iter = $iter");
    	$row = ibase_fetch_object($sel) or die ("ibase_fetch_object failed");
    	if(substr($row->V_CHAR,0,strlen($v_char)) != $v_char){
        	echo " CHAR fail:\n";
            echo " in:  $v_char\n";
            echo " out: $row->V_CHAR\n";
        }
        if($row->V_DATE != $v_date){
            echo " DATE fail\n";
            echo " in:  $v_date\n";
            echo " out: $row->V_DATE\n";
        }
        if($row->V_DECIMAL4_2 != $v_decimal4_2){
            echo " DECIMAL4_2 fail\n";
            echo " in:  $v_decimal4_2\n";
            echo " out: $row->V_DECIMAL4_2\n";
        }
        if($row->V_DECIMAL4_0 != $v_decimal4_0){
            echo " DECIMAL4_0 fail\n";
            echo " in:  $v_decimal4_0\n";
            echo " out: $row->V_DECIMAL4_0\n";
        }
        if($row->V_DECIMAL7_2 != $v_decimal7_2){
            echo " DECIMAL7_2 fail\n";
            echo " in:  $v_decimal7_2\n";
            echo " out: $row->V_DECIMAL7_2\n";
        }
        if($row->V_DECIMAL7_0 != $v_decimal7_0){
            echo " DECIMAL7_0 fail\n";
            echo " in:  $v_decimal7_0\n";
            echo " out: $row->V_DECIMAL7_0\n";
        }
        if($row->V_NUMERIC15_15 != $v_numeric15_15){
            echo " NUMERIC15_15 fail\n";
            echo " in:  $v_numeric15_15\n";
            echo " out: $row->V_NUMERIC15_15\n";
        }
        if($row->V_DECIMAL18_3 != $v_decimal18_3){
            echo " DECIMAL18_3 fail\n";
            echo " in:  $v_decimal18_3\n";
            echo " out: $row->V_DECIMAL18_3\n";
        }
        if($row->V_NUMERIC15_0 != (string)$v_numeric15_0){
            echo " NUMERIC15_0 fail\n";
            echo " in:  $v_numeric15_0\n";
            echo " out: $row->V_NUMERIC15_0\n";
        }

        if(abs($row->V_DOUBLE - $v_double) > abs($v_double / 1E15)){
            echo " DOUBLE fail\n";
            echo " in:  $v_double\n";
            echo " out: $row->V_DOUBLE\n";
        }
        if(abs($row->V_FLOAT - $v_float) > abs($v_float / 1E7)){
            echo " FLOAT fail\n";
            echo " in:  $v_float\n";
            echo " out: $row->V_FLOAT\n";
        }
        if($row->V_INTEGER != $v_integer){
            echo " INTEGER fail\n";
            echo " in:  $v_integer\n";
            echo " out: $row->V_INTEGER\n";
        }
        if($row->V_SMALLINT != $v_smallint){
            echo " SMALLINT fail\n";
            echo " in:  $v_smallint\n";
            echo " out: $row->V_SMALLINT\n";
        }

	  if(substr($row->V_VARCHAR,0,strlen($v_varchar)) != $v_varchar){
        	echo " VARCHAR fail:\n";
            echo " in:  $v_varchar\n";
            echo " out: $row->V_VARCHAR\n";
        }

        ibase_free_result($sel);
    } /* foreach($rows) */

	/* check for correct handling of duplicate field names */
	$q = ibase_query('SELECT 1 AS id, 2 AS id, 3 AS id, 4 AS id, 5 AS id, 6 AS id, 7 AS id, 8 AS id, 9 AS id,
		10 AS id, 11 AS id, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22 FROM rdb$database');
	var_dump(ibase_fetch_assoc($q));

    ibase_close();
    echo "end of test\n";
?>
--EXPECT--
array(22) {
  ["ID"]=>
  int(1)
  ["ID_01"]=>
  int(2)
  ["ID_02"]=>
  int(3)
  ["ID_03"]=>
  int(4)
  ["ID_04"]=>
  int(5)
  ["ID_05"]=>
  int(6)
  ["ID_06"]=>
  int(7)
  ["ID_07"]=>
  int(8)
  ["ID_08"]=>
  int(9)
  ["ID_09"]=>
  int(10)
  ["ID_10"]=>
  int(11)
  ["CONSTANT"]=>
  int(12)
  ["CONSTANT_01"]=>
  int(13)
  ["CONSTANT_02"]=>
  int(14)
  ["CONSTANT_03"]=>
  int(15)
  ["CONSTANT_04"]=>
  int(16)
  ["CONSTANT_05"]=>
  int(17)
  ["CONSTANT_06"]=>
  int(18)
  ["CONSTANT_07"]=>
  int(19)
  ["CONSTANT_08"]=>
  int(20)
  ["CONSTANT_09"]=>
  int(21)
  ["CONSTANT_10"]=>
  int(22)
}
end of test
