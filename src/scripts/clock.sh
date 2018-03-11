#!/bin/bash

####
## Show a popup of the current time.
##
## Usage:
##   clock
##
## Bind to key combinations in your window manager
####

if [[ "$1" = "help" || "$1" = "--help" || "$1" = "-h" ]]
then
	echo Usage for $0
	cat $0 | grep "^##" | grep -v "#!" | sed 's/^#\+//'
	exit 0
fi

mfsplash /var/lib/mfsplash/icon/clock.png --text "$(date '+%R %a %b %d')"
