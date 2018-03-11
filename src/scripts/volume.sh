#!/bin/bash

####
## Set master volume level, and display splash with new level
##
## Usage:
##   volume up -- +2% volume
##   volume down -- -2% volume
##   volume toggle -- toggle mute
##   volume [int] -- set to int% volume
##
## Bind to key combinations in your window manager
####

if [[ "$1" = "help" || "$1" = "--help" || "$1" = "-h" ]]
then
	echo Usage for $0
	cat $0 | grep "^##" | grep -v "#!" | sed 's/^#\+//'
	exit 0
fi

CURRENTVOL=$(amixer get Master | tail -n 1 | sed 's/^[^\[]*\[\([0-9]\+\)%\].*/\1/')

case $1 in
  toggle)
    amixer sset Master toggle | grep -q \\[on\\]
    if [[ $? -eq 0 ]]; then
      mfsplash /var/lib/mfsplash/icon/volume.png $CURRENTVOL
    else
      mfsplash /var/lib/mfsplash/icon/volume.png 0
    fi
    exit 0
    ;;
    
	up) NEXTVOL=$((CURRENTVOL+2)) ;;
	down) NEXTVOL=$((CURRENTVOL-2)) ;;
	*) NEXTVOL=$1 ;;
esac

echo set volume to $NEXTVOL
amixer sset Master $NEXTVOL%
amixer sset Master on
mfsplash /var/lib/mfsplash/icon/volume.png $NEXTVOL
