@ECHO off
setlocal enabledelayedexpansion

REM
REM Automated Build Script for LudumDare56 to copy the executable during a release build.
REM
REM <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
REM
REM Used to collect arguments: https://stackoverflow.com/questions/19835849/batch-script-iterate-through-arguments
REM -------------------------------------------------------------------------------------------------------------------

SET buildConfig=
SET buildPlatform=
SET executablePostfix=
SET executableName=
SET runDirectory="..\..\run\"

REM -------------------------------------------------------------------------------------------------------------------

SET argumentCount=0
for %%x in (%*) do (
	SET /A argumentCount+=1
	SET "argumentValues[!argumentCount!]=%%~x"
)

REM For debugging the array of collected arguments...
REM ECHO Number of processed arguments: %argumentCount%
REM for /L %%i in (1,1,%argumentCount%) do echo %%i- "!argumentValues[%%i]!"

SET /A argumentIndex=0
:process_arguments_loop
	if %argumentIndex% LSS %argumentCount% (
		SET argumentName=!argumentValues[%argumentIndex%]!

		IF "--build-config"=="!argumentName!" (
			REM This is some real magic provided by Neui from twitch! Grab next value in array.
			REM Could also break it down to a function: https://pastebin.com/tBzt1JnJ
			SET /A argumentIndex=argumentIndex + 1
			SET value=argumentValues[!argumentIndex!]
			CALL SET "value=%%!value!%%"

			SET buildConfig=!value!
			SET executablePostfix=_!buildConfig!
		) ELSE IF "--platform"=="!argumentName!" (
			SET /A argumentIndex=argumentIndex + 1
			SET value=argumentValues[!argumentIndex!]
			CALL SET "value=%%!value!%%"

			SET buildPlatform=!value!
		) ELSE IF "--name"=="!argumentName!" (
			SET /A argumentIndex=argumentIndex + 1
			SET value=argumentValues[!argumentIndex!]
			CALL SET "value=%%!value!%%"

			SET executableName=!value!
		)

		REM Increment index and loop again while index is valid.
		SET /A argumentIndex=argumentIndex + 1
		goto :process_arguments_loop
	)

REM For debugging the array of collected arguments...
REM for /L %%i in (1,1,%argumentCount%) do echo %%i- "!argumentValues[%%i]!"

ECHO Copying from %buildConfig% build with postfix: %executablePostfix%

REM -------------------------------------------------------------------------------------------------------------------

IF NOT DEFINED executableName (
	ECHO "Undefined executableName, please use --name project_name to set this."
	EXIT /B 404
)
IF NOT DEFINED buildConfig (
	ECHO "Undefined buildConfig, please use --build-config <debug, development, release, or public>"
	EXIT /B 404
)
IF NOT DEFINED executablePostfix (
	ECHO "Undefined executablePostfix, please use --build-config <debug, development, release, or public>"
	EXIT /B 404
)

IF "tb_windows"=="%buildPlatform%" (
	IF EXIST "%runDirectory%%executableName%%executablePostfix%.exe" (DEL "%runDirectory%%executableName%%executablePostfix%.exe")
	COPY "%buildConfig%\%executableName%.exe" "%runDirectory%\%executableName%%executablePostfix%.exe"
) ELSE (
	ECHO "Unknown buildPlatform: %buildPlatform%, please use --platform <tb_windows>"
)
