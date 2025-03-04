@REM
@REM git command must be in PATH
@REM

@REM php-firebird source directory
set PFB_SOURCE_DIR=D:\php-firebird\

for /f %%i in ('git -C %PFB_SOURCE_DIR%\php-firebird\ rev-parse --short HEAD') do set GIT_HASH=%%i

@REM sets php-firebird version part in extension file, for example, php_interbase-<<3.0.1-ba8e63b>>-7.3-vc15.dll
set PFB_VERS=3.0.1-%GIT_HASH%

@REM Directory where all compiled files will be copied
set PFB_OUTPUT_DIR=D:\php-firebird\releases\

@REM FB 32-bit and 64-bit libraries
set FB32_DIR=C:\Program Files\Firebird\Firebird_5_0-x86
set FB64_DIR=C:\Program Files\Firebird\Firebird_5_0
