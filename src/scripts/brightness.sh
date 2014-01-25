#!/bin/bash

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
[ $NEXT -gt 10 ] && echo "at max" && exit 4
if [ "$(id -u)" != "0" ]; then
	echo "Sorry, you are not root."
	exit 1
fi
echo $NEXT > /sys/class/backlight/acpi_video0/brightness

mfsplash /var/lib/mfsplash/icon/brightness.png "$NEXT"0
