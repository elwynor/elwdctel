@echo off
rem Build 26.10.19.1 11:15PM
rem RUN_TESTS.CMD - build and run the Dice for Teleconference unit tests.
rem Needs Visual Studio 2022 (or Build Tools 2022).  Run from anywhere.
rem Exit code 0 = all tests passed.
setlocal
set VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat
if not exist "%VCVARS%" set VCVARS=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat
call "%VCVARS%" >nul || exit /b 2
set HERE=%~dp0
set SRC=%HERE%..\SRC\ELWDCTEL
set OUT=%HERE%..\BUILD\tests
if not exist "%OUT%" mkdir "%OUT%"
cl /nologo /W3 /J /EHsc /I"%SRC%" /Fo"%OUT%\\" /Fe"%OUT%\TEST_UTIL.EXE" ^
   "%HERE%TEST_UTIL.CPP" "%SRC%\DC_UTIL.CPP" >"%OUT%\build.txt" || (type "%OUT%\build.txt" & exit /b 2)
"%OUT%\TEST_UTIL.EXE"
exit /b %ERRORLEVEL%
