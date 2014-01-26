#!/bin/bash

##
# Launch mfsplash with battery level.
#
# Has a separate charging vs not plugged in icon.
#
# Hook this up to a key combination, and use `batterywatch` for
# automatic battery level popups
##

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
