@echo off
@REM Do not modify this file directly.
@REM Create build_scripts/php-fb-config.bat if you need to change any variable
@REM and put overrides there

@REM php-firebird source directory
@REM Should point one level up. For example
@REM PFB_SOURCE_DIR=D:\php-firebird\ then your source should reside in D:\php-firebird\php-firebird\
set PFB_SOURCE_DIR=%~dp0..\..\

@REM Directory where all compiled files will be copied
set PFB_OUTPUT_DIR=%PFB_SOURCE_DIR%releases\

@REM FB 32-bit and 64-bit libraries
set PFB_FB32_DIR=C:\Program Files\Firebird\Firebird_5_0-x86
set PFB_FB64_DIR=C:\Program Files\Firebird\Firebird_5_0

@REM Attach current git commit hash to version. git command must be in PATH
set PFB_ATTACH_GIT_HASH_TO_VERS=0

@REM Additional flags for ./configure
@REM set PFB_CONFIGURE_FLAGS=--enable-debug
