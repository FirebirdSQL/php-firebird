@echo off
@REM
@REM Must be called under phpsdk-<php_vers>-<arch>.bat
@REM
@REM Calling script should set variables: <pfb_php_vers>
@REM
@REM set pfb_php_vers=7.4

if [%pfb_php_vers%] == [] (
    echo pfb_php_vers varible not set
    exit 1
)

@REM Handle current master branch
if "%pfb_php_vers%" == "master" (
    set pfb_git_args=
) else (
    set pfb_git_args=--branch PHP-%pfb_php_vers%
)

call phpsdk_buildtree php%pfb_php_vers%
git clone --depth 1 %pfb_git_args% https://github.com/php/php-src.git
cd php-src

@REM Remove built-in extension
if "%pfb_php_vers%" == "7.3" (
    rd /Q /S ext\interbase
)

call phpsdk_deps --update
