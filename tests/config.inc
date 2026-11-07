<?php

srand(((int)(float)microtime()*1000000));

$user = 'SYSDBA';
$password = 'masterkey';
$host = getenv('IBASE_TEST_HOST') ?: "127.0.0.1/3050";
ini_set('ibase.default_user',$user);
ini_set('ibase.default_password',$password);
ini_set('date.timezone', "UTC");
