#!/bin/bash

####
## Launch mfsplash with battery level.
##
## Has a separate charging vs not plugged in icon.
##
## Hook this up to a key combination, and use `batterywatch` for
## automatic battery level popups
####

if [[ "$1" = "help" || "$1" = "--help" || "$1" = "-h" ]]
then
	echo Usage for $0
	cat $0 | grep "^##" | grep -v "#!" | sed 's/^\#\+//'
	exit 0
fi

DISCHARGING=$(acpi | egrep "(Charging|Full)")
PERCENTAGE=$(acpi | sed 's/Battery [0-9][^0-9]*\([0-9]*\)%.*/\1/')

echo discharging $DISCHARGING percentage $PERCENTAGE

if [ -n "$DISCHARGING" ]
then
	ICON="charging.png"
else
	ICON="battery.png"
fi

mfsplash /var/lib/mfsplash/icon/$ICON $PERCENTAGE
