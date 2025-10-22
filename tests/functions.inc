<?php

srand(((int)(float)microtime()*1000000));

function out_table($table_name)
{
	echo "--- $table_name ---\n";
	$res = ibase_query("select * from $table_name");
	while ($r = ibase_fetch_row($res)) {
		echo join("\t",$r)."\t\n";
	}
	ibase_free_result($res);
	echo "---\n";
}

function out_result($result, $table_name = "")
{
	echo "--- $table_name ---\n";
	while ($r = ibase_fetch_row($result)) {
		echo join("\t",$r)."\t\n";
	}
	echo "---\n";
}

function out_result_trap_error($result, $table_name = "")
{
   echo "--- $table_name ---\n";
   while ($r = @ibase_fetch_row($result)) {
		echo join("\t",$r)."\t\n";
   }
   echo "errmsg [" . ibase_errmsg() . "]\n";
   echo "---\n";
}

/* M/D/Y H:M:S */
function rand_datetime()
{
    return sprintf("%02d/%02d/%4d %02d:%02d:%02d",
		rand()%12+1, rand()%28+1, rand()%100+1910,
		rand()%24,   rand()%60,  rand()%60);
}

/* random binary string  */
function rand_binstr($max_len)
{
    $len = rand() % $max_len;
    $s = "";
    while($len--) {
        $s .= sprintf("%c", rand() % 256);
    }
    return $s;
}

function rand_str($max_len)
{
    $len = rand() % $max_len;
    $s = "";
    while ($len--) {
        $s .= sprintf("%c", rand() % 26 + 65);
    }
    return $s;
}

function rand_number($len , $prec = -1, $sign = 1)
{
    if ($prec == -1) {
        $n = substr(rand() . rand(), 0, rand() % $len + 1);
        if (strlen($n) < $len) {
	    	$n .= "." . substr(rand(), 0, rand() % ($len - strlen($n)) + 1);
        }
    } else if ($prec == 0) {
        $n = substr(rand() . rand(), 0, rand() % $len + 1);
    } else if (($prec - $len) == 0) {
        $n = substr(rand() . rand(), 0, 1);
        $n .= "." . substr(rand(), 0, $prec);
    } else {
        $n = substr(rand() . rand(), 0, rand() % ($len - $prec) + 1);
        $n .= "." . substr(rand(), 0, $prec);
    }
    if ($sign && (rand() % 3 == 0)) {
        $n = "-" .$n;
    }
    return $n;
}

function fields2sql_parts($fields) {
    $fields_str = '"'.join('","', $fields).'"';
    $q_str = join(",", array_fill(0, count($fields), "?"));
    return [$fields_str, $q_str];
}

function array2sql_parts($data) {
    return [...fields2sql_parts(array_keys($data)), $data];
}

function dump_rows($q) {
    while($row = ibase_fetch_assoc($q)){
        var_dump($row);
    }
}

function dump_table_rows($table) {
    dump_rows(ibase_query(sprintf('SELECT * FROM "%s"', $table)));
}

function insert_into($table, $data) {
    [$fields_str, $q_str] = array2sql_parts($data);
    return ibase_query(sprintf('INSERT INTO "%s" (%s) VALUES (%s)',
        $table, $fields_str, $q_str
    ), ...array_values($data));
}

/** @var float $v */
function skip_if_fb_lt($v) {
    if(($cv = get_fb_version()) < $v)die("skip: Firebird version $cv < $v");
}

/** @var float $v */
function skip_if_fb_gt($v) {
    if(($cv = get_fb_version()) > $v)die("skip: Firebird version $cv > $v");
}

/** @var float $v */
function skip_if_fb_gte($v) {
    if(($cv = get_fb_version()) >= $v)die("skip: Firebird version $cv >= $v");
}
