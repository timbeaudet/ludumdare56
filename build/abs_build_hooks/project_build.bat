@ECHO off

REM
REM Simple batch script hooked into the Automated Build System to run project specific build commands. If an error
REM occurs that should halt the build and fail the nightly; set an error-code above 10 in the variable %abs_return_value%
REM   SET abs_return_value=11
REM 
REM To log any details to the email report, use ECHO like so:
REM   (ECHO "Here are some details about the custom step.")>>%abs_detailed_report_file%
REM -------------------------------------------------------------------------------------------------------------------
