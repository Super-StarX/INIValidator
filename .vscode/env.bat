@echo off
SET vswherePath=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
IF NOT EXIST "%vswherePath%" GOTO :ERROR


SET toolsSuffix=x86.x64
IF /I "%~1"=="arm64" SET toolsSuffix=ARM64

FOR /F "tokens=*" %%i IN (
    '"%vswherePath%" -latest -prerelease -products * ^
    -requires Microsoft.VisualStudio.Component.VC.Tools.%toolsSuffix% ^
    -version [16^,18^) ^
    -property installationPath'
) DO SET vsBase=%%i

IF "%vsBase%"=="" GOTO :ERROR

"%vsBase%\Common7\Tools\VsDevCmd.bat"
GOTO :EOF

:ERROR
ECHO Visual Studio not found

GOTO :EOF