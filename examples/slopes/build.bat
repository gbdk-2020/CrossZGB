@echo off
:start
cd src
make
cd ..
echo.
echo Press any key to recompile.
pause > nul && pause > nul
cls
goto start