#!/bin/bash

#
# Create a GameServers and upload all the files so the drivers can compete together.
#
# <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
#-----------------------------------------------------------------------------------------------------------------------

DIR=`dirname "$(readlink -f "$0")"`

# Exit script when errors happen
set -e

#LINODE_CONFIG_FILE=~/.linodecli/config
LINODE_CONFIG_FILE=~/.config/linode-cli
SSH_KEY_FILE=~/.ssh/id_rsa.pub

FLAG_CONFIG=false
DATACENTER=false
BANDWITH_LIMIT=

while true; do
	case "$1" in
		--config ) FLAG_CONFIG=true; shift ;;
		--datacenter ) DATACENTER="$2"; shift 2 ;;
		--limit ) BANDWITH_LIMIT="--limit $2"; shift 2 ;;
		* ) break ;;
	esac
done

function create_game_server {
	SERVER_LABEL="LudumDare56_`date +%Y-%m-%d_%H-%M`"
	echo "--> Creating Game Server \"$SERVER_LABEL\"..."
	linode-cli linodes create --label $SERVER_LABEL --authorized_keys "$(cat $SSH_KEY_FILE)" --root_pass `< /dev/urandom tr -dc _A-Z-a-z-0-9 | head -c${1:-128}`

	echo "--> Fetching server info..."
	SERVER_IP=`linode-cli linodes list --label $SERVER_LABEL --json | jq -r .[0].ipv4[0]`

	echo "--> Server IP address is $SERVER_IP"
}

function provision_game_server {
	# 2024-09-03: Once in a while the script would get stuck here and require re-creating a new gameserver. It seems
	#   there is another way to accept the new ssh host information with -o StrictHostKeyChecking=accept-new which is
	#   the purpose this serving. That option is in the ansible.cfg and appears to be running without issues... yet.
	#
	#   This did eventually complain when a new gameserver had a matching IP in known_hosts, but the key information
	#   didn't match (due to old game server). Since we are recreatinga new game server with the IP we have no other
	#   choice but to trust it. I can't think of any negative implications, since SERVER_IP is the address of our new
	#   game server and should be trustable?
	#echo "--> Grabbing SSH keys from server..."
	#ssh-keyscan $SERVER_IP >> ~/.ssh/known_hosts 2>/dev/null

	ssh-keygen -R $SERVER_IP

	echo "--> Setting up Ansible..."
	echo "$SERVER_IP ansible_user=root ansible_python_interpreter=/usr/bin/python3" > $DIR/ansible/inventory

	echo "--> Running Ansible..."
	# export ANSIBLE_DEBUG=1
	export ANSIBLE_CONFIG=$DIR/ansible/ansible.cfg
	ansible-playbook -v -i $DIR/ansible/inventory $DIR/ansible/gameserver.yml
}

function install_linode_cli {
	echo "--> Adding Linode's APT repo..."
	sudo bash -c 'echo "deb http://apt.linode.com/ $(lsb_release -cs) main" > /etc/apt/sources.list.d/linode.list'

	echo "--> Adding Linode's GPG key for cryptographically verifying packages..."
	wget -qO- https://apt.linode.com/linode.gpg | sudo apt-key add -

	echo "--> Updating APT repositories..."
	sudo apt -q update

	echo "--> Installing Linode CLI..."
	sudo apt -yq install linode-cli
}

function configure_linode_cli {
	echo -e "No Linode configuration found!\nFollow these instructions:\n"
	echo -e "Browse to:\n https://manager.linode.com/profile/api"
	echo -e "and create a new API key.\nPlease paste the generated API key here (it will be hidden):"

	read -s LINODE_APIKEY

	mkdir -p `dirname $LINODE_CONFIG_FILE`
	cat <<EOF > $LINODE_CONFIG_FILE
distribution Ubuntu 18.04 LTS
datacenter frankfurt
plan Nanode 1GB
api-key $LINODE_APIKEY
EOF
	chmod 0600 $LINODE_CONFIG_FILE
}

###
### The program begins here!
###
cat <<'EOF'

  ___                         _       _                  _   _
 / __|___ _ ___ _______ _    /_\ _  _| |_ ___ _ __  __ _| |_(_)___ _ _
 \__ / -_| '_\ V / -_| '_|  / _ | || |  _/ _ | '  \/ _` |  _| / _ | ' \
 |___\___|_|  \_/\___|_|   /_/ \_\_,_|\__\___|_|_|_\__,_|\__|_\___|_||_|

EOF

# if [ ! -x "$(command -v linode)" ]; then
# 	echo "Linode CLI not installed, installing now..."
# 	install_linode_cli
# 	echo ""
# fi

if [ ! -x "$(command -v jq)" ]; then
	echo "JSON parser jq not installed, installing now..."
	sudo apt -yq install jq
	echo ""
fi

if [ ! -x "$(command -v ansible)" ]; then
	echo "Ansible not installed, installing now..."
	sudo apt -yq install ansible
	echo ""
fi

if [[ "$FLAG_CONFIG" = true || ! -f $LINODE_CONFIG_FILE ]]; then
	configure_linode_cli
	echo ""
fi

if [ ! -f $SSH_KEY_FILE ]; then
	echo -e "ERROR: No SSH key found!\nPlease generate one with \`ssh-keygen' now."
	exit 1
fi

if [ ! $DATACENTER = false ]; then
	sed -ri "s/^(datacenter).*$/\1 $DATACENTER/" $LINODE_CONFIG_FILE
fi

create_game_server
echo "--> Waiting for server to become reachable..."
until ping -c1 $SERVER_IP &>/dev/null; do sleep 1; done
provision_game_server

deploySuccess=false
for attempt in { 1..5 }
do
	$DIR/deploy.sh --ip $SERVER_IP ${BANDWITH_LIMIT}
	if [[ $? == 0 ]]; then
		deploySuccess=true
		break
	fi
done

if [[ true == $deploySuccess ]]; then
	$DIR/start_server.sh --ip $SERVER_IP

	echo "--> Starting game server"
	#ssh $SERVER_IP "screen -d -m bash -c \"cd ~/ludumdare56/run/ && ./ludumdare56_server\""

	ssh $SERVER_IP "screen -d -m bash -c \"cd ~ && bash ./game_server_start.sh\""


	echo -e "\nGame server now running at $SERVER_IP\n"
	echo -e "To access the server:\n$ ssh $SERVER_IP\n$ screen -r\n"
else
	echo "FAILURE: GAME SERVER NOT STARTED!!"
	echo "Failed to deploy the files to the game server."
	echo "Run deploy.sh --ip $SERVER_IP to deploy files."
	echo "Run start_server.sh --ip $SERVER_IP to start game server."
	echo -e "\nFAILURE: GAME SERVER NOT STARTED!!\n"
fi
