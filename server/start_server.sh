#!/bin/bash

#
# Start the server with a given IP address.
#
# <!-- Copyright (c) 2021-2023 Tyre Bytes LLC - All Rights Reserved -->
#----------------------------------------------------------------------------------------------------------------------


FLAG_HELP=false
FLAG_DEPLOY=false
SERVER_IP=""
SERVER_PORT="45001"

while true; do
	case "$1" in
		--help | -h ) FLAG_HELP=true; shift ;;
		--deploy ) FLAG_DEPLOY=true; shift ;;
		--ip ) SERVER_IP="$2"; shift 2 ;;
		--port ) SERVER_PORT="$2"; shift 2 ;;
		* ) break ;;
	esac
done

if [[ "$FLAG_HELP" == true ]] || [[ -z "$SERVER_IP" ]]; then
	echo "To start the server use the following command:"
	echo "./start_server.sh --ip xxx.yyy.zzz.www to start the server."
	echo "   use --deploy to trigger deployment as well."
else

	if [[ "$FLAG_DEPLOY" == true ]]; then
		./deploy.sh --ip $SERVER_IP
	fi

echo "--> Setting server_running to true with IP $SERVER_IP"
ssh timbeaudet.com "echo '{ \
\"server_running\": true, \
\"server_ip\": \"$SERVER_IP\", \
\"server_port\": $SERVER_PORT \
}' > ~/tyrebytes.com/t/ludumdare56_status.json"
fi
