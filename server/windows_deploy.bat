SET /p ROR_VERSION=Enter version, 0.0.0:
SET RUN_DIRECTORY="..\run"
SET RELEASE_DIRECTORY="..\releases\rallyofrockets_%ROR_VERSION%_windows"

IF EXIST "%RELEASE_DIRECTORY%" (RMDIR /Q /S "%RELEASE_DIRECTORY%")
IF EXIST "%RELEASE_DIRECTORY%.zip" (DEL /Q /S /F "%RELEASE_DIRECTORY%.zip")

MKDIR %RELEASE_DIRECTORY%
COPY "%RUN_DIRECTORY%\rally_of_rockets.exe" %RELEASE_DIRECTORY%
COPY "%RUN_DIRECTORY%\glew32.dll" %RELEASE_DIRECTORY%
COPY "%RUN_DIRECTORY%\readme.txt" %RELEASE_DIRECTORY%
XCOPY /D /S "%RUN_DIRECTORY%\data" "%RELEASE_DIRECTORY%\data\"

powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('%RELEASE_DIRECTORY%', '%RELEASE_DIRECTORY%.zip'); }"

REM The following is an attempt to also deploy the Windows build to ror.com in an automated
REM fashion, I believe it did work as far as uploading, but did NOT place the upload in the
REM correct directory on the ror server.

REM bash -c './windows_upload.sh'

REM SET /p FTP_PASSWORD=Enter ftp password:

REM echo user timbeaudet> ftpcmd.dat
REM echo %FTP_PASSWORD%>> ftpcmd.dat
REM echo cd rallyofrockets.com/builds>> ftpcmd.dat
REM echo put %RELEASE_DIRECTORY%.zip>> ftpcmd.dat
REM echo quit>> ftpcmd.dat
REM ftp -n -s:ftpcmd.dat rallyofrockets.com
REM del ftpcmd.dat
