#!/usr/bin/env bash

#
# Automated Build Script for LudumDare56 to copy executables from build to run area on Linux / macOS during a build.
#
# <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
#---------------------------------------------------------------------------------------------------------------------#

export FinalProjectName=LudumDare56

export buildConfig=
export tbBuildPlatform=
export buildPlatform=
export executablePostfix=
export executableName=
export runDirectory="../../run/"

POSITIONAL=()
while [[ $# -gt 0 ]]; do
	echo param: $1
	case "$1" in
		--help ) FLAG_HELP=true; shift ;;
		--build-config ) buildConfig="$2"; executablePostfix="_${buildConfig}"; shift 2 ;;
		--platform ) tbBuildPlatform="$2"; shift 2 ;;
		--name ) executableName="$2"; shift 2 ;;
		-* ) echo "Unknown parameter \"$1\""; exit 2 ;;
		* ) POSITIONAL+=("$1"); shift ;;
	esac
done

print_usage() {
	cat <<EOF
Usage:
  $0 [flags]

Flags:
  --help           show this help
  --build-config   the build configuration to copy the executables from; debug, development release, public.
  --platform       the platform/system the build is created for; tb_linux, tb_macos, tb_web, etc.
  --name           the name of the executable
EOF
}

#---------------------------------------------------------------------------------------------------------------------#
if [ -z ${executableName} ]; then
	echo Undefined executableName, please use --name project_name to set this.
	return 404
fi

if [ -z ${buildConfig} ] || [ -z ${executablePostfix} ]; then
	echo "Undefined buildConfig, please use --build-config <debug, development, release, or public>"
	return 404
fi

if [ -z ${tbBuildPlatform} ]; then
	echo "Undefined buildPlatform, please use --platform <tb_macos, tb_linux or tb_web>"
	return 404
fi

# 2024-08-07: Remove the tb_ prefix from platform, eg. tb_linux becomes linux.
# To do a similar thing in batch we would use:   %variable:~3%
#   ikiwixz found this from https://ss64.com/nt/syntax-substring.html
buildPlatform=${tbBuildPlatform#*_};

if [ "tb_linux" == ${tbBuildPlatform} ]; then
	cp "../${buildPlatform}/${buildConfig}/${executableName}" "${runDirectory}${executableName}_linux${executablePostfix}"
elif [ "tb_macos" == ${tbBuildPlatform} ]; then
	#Copy the executable into the run directory, if using source-control this could be committed and shared with team.
	if [ -d "${runDirectory}${executableName}_macos${executablePostfix}" ]; then
		rm "${runDirectory}${executableName}_macos${executablePostfix}"
	fi

	cp "../macos/${buildConfig}/${executableName}.app/Contents/MacOS/${executableName}" "${runDirectory}${executableName}_macos${executablePostfix}"

	if [ "release" == ${buildConfig} ] || [ "public" == ${buildConfig} ]; then
		#If the Mac OS X application package already exists, first delete it before recreating.
		if [ -d "${runDirectory}../${FinalProjectName}.app" ]; then
			rm -r "${runDirectory}../${FinalProjectName}.app"
		fi

		#Make the directory structure of the Mac OS X application package, then copy executable and data into package.
		mkdir -p "${runDirectory}../${FinalProjectName}.app/Contents/Resources/data"
		cp "../macos/${buildConfig}/${executableName}.app/Contents/MacOS/${executableName}" "${runDirectory}../${FinalProjectName}.app/Contents/MacOS/${FinalProjectName}"
		cp -r "${runDirectory}data/" "${runDirectory}../${FinalProjectName}.app/Contents/Resources/data/"
	fi
else
	echo "Unknown buildPlatform, please use --platform <tb_linux, tb_macos, etc>"
	return 404
fi
