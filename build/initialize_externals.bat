@ECHO off

REM
REM Simple batch script to grab TurtleBrains, InternalCombustion and TrackBuilder or other dependencies for the project
REM   pulling them into build/externals/.
REM
REM Known Issues:
REM Due to revision not being the most recent revision, it appears that the check for SVN repositories being out of date
REM will always return true (update available) if the REVISION id is not the latest revision of the repository. This
REM probably happens with the git commit id's as well.
REM
REM There are (untested) potential issues; if the local-repo contains changes, if the local-repo is ahead of revision/commit
REM and other potential edge cases. Testing passed under ideal situations where a) local-repo did not exist, b) local-repo
REM was on revision/commit and c) local-repo was at previous revision/commit.
REM
REM <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
REM --------------------------------------------------------------------------------------------------------------------

REM Apparently %var% gets expanded upon reading the script and not during the running
REM the command so when used within a for loop, or IF?, oddities seem to ensue.
REM Enabling delayed expansion and using !var! causes the expansion to happen during
REM the command. Still I think I've seen oddities with regards to nested loops.
SETLOCAL enableextensions ENABLEDELAYEDEXPANSION

REM Skip initialization of externals for automated builds unless the project needs specific commits for external dependencies.
IF %abs_automating_builds% EQU 1 (
	EXIT /B 0
)

SET TURTLE_BRAINS_REVISION=main
SET INTERNAL_COMBUSTION_REVISION=main
SET TRACK_BUILDER_REVISION=main

REM This is a temporary file use to spit out the contents of a command and read them
REM back in to check if the contents was empty, a 4 command hack to the bash solution.
SET init_external_update_file="temp_update_check_file.abs"

echo "Performing: Initialization / Update of Externals."
SET updated_externals=0

REM If there is not abs_detailed_report_file variable, use stdout to display report.
@REM IF "%abs_detailed_report_file%" == "" (
@REM 	SET abs_detailed_report_file="conout$"
@REM )

(ECHO "\n\nupdating external repositories.\n") >> "%abs_detailed_report_file%"
(ECHO "========================================================\n") >> "%abs_detailed_report_file%"

IF NOT EXIST "externals/" (
	MKDIR "externals"
)

REM ---------------------------------------------------------- GIT: turtle_brains
IF EXIST "externals/turtle_brains/" (
	REM 2024-08-01: Would be nice to have a check that actually says there are updats or not, but lets just get things
	REM   done by assuming it always updates for right now. This can be a future improvement.
	SET updated_externals=1

	echo "Updating TurtleBrains to: %TURTLE_BRAINS_REVISION%"
	(git -C "externals/turtle_brains/" fetch)>>"%abs_detailed_report_file%"
	(git -C "externals/turtle_brains/" pull --rebase)>>"%abs_detailed_report_file%"
	(git -C "externals/turtle_brains/" checkout "%TURTLE_BRAINS_REVISION%")>>"%abs_detailed_report_file%"
) ELSE (
	echo "Initializing TurtleBrains to: %TURTLE_BRAINS_REVISION%"
	(git clone git@github.com:TyreBytes/turtle_brains.git "externals/turtle_brains/")>>"%abs_detailed_report_file%"
	(git -C "externals/turtle_brains/" checkout "%TURTLE_BRAINS_REVISION%")>>"%abs_detailed_report_file%"
)

REM ---------------------------------------------------------- GIT: ice
IF EXIST "externals/ice/" (
	REM 2024-08-01: Would be nice to have a check that actually says there are updats or not, but lets just get things
	REM   done by assuming it always updates for right now. This can be a future improvement.
	SET updated_externals=1

	echo "Updating InternalCombustion to a specific commit: %INTERNAL_COMBUSTION_REVISION%"
	(git -C "externals/ice/" fetch)>>"%abs_detailed_report_file%"
	(git -C "externals/ice/" pull --rebase)>>"%abs_detailed_report_file%"
	(git -C "externals/ice/" checkout "%INTERNAL_COMBUSTION_REVISION%")>>"%abs_detailed_report_file%"
) ELSE (
	echo "Initializing InternalCombustion to a specific commit: %INTERNAL_COMBUSTION_REVISION%"
	(git clone git@github.com:TyreBytes/ice.git "externals/ice/")>>"%abs_detailed_report_file%"
	(git -C "externals/ice/" checkout "%INTERNAL_COMBUSTION_REVISION%")>>"%abs_detailed_report_file%"
)

REM ---------------------------------------------------------- GIT: track_builder
IF EXIST "externals/track_builder/" (
	REM 2024-08-01: Would be nice to have a check that actually says there are updats or not, but lets just get things
	REM   done by assuming it always updates for right now. This can be a future improvement.
	SET updated_externals=1

	echo "Updating TrackBuilder to a specific commit: %TRACK_BUILDER_REVISION%"
	(git -C "externals/track_builder/" fetch)>>"%abs_detailed_report_file%"
	(git -C "externals/track_builder/" pull --rebase)>>"%abs_detailed_report_file%"
	(git -C "externals/track_builder/" checkout "%TRACK_BUILDER_REVISION%")>>"%abs_detailed_report_file%"
) ELSE (
	echo "Initializing TrackBuilder to a specific commit: %TRACK_BUILDER_REVISION%"
	(git clone git@github.com:TyreBytes/track_builder.git "externals/track_builder/")>>"%abs_detailed_report_file%"
	(git -C "externals/track_builder/" checkout "%TRACK_BUILDER_REVISION%")>>"%abs_detailed_report_file%"
)

REM ---------------------------------------------------------- BUILD
PUSHD "externals/turtle_brains/build/"
CALL make_project.bat --windows --clean
POPD

PUSHD "externals/ice/build/"
CALL make_project.bat --windows --clean
POPD

PUSHD "externals/track_builder/build/"
CALL make_project.bat --windows --clean
POPD
