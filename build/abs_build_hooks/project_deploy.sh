#!/bin/bash

#
# Simple batch script hooked into the Automated Build System to run project specific deploy commands. If an error
# occurs that should halt the build and fail the nightly; set an error-code above 10 in the variable abs_return_value
#   abs_return_value=11
# 
# To log any details to the email report, use printf like so:
#   printf "Here are some details about the custom step." >> "$abs_detailed_report_file"
# -------------------------------------------------------------------------------------------------------------------

web_release=./web/release

if [[ -d $web_release ]]; then
	echo "Performing: Automated Deploy."
	pushd $web_release > /dev/null

	TBDOTCOM_SERVERPATH="timbeaudet@timbeaudet.com"
	TBDOTCOM_BUILD_LOCATION="tyrebytes.com/games/ludumdare56/live/"

	rm -rf ./objects #exclude is not working, and rebuild for automation stuff anyway.
	rsync -a --relative --bwlimit=200 --progress ./ ${TBDOTCOM_SERVERPATH}:${TBDOTCOM_BUILD_LOCATION}
	#rsync -a --relative --bwlimit=100 --exclude ./objects ./ ${TBDOTCOM_SERVERPATH}:${TBDOTCOM_BUILD_LOCATION}
	pkill -USR1 -f irc.py

	popd > /dev/null
	echo "Completed the Automated Deploy Process."
fi
