#!/usr/bin/env bash

#
# Simple batch script to create an GCC make files and XCode workspaces using premake5.
#
# <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
#-----------------------------------------------------------------------------------------------------------------------

PROJECT_NAME=ludumdare56

# 2024-08-03: Everything below this point is directly copied in each make_project.bat unless otherwise noted or this
#   comment gets removed. The only thing that changes is the PROJECT_NAME variable above, and we should attempt to
#   keep the rest of it that generic as it makes it easier to maintain. Perhaps a project or two will need specific
#   edits or changes, but lets ensure there is good reason to.
# ----------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------------------------

FLAG_CLEAN=false
FLAG_RUN=false
FLAG_BUILD=false
FLAG_BUILD_ALL=true
FLAG_BUILD_DEBUG=false
FLAG_BUILD_RELEASE=false
FLAG_BUILD_PUBLIC=false
FLAG_MACOS=false
FLAG_LINUX=false
FLAG_WEB=false
FLAG_MAKE_JOBS=$(nproc)

BUILD_VERSION=$(cat version.txt)
if [ -z ${BUILD_VERSION} ]; then
	BUILD_VERSION=0.0.0
fi

BUILD_REPORT_FILE=/dev/stdout
if [ ! -z ${abs_detailed_report_file+x} ]; then
	BUILD_REPORT_FILE=${abs_detailed_report_file}
fi

POSITIONAL=()
while [[ $# -gt 0 ]]; do
	case "$1" in
		--clean ) FLAG_CLEAN=true; shift ;;
		--build ) FLAG_BUILD=true; shift ;;
		--run ) FLAG_RUN=true; shift ;;
		--macos ) FLAG_MACOS=true; FLAG_BUILD=true; shift ;;
		--linux ) FLAG_LINUX=true; FLAG_BUILD=true; shift ;;
		--web ) FLAG_WEB=true; FLAG_BUILD=true; shift ;;
		--build-version ) BUILD_VERSION="$2"; shift 2 ;;
		-d | --debug ) FLAG_BUILD_DEBUG=true; FLAG_BUILD_ALL=false; FLAG_BUILD=true; shift ;;
		-r | --release ) FLAG_BUILD_RELEASE=true; FLAG_BUILD_ALL=false; FLAG_BUILD=true; shift ;;
		-p | --public ) FLAG_BUILD_PUBLIC=true; FLAG_BUILD_ALL=false; FLAG_BUILD=true; shift ;;
		-j ) FLAG_MAKE_JOBS="$2"; shift 2 ;;
		-* ) echo "Unknown parameter \"$1\""; exit 2 ;;
		* ) POSITIONAL+=("$1"); shift ;;
	esac
done

if [[ "$FLAG_BUILD" == true ]] && [[ "$FLAG_BUILD_ALL" == true ]]; then
	FLAG_BUILD_DEBUG=true
	FLAG_BUILD_RELEASE=true
	FLAG_BUILD_PUBLIC=true
fi

if [[ "$FLAG_WEB" == false ]]; then #AND
if [[ "$FLAG_LINUX" == false ]]; then #AND
if [[ "$FLAG_MACOS" == false ]]; then
	kLinuxPlatform="Linux"
	currentPlatform=`uname`
	if [ $kLinuxPlatform == $currentPlatform ]; then
		FLAG_LINUX=true
	else
		FLAG_MACOS=true
	fi
fi
fi
fi

# Check to ensure the directory at least exists, if not forcefully clean.
#   Unfortunately the premake script will forcefully clean ALL targets if any one directory is missing including those
#   platforms that may already be partially built.
if [[ "$FLAG_LINUX" == true ]] && [[ ! -d "./linux/" ]]; then
	FLAG_CLEAN=true
elif [[ "$FLAG_WEB" == true ]] && [[ ! -d "./web/" ]]; then
	FLAG_CLEAN=true
elif [[ "$FLAG_MACOS" == true ]] && [[ ! -d "./macos/" ]]; then
	FLAG_CLEAN=true
fi

# ----------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------------------------
# ----------------------------------------------------------------------------------------------------------------------

if [[ "$FLAG_CLEAN" == true ]]; then
	echo "performing a clean build"
	premake5 --file="${PROJECT_NAME}.lua" --build-version=$BUILD_VERSION clean
fi

