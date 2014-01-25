#!/bin/bash

DISCHARGING=`acpi | grep Charging`
PERCENTAGE=`acpi | sed 's/Battery [0-9][^0-9]*\([0-9]*\)%.*/\1/'`

echo discharging $DISCHARGING percentage $PERCENTAGE

if [ -n "$DISCHARGING" ]
then
	ICON="charging.png"
else
	ICON="battery.png"
fi

mfsplash /var/lib/mfsplash/icon/$ICON $PERCENTAGE
