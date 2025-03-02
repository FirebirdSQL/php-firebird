@echo off

@REM config  ======================================================================================
call php-fb-config.bat

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

:usage
    call :log "Usage: %~nx0 php_vers cpp_vers"
exit /B

:MAIN
set pfb_php_vers=%1
set pfb_cpp_vers=%2

if [%pfb_php_vers%] == [] (
    call :usage
    echo pfb_php_vers varible not set
    exit 1
)

if [%pfb_cpp_vers%] == [] (
    call :usage
    echo pfb_cpp_vers varible not set
    exit 1
)

set pfb_build_root=php%pfb_php_vers%\%pfb_cpp_vers%\

(for %%a in (x86 x64) do (
    @REM check out or pull PHP version of interest
    if exist %pfb_build_root%\%%a\php-src\.git\ (
        call :log "Checking out PHP-%pfb_php_vers% %%a"
        git -C %pfb_build_root%\%%a\php-src pull || goto :error
    ) else (
        call :log "Cloning PHP-%pfb_php_vers% %%a"
        call phpsdk-%pfb_cpp_vers%-%%a.bat -t php-fb-sdk-init.bat || goto :error
    )

    if %%a EQU x86 ( set pfb_x86=1 ) else ( set pfb_x86=0 )

    (for %%n in (0 1) do (
        set pfb_nts=%%n
        call phpsdk-%pfb_cpp_vers%-%%a.bat -t php-fb-sdk-build.bat || goto :error
    ))
))

@REM check if ibase_connect() function exists in newly compiled extension
set check_code="if(!function_exists('ibase_connect'))exit(1);"

set TPATH=%PATH%
set PATH=%FB64_DIR%;%TPATH%
"%pfb_build_root%x64\php-src\x64\Release_TS\php.exe" -dextension=.\php_interbase.dll -r %check_code% || goto :error
"%pfb_build_root%x64\php-src\x64\Release\php.exe" -dextension=.\php_interbase.dll -r %check_code% || goto :error
set PATH=%FB32_DIR%;%TPATH%
"%pfb_build_root%x86\php-src\Release_TS\php.exe" -dextension=.\php_interbase.dll -r %check_code% || goto :error
"%pfb_build_root%x86\php-src\Release\php.exe" -dextension=.\php_interbase.dll -r %check_code% || goto :error
set PATH=%TPATH%

call :log "PHP %pfb_php_vers% build OK"

@REM copy compiled extension to target directory
copy %pfb_build_root%x64\php-src\x64\Release_TS\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%-x86_64.dll>nul
copy %pfb_build_root%x64\php-src\x64\Release\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%-nts-x86_64.dll>nul
copy %pfb_build_root%x86\php-src\Release_TS\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%.dll>nul
copy %pfb_build_root%x86\php-src\Release\php_interbase.dll %PFB_OUTPUT_DIR%php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%-nts.dll>nul

exit /B 0

:error
    call :log "PHP %pfb_php_vers% build FAILED"

exit /B 1
