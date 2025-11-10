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

function dump_rows($q, int $flags = 0) {
    while($row = ibase_fetch_assoc($q, $flags)){
        var_dump($row);
    }
}

function dump_table_rows($table, $tr = null, int $flags = 0) {
    if($tr)$args[] = $tr;
    $args[] = sprintf('SELECT * FROM "%s"', $table);

    dump_rows(ibase_query(...$args), $flags);
}

function insert_into($table, $data, $tr = null) {
    if($tr) $args[] = $tr;
    [$fields_str, $q_str] = array2sql_parts($data);
    $args[] = sprintf('INSERT INTO "%s" (%s) VALUES (%s)',
        $table, $fields_str, $q_str
    );

    return ibase_query(...array_merge($args, array_values($data)));
}

/** @var float $v */
function skip_if_fb_lt($v) {
    if(($cv = get_fb_version()) < $v)die("skip Firebird server version $cv < $v");
}

/** @var float $v */
function skip_if_fb_gt($v) {
    if(($cv = get_fb_version()) > $v)die("skip Firebird server version $cv > $v");
}

/** @var float $v */
function skip_if_fb_gte($v) {
    if(($cv = get_fb_version()) >= $v)die("skip Firebird server version $cv >= $v");
}

function skip_if_fbclient_lt(float $v): void {
    if(!function_exists("ibase_get_client_version"))die("skip Firebird client library version (undefined) < $v");
    if(($cv = ibase_get_client_version()) < $v)die("skip Firebird client library version $cv < $v");
}

function skip_if_fbclient_gte(float $v): void {
    if(function_exists("ibase_get_client_version")) {
        if(($cv = ibase_get_client_version()) >= $v)die("skip Firebird client library version $cv >= $v");
    }
}

function skip_if_ext_lt(int $v): void {
    if(!defined('IBASE_VER'))die("Skip IBASE_VER (not defined) < $v");
    if(IBASE_VER < $v)die("Skip IBASE_VER ".IBASE_VER." < $v");
}

function skip_if_ext_gte(int $v): void {
    if(defined('IBASE_VER')) {
        if(IBASE_VER >= $v)die("Skip IBASE_VER ".IBASE_VER." >= $v");
    }
}

function skip_if_php_lt(float $v): void {
    $cv = (float)PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;
    if($cv < $v)die("skip PHP version $cv < $v");
}

function skip_if_php_gte(float $v): void {
    $cv = (float)PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;
    if($cv >= $v)die("skip PHP version $cv >= $v");
}

function ibase_query_bulk(array $queries, $tr = null) {
    foreach($queries as $q){
        if(is_array($q)){
            [$sql, $args] = $q;
        } else {
            $sql = $q;
            $args = [];
        }

        if($tr) {
            ibase_query($tr, $sql, ...$args);
        } else {
            ibase_query($sql, ...$args);
        }
    }
}

/** @var Exception $e */
function php_ibase_exception_handler($e) {
   echo "Fatal error: Uncaught ".get_class($e).": ", $e->getMessage(), "\n";
}
