@ECHO off
setlocal enabledelayedexpansion

REM
REM Simple batch script to create a Visual Studio project using premake5 and the ludumdare56.lua script.
REM
REM <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
REM --------------------------------------------------------------------------------------------------------------------

SET PROJECT_NAME=ludumdare56

REM 2024-08-03: Everything below this point is directly copied in each make_project.bat unless otherwise noted or this
REM   comment gets removed. The only thing that changes is the PROJECT_NAME variable above, and we should attempt to
REM   keep the rest of it that generic as it makes it easier to maintain. Perhaps a project or two will need specific
REM   edits or changes, but lets ensure there is good reason to.
REM --------------------------------------------------------------------------------------------------------------------
REM --------------------------------------------------------------------------------------------------------------------
REM --------------------------------------------------------------------------------------------------------------------

SET FLAG_CLEAN=0
SET FLAG_BUILD=0
SET FLAG_BUILD_ALL=1
SET FLAG_BUILD_DEBUG=0
SET FLAG_BUILD_RELEASE=0
SET FLAG_BUILD_PUBLIC=0
SET DEFAULT_MAKE_JOBS=4
SET FLAG_MAKE_JOBS=%DEFAULT_MAKE_JOBS%

REM May wish to add FLAG_WIN32 and FLAG_WINDOWS (64bit), and plausibly FLAG_WEB if emscripten gets installed.
REM   2024-08-03: WIN32 is being deprecated and is no longer supported. All Window builds are forced to  64bit.
SET FLAG_WINDOWS=1
SET FLAG_WIN32=0

SET /P DEFAULT_BUILD_VERSION=<version.txt
IF NOT DEFINED DEFAULT_BUILD_VERSION (
	SET DEFAULT_BUILD_VERSION=0.0.0
)

SET BUILD_VERSION=%DEFAULT_BUILD_VERSION%

REM 2024-08-08: abs_detail_file already contains quotes, so we use them here to ensure everything works as expected.
SET BUILD_REPORT_FILE="build_report.txt"
IF DEFINED abs_detailed_report_file (
	SET BUILD_REPORT_FILE=%abs_detailed_report_file%
)

REM --------------------------------------------------------------------------------------------------------------------

SET argumentCount=0
for %%x in (%*) do (
	SET /A argumentCount+=1
	SET "argumentValues[!argumentCount!]=%%~x"
)

REM For debugging the array of collected arguments...
REM ECHO Number of processed arguments: %argumentCount%
REM for /L %%i in (1,1,%argumentCount%) do (
REM 	echo %%i- "!argumentValues[%%i]!"
REM )

REM The arguments array is 1-based mostly because I couldn't figure out the debugging line above with count - 1 being
REM   the last number. To get back to ZERO based indices, in the case we can:
REM   1. Move the argumentCount+=1 in the top for-loop to below the argumentValues line.
REM   2. for ... in (0, 1, count - 1) on the second loop for debugging.
REM   3. Change the if statement below to if %argumentIndex% LSS %argumentCount%
SET argumentIndex=1
SET nextArgumentIndex=2
:process_arguments_loop
	if %argumentIndex% LEQ %argumentCount% (
		SET argumentName=!argumentValues[%argumentIndex%]!

		IF "--clean"=="!argumentName!" (
			SET /A FLAG_CLEAN=1
		)
		IF "--build"=="!argumentName!" (
			SET /A FLAG_BUILD=1
		)

		IF "--win32"=="!argumentName!" (
			ECHO Error: windows platform not fully supported yet, use win32 for 32bit builds.
			REM SET /A FLAG_BUILD=1
			REM SET /A FLAG_WIN32=1
			goto :end_of_script
		)
		IF "--windows"=="!argumentName!" (
			SET /A FLAG_BUILD=1
			SET /A FLAG_WINDOWS=1
		)

		IF "--build-version"=="!argumentName!" (
			SET /A argumentIndex=argumentIndex + 1
			SET BUILD_VERSION=!argumentValues[%nextArgumentIndex%]!
			IF NOT DEFINED BUILD_VERSION (
				SET BUILD_VERSION=!DEFAULT_BUILD_VERSION!
			)
		)

		IF "--debug"=="!argumentName!" (
			SET FLAG_BUILD_DEBUG=1
			SET FLAG_BUILD_ALL=0
			SET FLAG_BUILD=1
		)
		IF "--release"=="!argumentName!" (
			SET FLAG_BUILD_RELEASE=1
			SET FLAG_BUILD_ALL=0
			SET FLAG_BUILD=1
		)
		IF "--public"=="!argumentName!" (
			SET FLAG_BUILD_PUBLIC=1
			SET FLAG_BUILD_ALL=0
			SET FLAG_BUILD=1
		)

		IF "-j"=="!argumentName!" (
			SET /A argumentIndex=argumentIndex + 1
			SET FLAG_MAKE_JOBS=!argumentValues[%nextArgumentIndex%]!
			IF NOT DEFINED FLAG_MAKE_JOBS (
				SET FLAG_MAKE_JOBS=!DEFAULT_MAKE_JOBS!
			)
		)

		REM Increment index and loop again while index is valid.
		SET /A argumentIndex=argumentIndex + 1
		SET /A nextArgumentIndex=nextArgumentIndex + 1
		goto :process_arguments_loop
	)