premake5 --file="${PROJECT_NAME}.lua" --build-version=$BUILD_VERSION gmake
premake5 --file="${PROJECT_NAME}.lua" --build-version=$BUILD_VERSION xcode4
premake5 --file="${PROJECT_NAME}.lua" --build-version=$BUILD_VERSION --os=macosx gmake
premake5 --file="${PROJECT_NAME}.lua" --build-version=$BUILD_VERSION --web="true" gmake

if [[ "$FLAG_WEB" == true ]]; then
	source ~/development/tools/linux/emsdk-portable/emsdk_env.sh

	cd web
	if [[ "$FLAG_BUILD_DEBUG" == true ]]; then
		emmake make -j $FLAG_MAKE_JOBS config=debug 2>> "$BUILD_REPORT_FILE"

		# 2024-08-07 Everything below ---THIS LINE HERE--- was from LudumDare56 (a game / executable project),
		#   which might not apply to the above which originated from TurtleBrains (a library / framework project).
		#
		# 2024-08-03: As I was changing/adding BUILD_CONFIG_FLAGs I found this a little more concerning. Since web builds
		#   are not my current priority I'm disabling the debug version until the problem is solved. It may be worth
		#   looking into release and public versions here too, but debug was using the /release/project.html file which
		#   seems TERRIBLE. Maybe if it were build/project.html would be okay, but /release/ stuff is only built with
		#   the release build and debug shouldn't require a release build also (if it does we need to set the flag and
		#   change the order of checks).
		echo Note: Debug version of web-builds are presently disabled. There was a not claiming they had issues.
		echo Come back and fix the issue!

		# emmake make -j $FLAG_MAKE_JOBS config=debug
		# #emcc debug/objects/**.o -g2 -s LINKABLE=1 -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2 -o debug/ludumdare56.html --preload-file ../../run/data@/data --shell-file ../web_shell.html
		# emcc debug/objects/**.o -O1 -s LINKABLE=1 -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2 -o release/ludumdare56.html --preload-file ../../run/data@/data --shell-file ../web_shell.html
		# echo Note: Debug version of web-builds have always seemed to have issues. So using release with log output.
		# echo Running sed magic from debug area.
		# sed -Ei 's/^.*?src="(.*?\.js)".*?$/script.src="\1";/' debug/ludumdare56.html
	fi

	if [[ "$FLAG_BUILD_RELEASE" == true ]]; then
		emmake make -j $FLAG_MAKE_JOBS config=release 2>> "$BUILD_REPORT_FILE"

		# 2024-08-07 Everything below ---THIS LINE HERE--- was from LudumDare56 (a game / executable project),
		#   which might not apply to the above which originated from TurtleBrains (a library / framework project).

		# emmake make -j $FLAG_MAKE_JOBS config=release
		# emcc release/objects/**.o -O1 -s LINKABLE=1 -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=2 -o release/ludumdare56.html --preload-file ../../run/data@/data --shell-file ../web_shell.html
		# echo Running sed magic from release area.
		# sed -Ei 's/^.*?src="(.*?\.js)".*?$/script.src="\1";/' release/ludumdare56.html
	fi

	if [[ "$FLAG_BUILD_PUBLIC" == true ]]; then
		emmake make -j $FLAG_MAKE_JOBS config=public 2>> "$BUILD_REPORT_FILE"
	fi
	cd ../
fi

if [[ "$FLAG_LINUX" == true ]]; then
	cd linux

	FLAG_RUN_CONFIG=
	if [[ "$FLAG_BUILD_DEBUG" == true ]]; then
		make -j $FLAG_MAKE_JOBS config=debug 2>> "$BUILD_REPORT_FILE"
		[ "$?" == 0 ] && BUILD_SUCCESS=true || BUILD_SUCCESS=false
		FLAG_RUN_CONFIG=debug
	fi
	if [[ "$FLAG_BUILD_RELEASE" == true ]]; then
		make -j $FLAG_MAKE_JOBS config=release 2>> "$BUILD_REPORT_FILE"
		[ "$?" == 0 ] && BUILD_SUCCESS=true || BUILD_SUCCESS=false
		FLAG_RUN_CONFIG=release
	fi
	if [[ "$FLAG_BUILD_PUBLIC" == true ]]; then
		make -j $FLAG_MAKE_JOBS config=public 2>> "$BUILD_REPORT_FILE"
		[ "$?" == 0 ] && BUILD_SUCCESS=true || BUILD_SUCCESS=false
		FLAG_RUN_CONFIG=public
	fi
	cd ../

	if [[ "$BUILD_SUCCESS" = false ]]; then
		if [ ! -z ${abs_detailed_report_file+x} ]; then
			return 1
		fi
	elif [[ "$FLAG_RUN" = true ]]; then
		(cd ../run/ && ./${PROJECT_NAME}_linux_${FLAG_RUN_CONFIG})
	fi
