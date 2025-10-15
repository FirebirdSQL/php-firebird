@echo off
@REM
@REM Must be called under phpsdk-<php_vers>-<arch>.bat
@REM
@REM Calling script should set variables:
@REM <PFB_FB32_DIR> <PFB_FB64_DIR> <PFB_SOURCE_DIR> <pfb_php_vers> [pfb_ts] [pfb_arch]
@REM
@REM set pfb_php_vers=7.4
@REM set pfb_ts=1 if thread safety enabled, 0 if not
@REM set pfb_arch=x86 to build agains 32-bit, otherwise 64-bit
@REM
@REM <PFB_FB32_DIR> <PFB_FB64_DIR> <PFB_SOURCE_DIR> all set in php-fb-config.bat
@REM

goto :MAIN

@REM log  =========================================================================================
@REM log <msg>
@REM example> call :log "<msg>"
:log
    set msg=%~1
    echo ---------------------------------------------------------------------
    echo %msg%
    echo ---------------------------------------------------------------------
exit /B

:MAIN
    if [%pfb_php_vers%] == [] (
        echo pfb_php_vers varible not set
        exit 1
    )

    set build_msg=Building PHP-%pfb_php_vers%

    if "%pfb_ts%" gtr "0" (
        set build_msg=%build_msg% non-TS
        set extra_args=--disable-zts
    ) else (
        set build_msg=%build_msg% TS
        set extra_args=
    )

    if %pfb_arch% EQU x86 (
        set with_interbase="shared,%PFB_FB32_DIR%"
        set build_msg=%build_msg% x86
    ) else (
        set with_interbase="shared,%PFB_FB64_DIR%"
        set build_msg=%build_msg% x86_64
    )

    call :log "%build_msg%"

    call phpsdk_buildtree php%pfb_php_vers%
    cd /D php-src
    call buildconf.bat --force --add-modules-dir=%PFB_SOURCE_DIR%
    call configure.bat --disable-all --enable-cli %extra_args% --with-interbase=%with_interbase%
    nmake
