# Scripts building php-firebird extension on Windows

## How it works

These scripts will clone or pull corresponding PHP version(s) from the PHP source repo and build 4 .dll files per PHP version: x86, x64 and ts and non-ts for each architecture.

Do not run scripts with ``-sdk-`` in their files names directly. These are called from php-sdk environment.

Make sure you got ~20GB free disk space to build for all PHP versions.

## Set up

Make sure ``git`` is in you PATH

1. Set up Microsoft Visual Studio vc15 and vs16.
2. Set up Firebird 32-bit and 64-bit installations or libraries.
3. Set up PHP-SDK according to https://wiki.php.net/internals/windows/stepbystepbuild_sdk_2
4. Clone php-firebird extension source somewhere.
4. Copy these build scripts to C:\php-sdk\
5. Adjust php-fb-config.bat.

    ``Note: PFB_SOURCE_DIR should point one level up. For example
    PFB_SOURCE_DIR=D:\php-firebird\ then your source should reside in D:\php-firebird\php-firebird\
    ``
6. Run ``php-fb-build-all.bat`` to build for all PHP versions or run ``php-fb-build.bat 7.4 vc15`` to build for particular version.