fi

if [[ "$FLAG_MACOS" = true ]]; then
	if [ $kLinuxPlatform == $currentPlatform ]; then
		echo "Attempting to use crossmac to build macOS from Linux"

		if [[ "$FLAG_CLEAN" = true ]]; then
			premake5 --file="${PROJECT_NAME}.lua" --os=macosx --build-version=$BUILD_VERSION clean
			premake5 --file="${PROJECT_NAME}.lua" --os=macosx --build-version=$BUILD_VERSION gmake
		fi
		sed -i 's/\(CC\|CXX\|AR\) = \(clang\|clang++\|ar\)/\1 = x86_64-apple-darwin15-\2/' macos/${PROJECT_NAME}.make

		if [[ "$FLAG_BUILD_DEBUG" == true ]]; then
			sed -i 's/\(CC\|CXX\|AR\) = \(clang\|clang++\|ar\)/\1 = x86_64-apple-darwin15-\2/' macos/${PROJECT_NAME}.make
			make -C macos config=debug -j $FLAG_MAKE_JOBS
			x86_64-apple-darwin15-strip macos/debug/$PROJECT_NAME
		fi

		if [[ "$FLAG_BUILD_RELEASE" == true ]]; then
			sed -i 's/\(CC\|CXX\|AR\) = \(clang\|clang++\|ar\)/\1 = x86_64-apple-darwin15-\2/' macos/${PROJECT_NAME}.make
			make -C macos config=release -j $FLAG_MAKE_JOBS
			x86_64-apple-darwin15-strip macos/release/$PROJECT_NAME
		fi

		if [[ "$FLAG_BUILD_PUBLIC" == true ]]; then
			sed -i 's/\(CC\|CXX\|AR\) = \(clang\|clang++\|ar\)/\1 = x86_64-apple-darwin15-\2/' macos/${PROJECT_NAME}.make
			make -C macos config=public -j $FLAG_MAKE_JOBS
			x86_64-apple-darwin15-strip macos/public/$PROJECT_NAME
		fi
	elif [[ "$FLAG_BUILD" == true ]]; then
		echo "Currently unable to build MacOS project in an automated manner due to"
		echo "premake5 and XCode with executable builds like games. May need to create default/project Info.plist"
		echo "or something similar in order to fix the issue."
		echo "    Open project > Build Settings > search "Generate Info.plist" and set to true"

		# 2024-08-07: First I finally found a way to supress info.plist issues by specifying one through premake which
		#   is happening now with a bunch of junk data. It appears from StackOverflow that this plist is going away or
		#   has already disappeared? That said we still need to manually choose the right scheme of a workspace, this
		#   appears random, and still set the working directory to /run/ to run from inside XCode.
		#
		#   In other news, I have successfully built debug using the following command and seen release build start
		#   using the command below it. I'm not sure if the debug version succeeded because I had already opened in
		#   XCode, but the release failed. This is where I've left off. If we can get these working then it may be
		#   possible to setup a nighties job on mac? Seems crontab and sendmail are doing SOMETHING here so perhaps
		#   it wouldn't be so hard to figure out.
		#
		#   xcodebuild -workspace ./macos/ludumdare56.xcworkspace -scheme ludumdare56 build
		#   xcodebuild -workspace ./macos/ludumdare56.xcworkspace -scheme ludumdare56 -configuration release build

		if [[ "$FLAG_BUILD_DEBUG" == true ]]; then
			xcodebuild -workspace ./macos/${PROJECT_NAME}.xcworkspace -scheme ${PROJECT_NAME} -configuration debug build 2>> "$BUILD_REPORT_FILE"
		fi

		if [[ "$FLAG_BUILD_RELEASE" == true ]]; then
			xcodebuild -workspace ./macos/${PROJECT_NAME}.xcworkspace -scheme ${PROJECT_NAME} -configuration release build 2>> "$BUILD_REPORT_FILE"
		fi

		if [[ "$FLAG_BUILD_PUBLIC" == true ]]; then
			xcodebuild -workspace ./macos/${PROJECT_NAME}.xcworkspace -scheme ${PROJECT_NAME} -configuration public build 2>> "$BUILD_REPORT_FILE"
		fi
	fi
fi
