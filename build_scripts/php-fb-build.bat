@echo off
@REM php-fb-build.bat <pfb_php_tag> <pfb_cpp_vers> <pfb_ts> <pfb_arch>
@REM php-fb-build.bat php-7.4.13 vc15 [0|1] [x64|x86]
@REM

@REM config  ======================================================================================
call %~dp0php-fb-config.bat

set pfb_php_tag=%1
set pfb_cpp_vers=%2
set pfb_ts=%3
set pfb_arch=%4

if "%pfb_php_tag%" == "" (
    call :log "pfb_php_tag varible not set"
    call :usage
    exit 1
)

if "%pfb_cpp_vers%" == "" (
    call :log "pfb_cpp_vers varible not set"
    call :usage
    exit 1
)

if "%pfb_ts%" == "1" (
    set pfb_ts=1
) else (
    set pfb_ts=0
)

if not "%pfb_arch%" == "x86" (
    set pfb_arch=x64
)

@REM Convert php-8.4.13 -> 8.4
for /f "tokens=2,3 delims=-." %%a in ("%pfb_php_tag%") do set pfb_php_vers=%%a.%%b

if "%pfb_php_vers%" == "" (
    call :log "BUG: pfb_php_vers should be set at this point"
    exit 1
)

@REM Initialize
set php_root=php%pfb_php_vers%\%pfb_cpp_vers%\%pfb_arch%\php-src\
set php_interbase=php_interbase-%PFB_VERS%-%pfb_php_vers%-%pfb_cpp_vers%

if not exist "%php_root%.git\" (
    call :log "Cloning %pfb_php_tag% %pfb_arch%"
    call phpsdk-%pfb_cpp_vers%-%pfb_arch%.bat -t %~dp0php-fb-sdk-init.bat || goto :error
)

if "%pfb_arch%" == "x86" (
    set build_root=%php_root%
    set php_interbase=%php_interbase%-x86
) else (
    set build_root=%php_root%x64\
)

if %pfb_ts% equ 1 (
    set build_root=%build_root%Release_TS\
) else (
    set build_root=%build_root%Release\
    set php_interbase=%php_interbase%-nts
)

@REM Build
call :log "Building %php_interbase%.dll..."
call phpsdk-%pfb_cpp_vers%-%pfb_arch%.bat -t %~dp0php-fb-sdk-build.bat || goto :error

@REM Validate
set vb_check_code=^
if(!extension_loaded('interbase')){ print \"Extension not loaded\n\"; exit(1); }^
if('php-'.PHP_VERSION != '%pfb_php_tag%'){ printf(\"Version mismatch: expected '%pfb_php_tag%', but got '%%s' \n\", 'php-'.PHP_VERSION); exit(1); }^
if((int)ZEND_THREAD_SAFE != %pfb_ts%){ printf(\"Thread Safety mismatch: expected %pfb_ts%, but got %%d \n\", ZEND_THREAD_SAFE); exit(1); }^
if((PHP_INT_SIZE == 8 ? 'x64' : 'x86') != '%pfb_arch%'){ printf(\"Architecture mismatch: expected '%pfb_arch%', but got '%%s' \n\", (PHP_INT_SIZE == 8 ? 'x64' : 'x86')); exit(1); }

if "%pfb_arch%" == "x86" (
    set vb_libs=%PFB_FB32_DIR%
) else (
    set vb_libs=%PFB_FB64_DIR%
)

call :log "Validating %php_interbase%.dll..."
set vb_cmd=cmd /c set "PATH=%vb_libs%;%PATH%" %build_root%php_exe -dextension=.\php_interbase.dll -r "%vb_check_code%"
%vb_cmd% || goto :error

call :log "Copying %php_interbase%.dll..."
copy "%build_root%php_interbase.dll" "%PFB_OUTPUT_DIR%%php_interbase%.dll" || goto :error

call :log "Build OK" "%pfb_php_tag% %pfb_cpp_vers% %pfb_arch% Thread Safety %pfb_ts%" "%php_interbase%.dll"

exit /B

@REM log  =========================================================================================
@REM log <msg>
@REM example> call :log "<msg1>"
:log
    echo ---------------------------------------------------------------------
    for %%a in (%*) do ( echo %%~a )
    echo ---------------------------------------------------------------------
exit /B

@REM usage  =======================================================================================
:usage
    call :log "Usage: %~nx0 php_tag cpp_vers [ts=0|1] [arch=x86|x64]" "       Example: %~nx0 php-8.4.13 vs17 1 x86"
exit /B

@REM error  =======================================================================================
:error
    call :log "Build FAILED" "%pfb_php_tag% %pfb_cpp_vers% %pfb_arch% Thread Safety %pfb_ts%" "%php_interbase%.dll"
exit /B 1
