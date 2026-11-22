<?php

$user = 'SYSDBA';
$password = 'masterkey';
$host = getenv('IBASE_TEST_HOST') ?: "127.0.0.1/3050";
ini_set('ibase.default_user',$user);
ini_set('ibase.default_password',$password);
putenv("ISC_USER=$user");
putenv("ISC_PASSWORD=$password");
ini_set('date.timezone', "UTC");
