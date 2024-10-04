@ECHO off

REM
REM Simple batch script hooked into the Automated Build System to run project specific update commands. If an error
REM occurs that should halt the build and fail the nightly; set an error-code above 10 in the variable %abs_return_value%
REM 
REM To log any details to the email report, use:
REM   (ECHO "Here are some details about the custom step.")>>%abs_detailed_report_file%
REM -------------------------------------------------------------------------------------------------------------------

SET abs_initialize_externals="%CD%\initialize_externals.bat"
IF EXIST %abs_initialize_externals% (
	CALL %abs_initialize_externals%
	REM TODO: Check the return value from the hook and set failure if needed.
)
