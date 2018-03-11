#!/bin/bash

####
## Increase, reduce, or specify screen brightness.
##
## Usage
##    brightness up -- increase brightness by 10%
##    brightness down - decrease brightness by 10%
##    brightness [int] - set brightness to 10 * int %
##
## Bind to a key combination in your window manager. requires root perms
##
## This may not work exactly for your laptop. The /sys/class/backlight path may
## not be the same on all systems.
####

if [[ "$1" = "help" || "$1" = "--help" || "$1" = "-h" ]]
then
	echo Usage for $0
	cat $0 | grep "^##" | grep -v "#!" | sed 's/^#\+//'
	exit 0
fi

if [ "$(id -u)" != "0" ]; then
	echo "Warning: changing backlight may require root...Trying anyways."
fi

SCREENROOT="/sys/class/backlight"
SCREEN="$SCREENROOT/$(ls $SCREENROOT | head -n 1)"
CURRENT=$(cat $SCREEN/brightness)
MAX=$(cat $SCREEN/max_brightness)

[ $CURRENT -gt 0 ] || (echo "at min" && exit 4)
[ $CURRENT -lt $MAX ] || (echo "at max" && exit 4)

INPCT=$(( $CURRENT * 100 / $MAX ))

if [ "$1" = "up" ]; then
	INPCT=$(( $INPCT + 2 ))
elif [ "$1" = "down" ]; then
	INPCT=$(( $INPCT - 2 ))
else
  INPCT=$1
fi

NEXT=$(( $INPCT * $MAX / 100 ))

if [[ $NEXT -eq $CURRENT ]] ; then
  NEXT=$(( $CURRENT + 1 ))
fi

[ -z "$NEXT" ] && echo "Provide a bightness please - up down or a number" && exit 2
if [ $NEXT -gt $MAX ]; then
  NEXT=$MAX
elif [ $NEXT -lt 0 ]; then
  NEXT=0
fi

echo $NEXT > $SCREEN/brightness

mfsplash /var/lib/mfsplash/icon/brightness.png "$INPCT"
