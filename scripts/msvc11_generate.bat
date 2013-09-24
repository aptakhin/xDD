@if exist "%ProgramFiles%\Microsoft Visual Studio 11.0\VC\VCVARSALL.BAT" call "%ProgramFiles%\Microsoft Visual Studio 11.0\VC\VCVARSALL.BAT"
@if exist "%ProgramFiles(x86)%\Microsoft Visual Studio 11.0\VC\VCVARSALL.BAT" call "%ProgramFiles(x86)%\Microsoft Visual Studio 11.0\VC\VCVARSALL.BAT"

@set QTDIR=C:\Qt\Qt5.1.1

@cd ..

@if not exist "build-vs11" mkdir build-vs11
@cd "build-vs11"
@call cmake.exe -G "Visual Studio 11" ..\

@cd ..
