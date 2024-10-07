#!/bin/bash

ITCHIO_PROJECT_NAME=timbeaudet/ludumdare56
ITCHIO_RELEASE_CHANNEL=alpha
TBDOTCOM_SERVERPATH=timbeaudet@timbeaudet.com
TBDOTCOM_BUILD_LOCATION=tyrebytes.com/t/ludumdare56/
#TBDOTCOM_BUILD_LOCATION=timbeaudet.com/goods/ludumdare/ldXY/
BANDWIDTH_LIMIT_KBPS=1400

SHOULD_LIMIT_BANDWIDTH_HACK=false
BANDWIDTH_LIMIT_INTERFACE=enp2s0

FLAG_HELP=false
FLAG_CLEAN=true
FLAG_BUILD_CONFIG=public
FLAG_MAKE_JOBS=$(nproc)
FLAG_LINUX=false
FLAG_MACOS=false
FLAG_WINDOWS=false
FLAG_OFFLINE=false
FLAG_SKIP_ITCH=true
BUILD_ALL_PLATFORMS=true

POSITIONAL=()
while [[ $# -gt 0 ]]; do
	case "$1" in
		--help ) FLAG_HELP=true; shift ;;
		--clean ) FLAG_CLEAN=true; shift ;;
		--dirty ) FLAG_CLEAN=false; shift ;;
		-d | --debug ) FLAG_BUILD_CONFIG=debug; shift ;;
		-r | --release ) FLAG_BUILD_CONFIG=release; shift ;;
		-p | --public ) FLAG_BUILD_CONFIG=public; shift ;;
		-j ) FLAG_MAKE_JOBS="$2"; shift 2 ;;
		--linux ) FLAG_LINUX=true; BUILD_ALL_PLATFORMS=false; shift ;;
		--macos ) FLAG_MACOS=true; BUILD_ALL_PLATFORMS=false; shift ;;
		--windows ) FLAG_WINDOWS=true; BUILD_ALL_PLATFORMS=false; shift ;;
		--offline ) FLAG_OFFLINE=true; shift ;;
		--limit ) SHOULD_LIMIT_BANDWIDTH_HACK=true; BANDWIDTH_LIMIT_KBPS="$2"; shift 2 ;;
		--skip-itch ) FLAG_SKIP_ITCH=true; shift ;;
		--itch ) FLAG_SKIP_ITCH=false; shift ;;
		-* ) echo "Unknown parameter \"$1\""; exit 2 ;;
		* ) POSITIONAL+=("$1"); shift ;;
	esac
done

DIR=`dirname "$(readlink -f "$0")"`

# echo "${POSITIONAL[*]}"

# 2024-09-03: No longer specifying the version through build_and_distribute script calls, use the version.txt file and
#   keep that committed to source control as the builds are created.
#BUILD_VERSION=${POSITIONAL[0]}
BUILD_VERSION=$(cat version.txt)
if [ -z ${BUILD_VERSION} ]; then
	BUILD_VERSION=0.0.0
fi

print_usage() {
	cat <<EOF
Usage:
  $0 <version string> [flags]

Flags:
  --help        show this help
  --dirty       skip the clean build
  --offline     perform a dry run without uploading
  -d, --debug   build in debug mode
  -r, --release build in release mode
  -p, --public  build in public mode (default if -d, -r and -p not specified)
  -j [N]        build with N concurrent jobs
  --skip-itch   skips the itch.io deployment (default is project dependent)
  --itch        also deploy to itch.io (default is project dependent)
  --limit [N]   limit upload speed to N kb/s

If none of these flags are specified, all of them will apply:
  --linux       build for Linux
  --macos       build for MacOS
  --windows     distribute for Windows
EOF
}

