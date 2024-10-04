#!/bin/bash

DIR=`dirname "$(readlink -f "$0")"`

# Exit script when errors happen
set -e

function delete_server {
	echo "--> Deleting server $1..."
	#linode delete --label $1
	DELETED_SERVER_ID="$(linode-cli linodes list --text | grep "\b${1}\b" | awk '{print $1}')"
	echo "--> Deleting server by ID $DELETED_SERVER_ID..."
	linode-cli linodes delete $DELETED_SERVER_ID
}

cat <<'EOF'

  ___                         _       _                  _   _
 / __|___ _ ___ _______ _    /_\ _  _| |_ ___ _ __  __ _| |_(_)___ _ _
 \__ / -_| '_\ V / -_| '_|  / _ | || |  _/ _ | '  \/ _` |  _| / _ | ' \
 |___\___|_|  \_/\___|_|   /_/ \_\_,_|\__\___|_|_|_\__,_|\__|_\___|_||_|

EOF

echo "--> Fetching list of servers..."
SERVER_LABELS=`linode-cli linodes list --json | jq -r '.[].label'`

if [[ "$SERVER_LABELS" = "null" ]]; then
	echo "No servers running."
elif [[ $(echo "$SERVER_LABELS" | wc -l) = 1 ]]; then
	delete_server $SERVER_LABELS
	$DIR/stop_server.sh
	echo "--> Fake "MasterServer" notified GameServer closed."
else
	mapfile -t SERVER_LABELS_ARRAY < <(echo "$SERVER_LABELS")

	echo "There are multiple servers running."
	for i in "${!SERVER_LABELS_ARRAY[@]}"; do
		printf " %s) %s\n" "$i" "${SERVER_LABELS_ARRAY[$i]}"
	done

	echo -n "Pick which one to delete: "
	read opt

	if [[ $opt =~ ^[0-9]+$ ]] && (( (opt >= 0) && (opt < "${#SERVER_LABELS_ARRAY[@]}") )); then
		delete_server ${SERVER_LABELS_ARRAY[$opt]}

		read -p "Do you also want to set server_running to false? [Y/n] " response
		if [[ $response =~ ^[Yy]?$ ]]; then
			$DIR/stop_server.sh
		fi
	else
		echo "Invalid option."
	fi
fi
