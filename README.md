# PHP Firebird extension

## Building the driver

### Build the driver on Linux
First of all, we have to meet some requirements. This means we need to install the `phpize` command. The `phpize` command is used to prepare the build environment for a PHP extension.
Install the `phpize` command. This is usually done by installing the `php7-devel` or `php8-devel` package using the system's package manager. Also install the fbclient library and developer packages.

For OpenSuse 15.1 and PHP 7 use
```
$ zypper in php7-devel libfbclient2 libfbclient-devel
```

The command in Linux Mint 20 / Ubuntu is
```
sudo apt-get install php-dev firebird-dev firebird3.0 firebird3.0-common firebird3.0-server
```

Now make sure you provide the fbclient.so and the header files (ibase.h). These are needed to compile. You can specify the include path for the ibase.h file with CPPFLAGS as you can see in the following listing.
```
$ git clone https://github.com/FirebirdSQL/php-firebird.git
$ cd php-firebird
$ phpize
$ CPPFLAGS=-I/usr/include/firebird ./configure
$ make
```

Note: If you use different PHP versions in parallel don't forget to make the correct settings. Linux Mint 20 / Ubuntu uses this syntax:
```
$ git clone https://github.com/FirebirdSQL/php-firebird.git
$ cd php-firebird
$ phpize7.4
$ CPPFLAGS=-I/usr/include/firebird ./configure --with-php-config=/usr/bin/php-config7.4
$ make
```

If the configure process passes you will get following message:
```
$ Build complete.
$ Don't forget to run 'make test'.
```
You can find the `interbase.so` file in directory `php-firebird/modules`. Copy the file to your php extension dir and restart your webserver.

#### Clean up your working directory
After you've created the binary data, many temporary files will be created in your working directory. These can be removed with the command `phpize --clean`. Then you have a tidy directory again.

### Build the driver on Windows
First of all, we have to meet some requirements. This means we need to install the Git for Windows and Visual Studio 2017 with following components:
Visual C++ 2017 (vc15) or Visual C++ 2019 (vs16) must be installed prior SDK usage. Required components
- C++ dev
- Windows SDK
- .NET dev

Also make sure you are using a 64-bit build host with Windows 7 or later.
Of course we need some Firebird related stuff. The easiest way is to install the related Firebird version on your build host including the development files.

To start the build process open a command line. We assume that the build is done in the directory `c:\php-sdk`. So make sure you have the permission to create that folder on Drive C:.
```
git clone https://github.com/Microsoft/php-sdk-binary-tools.git c:\php-sdk
cd c:\php-sdk
git checkout php-sdk-2.2.0
```
With the above we downloaded the PHP SDK and entered our working directory. 

Next we will prepare our build environment.
For Win32 do:
```
phpsdk-vc15-x86.bat
```
Use following command for Win64:
```
phpsdk-vc15-x64.bat
```
If you use VS 2019, replace vc15 by vs16.

Now let's create the build structure, download the PHP sources and checkout the desired development branch:
```
phpsdk_buildtree phpmaster
git clone https://github.com/php/php-src.git && cd php-src && git checkout PHP-7.4.0
```

Since we have our PHP sources now, we're on to get the depending libraries.
```
phpsdk_deps --update --branch 7.4
```

In the next step we will download our Firebird extension sources.
```
mkdir ..\pecl
git clone https://github.com/FirebirdSQL/php-firebird.git ..\pecl\interbase
```

If everything is ok, we can now compile our PHP extension. Please specify the the correct path to your Firebird installation.

#### Build TS extension
Usually you will build thread safe extensions.
For Win32 thread safe (TS) do:
```
buildconf && configure --enable-cli --with-interbase="shared,C:\Program Files (x86)\Firebird\3_0" && nmake
```
For Win64thread safe (TS) do:
```
buildconf && configure --enable-cli --with-interbase="shared,C:\Program Files\Firebird\3_0" && nmake
```
After the compilation you can find your extension called `php_interbase.dll` e.g. in `C:\php-sdk\phpmaster\vc15\x64\php-src\x64\Release_TS\php_interbase.dll`
Replace x64 with x86 for Win32.

#### Build NTS extension
For Win32 non-thread safe (NTS) run:
```
buildconf && configure --disable-zts --enable-cli --with-interbase="shared,C:\Program Files (x86)\Firebird\3_0" && nmake
```
For Win64 non-thread safe (NTS) run:
```
buildconf && configure --disable-zts --enable-cli --with-interbase="shared,C:\Program Files\Firebird\3_0" && nmake
```
After the compilation you can find your extension called `php_interbase.dll` e.g. in `C:\php-sdk\phpmaster\vc15\x86\php-src\Release`
Replace x86 with x64 for Win64.





