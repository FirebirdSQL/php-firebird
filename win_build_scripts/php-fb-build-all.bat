@echo off

call php-fb-build.bat 7.3 vc15 || exit /B %ERRORLEVEL%
call php-fb-build.bat 7.4 vc15 || exit /B %ERRORLEVEL%
call php-fb-build.bat 8.0 vs16 || exit /B %ERRORLEVEL%
call php-fb-build.bat 8.1 vs16 || exit /B %ERRORLEVEL%
call php-fb-build.bat 8.2 vs16 || exit /B %ERRORLEVEL%
call php-fb-build.bat master vs16 || exit /B %ERRORLEVEL%
