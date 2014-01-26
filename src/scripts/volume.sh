#!/bin/bash

##
# Set master volume level, and display splash with new level
#
# Usage:
#   battery up -- +2% battery
#   battery down -- -2% battery
#   battery [int] -- set to int% battery
#
# Bind to key combinations in your window manager
##

CURRENTVOL=$(amixer get Master | tail -n 1 | sed 's/^[^\[]*\[\([0-9]\+\)%\].*/\1/')

case $1 in
	up) NEXTVOL=$((CURRENTVOL+2)) ;;
	down) NEXTVOL=$((CURRENTVOL-2)) ;;
	*) NEXTVOL=$1 ;;
esac

ech set volume to $NEXTVOL
amixer set Master $NEXTVOL%
mfsplash /var/lib/mfsplash/icon/volume.png $NEXTVOL
