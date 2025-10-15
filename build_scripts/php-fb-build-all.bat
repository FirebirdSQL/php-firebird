@echo off

call %~dp0php-fb-build.bat php-7.3.33 vc15 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-7.4.13 vc15 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-8.0.30 vs16 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-8.1.33 vs16 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-8.2.29 vs16 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-8.3.26 vs16 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-8.4.13 vs17 || exit %ERRORLEVEL%
call %~dp0php-fb-build.bat php-8.5.0RC2 vs17 || exit %ERRORLEVEL%
