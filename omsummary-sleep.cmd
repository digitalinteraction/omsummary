@echo off
setlocal

if not exist "%~dp0omsummary.exe" goto notfound_exe

if "%~1"=="" goto usage

:run

set FILE=%~dpnx1
if not exist "%FILE%" goto notfound_file
if not "%FILE:~-10%"==".sleep.csv" goto invalid_file

set TIMES=%FILE:~0,-10%.sleep.times.csv
if not exist "%TIMES%" goto notfound_times

set OUTPUT=%FILE:~0,-10%.sleep.summary.csv

echo --- %FILE% ---
echo on
"%~dp0omsummary.exe" -mode:sleep -in "%FILE%" -times "%TIMES%" -out "%OUTPUT%"
@echo off
echo ----------

shift
if not "%1"=="" goto run
goto done



:usage
echo.No files specified -- drag files on to this program.
pause
goto end

:notfound_exe
echo.Executable file not found: "%~dp0omsummary.exe"
pause
goto end

:invalid_file
echo ERROR: Input sleep file not of expected name (.sleep.csv): %FILE%
pause
goto end

:notfound_file
echo ERROR: Input sleep file not found: %FILE%
pause
goto end

:notfound_times
echo ERROR: Times file not found: %TIMES%
pause
goto end

:done
echo.
echo Done!
pause
goto end


:end