IF %FLAG_BUILD% EQU 1 (
	IF %FLAG_BUILD_ALL% EQU 1 (
		SET /A FLAG_BUILD_DEBUG=1
		SET /A FLAG_BUILD_RELEASE=1
		SET /A FLAG_BUILD_PUBLIC=1
	)

	REM Check to ensure the platform directory at least exists, if not forcefully clean.
	REM   Unfortunately the way we are doing this will forcefully clean ALL targets
	REM   including those platforms that may already be partially built.
	IF %FLAG_WINDOWS% EQU 1 (
		IF NOT EXIST "windows/%PROJECT_NAME%.sln" (
			SET /A FLAG_CLEAN=1
		)
	)
)

REM --------------------------------------------------------------------------------------------------------------------
REM --------------------------------------------------------------------------------------------------------------------
REM --------------------------------------------------------------------------------------------------------------------

IF %FLAG_CLEAN% EQU 1 (
	ECHO "performing a clean build"
	premake5 --file="%PROJECT_NAME%.lua" --build-version=%BUILD_VERSION% clean
)

premake5 --file="%PROJECT_NAME%.lua" --build-version=%BUILD_VERSION% vs2022

IF %FLAG_BUILD% EQU 1 (
	IF NOT DEFINED DevEnvDir (
		REM Used on 32bit Windows XP machine with VisualStudio 2010
		REM CALL "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
		REM Used on 64bit Windows 10 machine with VisualStudio 2015
		REM CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
		REM Used on 64bit Windows 10 machine with VisualStudio 2022 (aka Moose)
		CALL "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
	)

	REM Without /maxcpucount msbuild will only use a single core, can specify a number /maxcpucount:2 or leave blank for all.
	REM /nologo hides a few lines being printed that we don't care about.
	REM /verbosity:quiet seems to give only warnings and errors about building the project.
	REM /flp1 is short for fileloggerparemeters:1 and sets up log file location and appends to it.
	SET extra_options=/nologo /maxcpucount:%FLAG_MAKE_JOBS% /verbosity:quiet /flp1:logfile=%BUILD_REPORT_FILE%;verbosity=quiet;append=true

	IF %FLAG_WINDOWS% EQU 1 (
		IF %FLAG_BUILD_DEBUG% EQU 1 (
			ECHO building debug of %CD%"/windows/%PROJECT_NAME%.sln"
			msbuild "windows/%PROJECT_NAME%.sln" /property:Configuration=debug /property:Platform="x64" !extra_options!
			IF NOT 0 == %errorlevel% (
				ECHO debug build windows failed
				IF NOT "%abs_detailed_report_file%" == "" (
					EXIT /B 1
				)
			)
		)

		IF %FLAG_BUILD_RELEASE% EQU 1 (
			ECHO building release of %CD%"/windows/%PROJECT_NAME%.sln"
			msbuild "windows/%PROJECT_NAME%.sln" /property:Configuration=release /property:Platform="x64" !extra_options!
			IF NOT 0 == %errorlevel% (
				ECHO release build windows failed
				IF NOT "%abs_detailed_report_file%" == "" (
					EXIT /B 1
				)
			)
		)

		IF %FLAG_BUILD_PUBLIC% EQU 1 (
			ECHO building public of %CD%"/windows/%PROJECT_NAME%.sln"
			msbuild "windows/%PROJECT_NAME%.sln" /property:Configuration=public /property:Platform="x64" !extra_options!
			IF NOT 0 == %errorlevel% (
				ECHO public build windows failed
				IF NOT "%abs_detailed_report_file%" == "" (
					EXIT /B 1
				)
			)
		)
	)
) ELSE (
	ECHO No configurations built, %PROJECT_NAME% solution can be opened with Visual Studio 2022.
)

:end_of_script
