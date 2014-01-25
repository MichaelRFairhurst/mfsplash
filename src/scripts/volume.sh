#!/bin/bash

CURRENTVOL=$(amixer get Master | tail -n 1 | sed 's/^[^\[]*\[\([0-9]\+\)%\].*/\1/')

case $1 in
	up) NEXTVOL=$((CURRENTVOL+2)) ;;
	down) NEXTVOL=$((CURRENTVOL-2)) ;;
	*) NEXTVOL=$1 ;;
esac

ech set volume to $NEXTVOL
amixer set Master $NEXTVOL%
mfsplash /var/lib/mfsplash/icon/volume.png $NEXTVOL