build_mac() (
	set -e
	echo "--> Building for MacOS"

#	For some reason, the following does not work, it seems to give fatal errors string not found
#	Much like the crossmac can't find standard headers or something?
#	[ $FLAG_BUILD_CONFIG == "debug" ] && DEBUG="--debug" || DEBUG=""
#	[ $FLAG_CLEAN == true ] && CLEAN="--clean" || CLEAN=""
#	$DIR/make_project.sh --macos --build-version $BUILD_VERSION -j $FLAG_MAKE_JOBS $DEBUG $CLEAN

	if [[ $FLAG_CLEAN = true ]] || [[ ! -f $DIR/macos/ludumdare56.make ]]; then
		if [[ $FLAG_CLEAN = true ]]; then
			premake5 --file=$DIR/ludumdare56.lua --os=macosx --build-version=$BUILD_VERSION clean
		fi
		premake5 --file=$DIR/ludumdare56.lua --os=macosx --build-version=$BUILD_VERSION gmake
	fi

	sed -i 's/\(CC\|CXX\|AR\) = \(clang\|clang++\|ar\)/\1 = x86_64-apple-darwin15-\2/' $DIR/macos/ludumdare56.make

	make -C $DIR/macos config=$FLAG_BUILD_CONFIG -j $FLAG_MAKE_JOBS
	x86_64-apple-darwin15-strip $DIR/macos/$FLAG_BUILD_CONFIG/ludumdare56

	echo "--> Moving files to distribution/"
	mkdir -p $DIR/../distribution/macos/LudumDare56.app/Contents/MacOS/
	rsync -avhiP $DIR/macos/$FLAG_BUILD_CONFIG/ludumdare56 $DIR/../distribution/macos/LudumDare56.app/Contents/MacOS/LudumDare56
	rsync -avhiP $DIR/../run/data $DIR/../distribution/macos/LudumDare56.app/Contents/Resources/ --exclude ".git"
)

build_linux() (
	set -e
	echo "--> Building for Linux"

	[ $FLAG_CLEAN == true ] && CLEAN="--clean" || CLEAN=""
	$DIR/make_project.sh --linux --build-version $BUILD_VERSION -j $FLAG_MAKE_JOBS --$FLAG_BUILD_CONFIG $CLEAN
	strip $DIR/linux/$FLAG_BUILD_CONFIG/ludumdare56

	echo "--> Moving files to distribution/"
	mkdir -p $DIR/../distribution/linux/ludumdare56/
	rsync -avhiP $DIR/linux/$FLAG_BUILD_CONFIG/ludumdare56 $DIR/../distribution/linux/ludumdare56/
	rsync -avhiP $DIR/../run/data $DIR/../run/readme.txt $DIR/../distribution/linux/ludumdare56/ --exclude ".git"
)

###
### stuff starts here!
###

if [[ $BUILD_VERSION = "" ]] || [[ $FLAG_HELP = true ]]; then
	print_usage
	exit 1
fi

echo "--> Creating builds for version \"$BUILD_VERSION\""

if [[ $BUILD_ALL_PLATFORMS = true ]] || [[ $FLAG_MACOS = true ]]; then
	build_mac
	BUILD_STATUS_MAC=$?
	echo $BUILD_STATUS_MAC
fi

if [[ $BUILD_ALL_PLATFORMS = true ]] || [[ $FLAG_LINUX = true ]]; then
	build_linux
	BUILD_STATUS_LINUX=$?
	echo $BUILD_STATUS_LINUX
fi

GAME_BUILDS_DIR=/mnt/raid1/shelter/development/game_builds/
if [[ $BUILD_ALL_PLATFORMS = true ]] || [[ $FLAG_WINDOWS = true ]]; then
	BUILD_STATUS_WINDOWS=1
	if [[ -f "${GAME_BUILDS_DIR}/ludumdare56/${BUILD_VERSION}/ludumdare56_${BUILD_VERSION}_windows.zip" ]]; then
		BUILD_STATUS_WINDOWS=0
	fi
	echo $BUILD_STATUS_WINDOWS
fi

if [[ $FLAG_OFFLINE == true ]]; then
	echo "Not uploading because --offline was specified."
