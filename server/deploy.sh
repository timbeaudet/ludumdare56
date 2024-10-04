#!/bin/bash

#
# Deploy the files to server with a given IP address.
#
# <!-- Copyright (c) 2021 Tyre Bytes LLC - All Rights Reserved -->
#----------------------------------------------------------------------------------------------------------------------

#According to tsjost, this will force the script to exit when an error happens.
set -e

DIR=`dirname "$(readlink -f "$0")"`

SERVER_IP=""
SERVER_LOCATION="~/ludumdare56/run/"
BANDWITH_LIMIT=""

while true; do
	case "$1" in
		--help | -h ) FLAG_HELP=true; shift ;;
		--ip ) SERVER_IP="$2"; shift 2 ;;
		--limit ) BANDWITH_LIMIT="--bwlimit=$2"; shift 2 ;;
		* ) break ;;
	esac
done

if [[ -z "$SERVER_IP" ]]; then
	echo "Error failed to upload the server to unknown ip."
else
	echo "Deploying Trophy Brawlers to server at $SERVER_IP"

	rsync -zavhiP ${BANDWITH_LIMIT} $DIR/../run/data ${SERVER_IP}:${SERVER_LOCATION}
	rsync -zavhiP ${BANDWITH_LIMIT} $DIR/../server/game_server_start.sh ${SERVER_IP}:"~/"

	if [[ -f $DIR/../build/linux/release/ludumdare56_server ]]; then
		rsync -zavhiP ${BANDWITH_LIMIT} $DIR/../build/linux/release/ludumdare56_server ${SERVER_IP}:${SERVER_LOCATION}"ludumdare56_server_release"
	fi
	if [[ -f $DIR/../build/linux/public/ludumdare56_server ]]; then
		rsync -zavhiP ${BANDWITH_LIMIT} $DIR/../build/linux/public/ludumdare56_server ${SERVER_IP}:${SERVER_LOCATION}"ludumdare56_server"
	fi

	#rsync -avhiP ${BANDWITH_LIMIT} $DIR/../server/secrets/ ${SERVER_IP}:${SERVER_LOCATION}/data/secrets
	#rsync -avhiP ${BANDWITH_LIMIT} $DIR/../build/linux/debug/ludumdare56_server ${SERVER_IP}:${SERVER_LOCATION}"_debug"
fi
