# PHP Firebird extension

## Building the driver

### Build the driver on Linux
First of all, we have to meet some requirements. This means we need to install the `phpize` command. The `phpize` command is used to prepare the build environment for a PHP extension.
Install the `phpize` command. This is usually done by installing the `php7-devel` package using the system's package manager.
For OpenSuse use
```
$ zypper in php7-devel
```

Now make sure you provide the fbclient.so and the header files (ibase.h). These are needed to compile. You can specify the include path for the ibase.h file with CPPFLAGS as you can see in the following listing.
```
$ git clone https://github.com/MartinKoeditz/php_firebird.git php-firebird
$ cd php-firebird
$ phpize
$ CPPFLAGS=-I/usr/include/firebird ./configure
$ make
```
If the configure process passes you will get following message:
```
$ Build complete.
$ Don't forget to run 'make test'.
```
You can find the `interbase.so` file in directory `php-firebird/modules`. Copy the file to your php extension dir and restart your webserver.