else
	echo "--> Distributing packages"

	if [[ $SHOULD_LIMIT_BANDWIDTH_HACK == true ]]; then
		echo "Attempting to limit bandwidth, needs sudo password."
		sudo tc qdisc add dev $BANDWIDTH_LIMIT_INTERFACE handle 1: root htb default 11
		sudo tc class add dev $BANDWIDTH_LIMIT_INTERFACE parent 1: classid 1:1 htb rate 1000Mbps
		sudo tc class add dev $BANDWIDTH_LIMIT_INTERFACE parent 1:1 classid 1:11 htb rate ${BANDWIDTH_LIMIT_KBPS}kbit
	fi

	if [[ $BUILD_STATUS_WINDOWS == 0 ]]; then
		echo "--> Distributing Windows package"

		ZIP_FILENAME=${GAME_BUILDS_DIR}ludumdare56/${BUILD_VERSION}/ludumdare56_${BUILD_VERSION}_windows.zip
		#INSTALLER_FILENAME=${GAME_BUILDS_DIR}ludumdare56/${BUILD_VERSION}/LudumDare56Installer_${BUILD_VERSION}.exe

		if [[ $FLAG_SKIP_ITCH == false ]]; then
			[ $FLAG_BUILD_CONFIG == "debug" ] && DEBUG="-debug" || DEBUG=""
			butler push --userversion $BUILD_VERSION $ZIP_FILENAME ${ITCHIO_PROJECT_NAME}:windows${DEBUG}-${ITCHIO_RELEASE_CHANNEL}
		fi

		rsync --archive --verbose --human-readable --itemize-changes --partial --progress --bwlimit=$BANDWIDTH_LIMIT_KBPS $ZIP_FILENAME $TBDOTCOM_SERVERPATH:$TBDOTCOM_BUILD_LOCATION
		if [[ -f "$INSTALLER_FILENAME" ]]; then
			rsync --archive --verbose --human-readable --itemize-changes --partial --progress --bwlimit=$BANDWIDTH_LIMIT_KBPS $INSTALLER_FILENAME $TBDOTCOM_SERVERPATH:$TBDOTCOM_BUILD_LOCATION
		fi
	fi

	if [[ $BUILD_STATUS_LINUX == 0 ]]; then
		echo "--> Distributing Linux package"

		TAR_FILENAME=ludumdare56_${BUILD_VERSION}_linux.tar.gz

		(cd $DIR/../distribution/linux && tar czf ../$TAR_FILENAME ludumdare56)

		if [[ $FLAG_SKIP_ITCH == false ]]; then
			[ $FLAG_BUILD_CONFIG == "debug" ] && DEBUG="-debug" || DEBUG=""
			butler push --userversion $BUILD_VERSION $DIR/../distribution/$TAR_FILENAME ${ITCHIO_PROJECT_NAME}:linux64${DEBUG}-${ITCHIO_RELEASE_CHANNEL}
		fi

		rsync --archive --verbose --human-readable --itemize-changes --partial --progress --bwlimit=$BANDWIDTH_LIMIT_KBPS $DIR/../distribution/$TAR_FILENAME $TBDOTCOM_SERVERPATH:$TBDOTCOM_BUILD_LOCATION
		rsync --archive --verbose --human-readable --itemize-changes --partial --progress $DIR/../distribution/$TAR_FILENAME ${GAME_BUILDS_DIR}ludumdare56/${BUILD_VERSION}/$TAR_FILENAME
	fi

	if [[ $BUILD_STATUS_MAC == 0 ]]; then
		echo "--> Distributing MacOS package"

		(cd $DIR/../distribution/macos && zip -r9 ../ludumdare56_${BUILD_VERSION}_macos.zip .)

		if [[ $FLAG_SKIP_ITCH == false ]]; then
			[ $FLAG_BUILD_CONFIG == "debug" ] && DEBUG="-debug" || DEBUG=""
			# Tell Butler to use the .app directory directly, because it will create a
			# zip itself and also a patch for the Itch desktop app users
			# (I don't know if patch stuff works if you upload your own .zip)
			butler push --userversion $BUILD_VERSION $DIR/../distribution/macos/ ${ITCHIO_PROJECT_NAME}:macos64${DEBUG}-${ITCHIO_RELEASE_CHANNEL}
		fi

		rsync --archive --verbose --human-readable --itemize-changes --partial --progress --bwlimit=$BANDWIDTH_LIMIT_KBPS $DIR/../distribution/ludumdare56_${BUILD_VERSION}_macos.zip $TBDOTCOM_SERVERPATH:$TBDOTCOM_BUILD_LOCATION
		rsync --archive --verbose --human-readable --itemize-changes --partial --progress $DIR/../distribution/$MACOS_ZIP_FILENAME ${GAME_BUILDS_DIR}ludumdare56/${BUILD_VERSION}/$MACOS_ZIP_FILENAME
	fi

	if [[ $SHOULD_LIMIT_BANDWIDTH_HACK == true ]]; then
		echo "Attempting to stop bandwidth limit, needs sudo password."
		sudo tc qdisc del dev $BANDWIDTH_LIMIT_INTERFACE root
	fi
fi

echo "Build and distribute done!"
