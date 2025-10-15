@echo off

@REM config  ======================================================================================
call %~dp0php-fb-config.bat

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

@REM usage  =======================================================================================
:usage
    call :log "Usage: %~nx0 php_tag cpp_vers"
exit /B

@REM validate_build ===============================================================================
@REM validate_build <path_to_php_exe>:string <arch>:string <ts>:int
:validate_build
setlocal disabledelayedexpansion
set vb_php=%~1
set vb_arch=%~2
set vb_ts=%~3

set vb_check_code=^
if(!extension_loaded('interbase')){ print \"Extension not loaded\n\"; exit(1); }^
if('php-'.PHP_VERSION != '%pfb_php_tag%'){ printf(\"Version mismatch: expected '%pfb_php_tag%', but got '%%s' \n\", 'php-'.PHP_VERSION); exit(1); }^
if((int)ZEND_THREAD_SAFE != %vb_ts%){ printf(\"Thread Safety mismatch: expected %vb_ts%, but got %%d \n\", ZEND_THREAD_SAFE); exit(1); }^
if((PHP_INT_SIZE == 8 ? 'x64' : 'x86') != '%vb_arch%'){ printf(\"Architecture mismatch: expected '%vb_arch%', but got '%%s' \n\", (PHP_INT_SIZE == 8 ? 'x64' : 'x86')); exit(1); }

if "%vb_arch%" == "x86" (
    set vb_libs=%PFB_FB32_DIR%
) else (
    set vb_libs=%PFB_FB64_DIR%
)

call :log "Validating %pfb_php_tag% %vb_arch% Thread Safety %vb_ts%"

set vb_cmd=cmd /c set "PATH=%vb_libs%;%PATH%" %php_exe% -dextension=.\php_interbase.dll -r "%vb_check_code%"
%vb_cmd% || exit /B 1

echo Validated OK
echo ---------------------------------------------------------------------

exit /B

:MAIN

set pfb_php_tag=%1
set pfb_cpp_vers=%2

if "%pfb_php_tag%" == "" (
    call :usage
    echo pfb_php_tag varible not set
    exit 1
)

if "%pfb_cpp_vers%" == "" (
    call :usage
    echo pfb_cpp_vers varible not set
    exit 1
)

@REM Convert php-8.4.13 -> 8.4
for /f "tokens=2,3 delims=-." %%a in ("%pfb_php_tag%") do set pfb_php_vers=%%a.%%b

if "%pfb_php_vers%" == "" (
    echo BUG: pfb_php_vers should be set at this point
    exit 1
)

set pfb_build_root=php%pfb_php_vers%\%pfb_cpp_vers%\

(for %%a in (x64 x86) do (
    set pfb_arch=%%a

    if not exist "%pfb_build_root%\%%a\php-src\.git\" (
        call :log "Cloning %pfb_php_tag% %%a"
        call phpsdk-%pfb_cpp_vers%-%%a.bat -t %~dp0php-fb-sdk-init.bat || goto :error
    )

    if "%%a" == "x86" (
        set php_exe_arch=%pfb_build_root%%%a\php-src\
    ) else (
        set php_exe_arch=%pfb_build_root%%%a\php-src\x64\
    )

    setlocal enabledelayedexpansion
    (for %%t in (0 1) do (
        set pfb_ts=%%t
        if "%%t" equ "1" (
            set php_exe="!php_exe_arch!Release_TS\php.exe"
        ) else (
            set php_exe="!php_exe_arch!Release\php.exe"
        )

        if "!php_exe!" == "" (
            echo BUG: php_exe should be set at this point
            exit 1
        )

        call phpsdk-%pfb_cpp_vers%-%%a.bat -t %~dp0php-fb-sdk-build.bat || goto :error

        call :validate_build !php_exe! !pfb_arch! !pfb_ts! || goto :error
    ))
))

echo.
call :log "%pfb_php_tag% build OK"

@REM copy compiled extension to target directory
copy %pfb_build_root%x64\php-src\x64\Release_TS\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%-x64.dll>nul
copy %pfb_build_root%x64\php-src\x64\Release\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%-nts-x64.dll>nul
copy %pfb_build_root%x86\php-src\Release_TS\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%.dll>nul
copy %pfb_build_root%x86\php-src\Release\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%-nts.dll>nul

exit /B 0

:error
    call :log "%pfb_php_tag% build FAILED"

exit /B 1
