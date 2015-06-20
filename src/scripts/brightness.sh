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
## This may not work exactly for your laptop. The /sys/ path may not be
## the same on all systems. Also check for a max_brightness file, it may
## accept brightnesses from 0 to 100000000 which would require a tweak or
## two to this script....
####

if [[ "$1" = "help" || "$1" = "--help" || "$1" = "-h" ]]
then
	echo Usage for $0
	cat $0 | grep "^##" | grep -v "#!" | sed 's/^#\+//'
	exit 0
fi

CURRENT=$(cat /sys/class/backlight/acpi_video0/brightness)

if [ "$1" = "up" ]
then
	NEXT=$(( $CURRENT + 1 ))
else
	if [ "$1" = "down" ]
	then
		NEXT=$(( $CURRENT - 1 ))
	else
		NEXT=$1
	fi
fi

[ -z "$NEXT" ] && echo "Provide a bightness please - up down or a number" && exit 2
[ $NEXT -lt 0 ] && echo "at min" && exit 4
[ $NEXT -gt 1953 ] && echo "at max" && exit 4
if [ "$(id -u)" != "0" ]; then
	echo "Sorry, you are not root."
	exit 1
fi
echo $NEXT > /sys/class/backlight/acpi_video0/brightness

mfsplash /var/lib/mfsplash/icon/brightness.png "$NEXT"0
