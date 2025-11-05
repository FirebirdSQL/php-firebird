@echo off

set "phps="php-7.4.13 vc15" "php-8.0.30 vs16" "php-8.1.33 vs16" "php-8.2.29 vs16" "php-8.3.26 vs16" "php-8.4.13 vs17" "php-8.5.0RC2 vs17""

setlocal enabledelayedexpansion
for %%p in (%phps%) do (
    for /f "tokens=1,2" %%a in (%%p) do (
        set php_vers=%%a
        set cpp_vers=%%b
        call %~dp0php-fb-build.bat !php_vers! !cpp_vers! 1 x64 || exit %ERRORLEVEL%
        call %~dp0php-fb-build.bat !php_vers! !cpp_vers! 0 x64 || exit %ERRORLEVEL%
        call %~dp0php-fb-build.bat !php_vers! !cpp_vers! 1 x86 || exit %ERRORLEVEL%
        call %~dp0php-fb-build.bat !php_vers! !cpp_vers! 0 x86 || exit %ERRORLEVEL%
    )
)
endlocal
