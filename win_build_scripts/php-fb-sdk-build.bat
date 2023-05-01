@echo off
@REM
@REM Must be called under phpsdk-<php_vers>-<arch>.bat
@REM
@REM Calling script should set variables:
@REM <PFB_FB32_DIR> <PFB_FB64_DIR> <PFB_SOURCE_DIR> <pfb_php_vers> [pfb_nts] [pfb_x86]
@REM
@REM set pfb_php_vers=7.4
@REM set pfb_nts=1 if nts expected, 0 if ts
@REM set pfb_x86=1 if linking to x86 fbclient, o if x64
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

    if "%pfb_nts%" gtr "0"  (
        set build_msg=%build_msg% non-TS
        set extra_args=--disable-zts
    ) else (
        set build_msg=%build_msg% TS
        set extra_args=
    )

    if "%pfb_x86%" gtr "0"  (
        set with_interbase="shared,%PFB_FB32_DIR%lib;%PFB_FB32_DIR%include"
        set build_msg=%build_msg% x86
    ) else (
        set with_interbase="shared,%PFB_FB64_DIR%lib;%PFB_FB64_DIR%include"
        set build_msg=%build_msg% x86_64
    )

    call :log "%build_msg%"

    call phpsdk_buildtree php%pfb_php_vers%
    cd /D php-src
    call buildconf.bat --force --add-modules-dir=%PFB_SOURCE_DIR%
    call configure.bat --disable-all --enable-cli %extra_args% --with-interbase=%with_interbase%
    nmake
