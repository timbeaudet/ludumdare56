#!/bin/bash

#
# Simple batch script hooked into the Automated Build System to run project specific test commands. If an error
# occurs that should halt the build and fail the nightly; set an error-code above 10 in the variable abs_return_value
#   abs_return_value=11
# 
# To log any details to the email report, use printf like so:
#   printf "Here are some details about the custom step." >> "$abs_detailed_report_file"
# -------------------------------------------------------------------------------------------------------------------
