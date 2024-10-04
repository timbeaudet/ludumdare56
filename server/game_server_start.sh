#!/bin/bash

#
# A script to place on a GameServer to get the server started.
#
# <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
#-----------------------------------------------------------------------------------------------------------------------

echo "Starting the Game Server."

cd ~/ludumdare56/run/
if [[ -f ./ludumdare56_server ]]; then
	./ludumdare56_server
elif [[ -f ./ludumdare56_server_release ]]; then
	./ludumdare56_server_release
else
	echo "Did not start a Trophy Brawlers Game Server, was not uploaded?"
fi
