@echo off
@REM php-firebird source directory
@REM Should point one level up. For example
@REM PFB_SOURCE_DIR=D:\php-firebird\ then your source should reside in D:\php-firebird\php-firebird\
set PFB_SOURCE_DIR=%~dp0..\..\

@REM Directory where all compiled files will be copied
set PFB_OUTPUT_DIR=%PFB_SOURCE_DIR%releases\

@REM FB 32-bit and 64-bit libraries
set PFB_FB32_DIR=C:\Program Files\Firebird\Firebird_5_0-x86
set PFB_FB64_DIR=C:\Program Files\Firebird\Firebird_5_0

@REM Attach current git commit hash. git command must be in PATH
set USE_GIT_HASH=0

for /f "tokens=3" %%i in ('findstr /b /c:"#define PHP_INTERBASE_VER_MAJOR" %~dp0..\php_interbase.h') do set VER_MAJOR=%%i
for /f "tokens=3" %%i in ('findstr /b /c:"#define PHP_INTERBASE_VER_MINOR" %~dp0..\php_interbase.h') do set VER_MINOR=%%i
for /f "tokens=3" %%i in ('findstr /b /c:"#define PHP_INTERBASE_VER_REV" %~dp0..\php_interbase.h') do set VER_REV=%%i
set PFB_VERS=%VER_MAJOR%.%VER_MINOR%.%VER_REV%-beta

if "%USE_GIT_HASH%" gtr "0" (
	for /f %%i in ('git -C %~dp0..\ rev-parse --short HEAD') do set PFB_VERS=%PFB_VERS%-%%i
)
