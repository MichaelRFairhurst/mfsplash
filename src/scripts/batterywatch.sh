#!/bin/bash

##
# Launch this script with your X session to get automatic battery notifications when
# 1. battery goes below 20%
# 2. battery goes below 10%
# 3. battery goes below 5%
# 4. battery is at or below 3% and not plugged in
# 5. battery is plugged in
# 6. battery is unplugged
##


PREVBAT=100
PREVCHG="false"

while true
do
	sleep 1
	BATTERY=$(acpi | sed 's/Battery [0-9][^0-9]*\([0-9]*\)%.*/\1/')
	if [ -z "$(acpi | egrep "(Charging|Full)" )" ]
	then
		CHARGING="false"
	else
		CHARGING="true"
	fi

	echo battery $BATTERY
	echo charging $CHARGING

	if [[ "$BATTERY" -lt 4 && "$CHARGING" == "false" ]]
	then
		battery
	fi

	if [[ "$BATTERY" = 5 && "$PREVBAT" -gt 5 ]]
	then
		battery
	fi

	if [[ "$BATTERY" = 10 && "$PREVBAT" -gt 10 ]]
	then
		battery
	fi

	if [[ "$BATTERY" = 20 && "$PREVBAT" -gt 20 ]]
	then
		battery
	fi

	if [[ "$PREVCHG" != "$CHARGING" ]]
	then
		battery &
	fi

	PREVBAT=$BATTERY
	PREVCHG=$CHARGING

	echo prevbattery $PREVBAT
	echo prevcharging $PREVCHG

done
