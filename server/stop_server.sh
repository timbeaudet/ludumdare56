#!/bin/bash

#
# Start the server with a given IP address.
#
# <!-- Copyright (c) 2021 Tyre Bytes LLC - All Rights Reserved -->
#----------------------------------------------------------------------------------------------------------------------

echo "--> Setting server_running to false"

ssh timbeaudet.com 'echo "{ \
\"server_running\": false \
}" > "/home/timbeaudet/tyrebytes.com/t/ludumdare56_status.json"'

echo "--> Finished setting server_running to false"
