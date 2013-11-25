@echo off

REM Batch file for setting environmental variables INCLUDE and LIB
REM by tcpie, november 2013

echo Batch script to set INCLUDE and LIB environmental variables for usage of this project.
echo.
echo Note that this batch file does not check the variables to see if they are already correct. Thus running this script multiple times may bloat your environmental variables!

set /P continue=Do you wish to continue? (y/n): %=%

IF NOT %continue%==y goto :end

:include_label
IF NOT DEFINED INCLUDE goto :set_new_include	
IF %INCLUDE% == "" goto :set_new_include
goto :set_old_include

:set_new_include
	set temp1=%~dp0
	setx INCLUDE %temp1%
goto :lib_label

:set_old_include
	set temp1=%INCLUDE%;%~dp0
	setx INCLUDE %temp1%

:lib_label
IF NOT DEFINED LIB goto :set_new_lib	
IF %LIB% == "" goto :set_new_lib
goto :set_old_lib

:set_new_lib
	set temp1=%~dp0Release\
	setx LIB %temp1%
	goto :end

:set_old_lib
	set temp1=%LIB%;%~dp0Release\
	setx LIB %temp1%

:end
echo Done. Exiting...
