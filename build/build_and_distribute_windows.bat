@ECHO off

REM
REM Build and prepare the game for distribution. To actually upload/deploy use rhino and call the sister script like so:
REM   build_and_distribute.sh --windows
REM
REM <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
REM --------------------------------------------------------------------------------------------------------------------

SET /P BUILD_VERSION=<version.txt

ECHO Creating a PUBLIC build of LudumDare56 v%BUILD_VERSION%
CALL make_project.bat --clean --build --public --build-version=%BUILD_VERSION%

SET RUN_DIRECTORY="..\run"
SET RELEASE_DIRECTORY="..\distribution\windows\ludumdare56_%BUILD_VERSION%_windows"

IF 0 == %errorlevel% (
	ECHO BuildAndDistribute: Creating a packaged zip to upload on itch.io and tyrebytes.com/games
	ECHO BuildAndDistribute: Package will be placed at: %RELEASE_DIRECTORY%

	IF EXIST "%RELEASE_DIRECTORY%" (RMDIR /Q /S "%RELEASE_DIRECTORY%")
	IF EXIST "%RELEASE_DIRECTORY%.zip" (DEL /Q /S /F "%RELEASE_DIRECTORY%.zip")

	MKDIR %RELEASE_DIRECTORY%
	COPY "%RUN_DIRECTORY%\ludumdare56_public.exe" %RELEASE_DIRECTORY%\ludumdare56.exe
	COPY "%RUN_DIRECTORY%\glew32.dll" %RELEASE_DIRECTORY%
	COPY "%RUN_DIRECTORY%\readme.txt" %RELEASE_DIRECTORY%
	XCOPY /D /S /Q /EXCLUDE:exclude_windows.txt "%RUN_DIRECTORY%\data" "%RELEASE_DIRECTORY%\data\"

	powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('%RELEASE_DIRECTORY%', '%RELEASE_DIRECTORY%.zip'); }"

	ECHO BuildAndDistribute: Build was a success, compressed to zip, now moving to rhino for further distribution.
	REM build_and_distribute_windows.bat does not actually perform any distributions it will just upload to
	REM the local linux file server (rhino) and await the build_and_distribution.sh to be run from there.
	MKDIR Z:\game_builds\ludumdare56\%BUILD_VERSION%\
	COPY %RELEASE_DIRECTORY%.zip "Z:\game_builds\ludumdare56\%BUILD_VERSION%\ludumdare56_%BUILD_VERSION%_windows.zip"
	ECHO Copied to rhino/game_builds - Upload using build_and_distribute.sh from rhino.

	REM It is possible for Windows to upload directly to itch.io, though this has not been tested since the
	REM script was added to the template project. Simply uncomment the next four lines to try it out:

	REM ECHO BuildAndDistribute: Build was a success, now deploying to itch.io using butler...
	REM Deploy to itch.io account using butler app.
	REM butler push --userversion %BUILD_VERSION% ..\distribution\windows\ludumdare56_%BUILD_VERSION%_windows.zip timbeaudet/ludumdare56:windows-alpha
	REM ECHO BuildAndDistribute: You must MANUALLY upload the %RELEASE_DIRECTORY%.zip package.
) ELSE (
	ECHO BuildAndDistribute: Build failed, skipping distribution.
)
