@echo off
if "%ZGB_PATH%"=="" goto :continue
echo CrossZGB is already installed!
goto :EOF

:continue
echo Installing CrossZGB:

echo Adding path to MAKE and MSYS into the system PATH...
setx /M PATH "%cd%\..\env\make-3.81-bin\bin;%cd%\..\env\msys\bin\;%PATH%"

echo Setting ZGB_PATH to "%cd%\common"...
setx ZGB_PATH "%cd%\common"

echo Setting GBDK_HOME to "%cd%\..\gbdk"...
setx GBDK_HOME "%cd%\..\gbdk"

echo Done!