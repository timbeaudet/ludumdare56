#!/bin/bash

# More documentation on how to use this is found at:
# https://github.com/timbeaudet/knowledge_base/blob/master/processes/deploy_web_live.md

cd "$(dirname "$0")"
echo Running AutomationTask from: `pwd`

#Hack this is obviously dependent on my system and tool location...
#The following directory include tools like premake5 required for
#the tool chain of "automate_builds.sh" from https://github.com/timbeaudet/build_automation
export PATH=$PATH:/home/timbeaudet/development/tools/linux

#Need to go back to project root, so that the automate_builds script will 
#find /automated/ and auto_* scripts to run. This pushd directory path must
#be the location of the project to automate.
pushd /home/timbeaudet/development/personal/ludumdare56/ > /dev/null
/home/timbeaudet/development/tools/linux/automate_builds.sh
popd > /dev/null
