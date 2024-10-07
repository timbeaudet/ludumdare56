#!/bin/bash

#
# Simple batch script to grab TurtleBrains, InternalCombustion and TrackBuilder or other dependencies for the project
#   pulling them into build/externals/.
#
# <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
#----------------------------------------------------------------------------------------------------------------------

# Skip initialization of externals for automated builds unless the project needs specific commits for external dependencies.
# if [[ "$abs_automating_builds" == true ]]; then
# 	return 0
# fi

echo "Performing: Initialization / Update of Externals."
updated_externals=false

TURTLE_BRAINS_REVISION=main
INTERNAL_COMBUSTION_REVISION=main
TRACK_BUILDER_REVISION=main

#If there is not abs_detailed_report_file variable, use stdout to display report.
if [ -z ${abs_detailed_report_file+x} ]; then
	abs_detailed_report_file=/dev/stdout
fi

printf "\n\nupdating external repositories.\n" 2>&1 >> "$abs_detailed_report_file"
printf "========================================================\n" 2>&1 >> "$abs_detailed_report_file"

#---------------------------------------------------------- GIT: turtle_brains
if [[ -d externals/turtle_brains ]]; then
	# 2024-08-01: Would be nice to have a check that actually says there are updats or not, but lets just get things
	#   done by assuming it always updates for right now. This can be a future improvement.
	updated_externals=true

	echo "Updating TurtleBrains to: $TURTLE_BRAINS_REVISION"
	git -C "externals/turtle_brains/" fetch 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/turtle_brains/" pull --rebase 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/turtle_brains/" checkout "$TURTLE_BRAINS_REVISION" 2>&1 >> "$abs_detailed_report_file"
else
	echo "Initializing TurtleBrains to: $TURTLE_BRAINS_REVISION"
	git clone git@github.com:TyreBytes/turtle_brains.git "externals/turtle_brains/" 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/turtle_brains/" checkout "$TURTLE_BRAINS_REVISION" 2>&1 >> "$abs_detailed_report_file"
fi

#---------------------------------------------------------- GIT: ice
if [[ -d externals/ice ]]; then
	# 2024-08-01: Would be nice to have a check that actually says there are updats or not, but lets just get things
	#   done by assuming it always updates for right now. This can be a future improvement.
	updated_externals=true

	echo "Updating InternalCombustion to: $INTERNAL_COMBUSTION_REVISION"
	git -C "externals/ice/" fetch 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/ice/" pull --rebase 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/ice/" checkout "$INTERNAL_COMBUSTION_REVISION" 2>&1 >> "$abs_detailed_report_file"
else
	echo "Initializing InternalCombustion to: $INTERNAL_COMBUSTION_REVISION"
	git clone git@github.com:TyreBytes/ice.git "externals/ice/" 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/ice/" checkout "$INTERNAL_COMBUSTION_REVISION" 2>&1 >> "$abs_detailed_report_file"
fi

#---------------------------------------------------------- GIT: track_builder
if [[ -d externals/track_builder ]]; then
	# 2024-08-01: Would be nice to have a check that actually says there are updats or not, but lets just get things
	#   done by assuming it always updates for right now. This can be a future improvement.
	updated_externals=true

	echo "Updating TrackBuilder to: $TRACK_BUILDER_REVISION"
	git -C "externals/track_builder/" fetch 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/track_builder/" pull --rebase 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/track_builder/" checkout "$TRACK_BUILDER_REVISION" 2>&1 >> "$abs_detailed_report_file"
else
	echo "Initializing TrackBuilder to: $TRACK_BUILDER_REVISION"
	git clone git@github.com:TyreBytes/track_builder.git "externals/track_builder/" 2>&1 >> "$abs_detailed_report_file"
	git -C "externals/track_builder/" checkout "$TRACK_BUILDER_REVISION" 2>&1 >> "$abs_detailed_report_file"
fi


#---------------------------------------------------------- BUILD
pushd externals/turtle_brains/build/
source make_project.sh --clean --build
popd > /dev/null

pushd externals/ice/build/
source make_project.sh --clean --build
popd > /dev/null

pushd externals/track_builder/build/
source make_project.sh --clean --build
popd > /dev/null
