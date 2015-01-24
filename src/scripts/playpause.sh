#!/bin/bash
#For the single Play/Pause key
GetStatus=$(xmms2 current | awk -F":" '{print $1}')

if [ "$GetStatus" == "Playing" ]; then xmms2 pause;fi
if [ "$GetStatus" == "Paused" ]; then xmms2 play;fi

if [ "$GetStatus" == "Stopped" ]; then
   xmms2 play
fi

Times=$(xmms2 current | awk -F": " '{print $3}')
MinutesNow=$(echo $Times | cut -b 1-2 | sed s/^0// )
SecondsNow=$(echo $Times | cut -b 4-5 | sed s/^0// )
MinutesTotal=$(echo $Times | cut -b 10-11 | sed s/^0// )
SecondsTotal=$(echo $Times | cut -b 13-14 | sed s/^0// )

Percentage=$(( ( ($MinutesNow * 60 + $SecondsNow) * 100) / ($MinutesTotal * 60 + $SecondsTotal) ))

mfsplash /var/lib/mfsplash/icon/music.png $Percentage

UPDATECOUNT=0

while true
do

	UPDATECOUNT=$(( $UPDATECOUNT + 1 ))

	if [ $UPDATECOUNT = "10" ]
	then
		exit 0
	fi

	sleep 1
	Times=$(xmms2 current | awk -F": " '{print $3}')
	MinutesNow=$(echo $Times | cut -b 1-2 | sed s/^0// )
	SecondsNow=$(echo $Times | cut -b 4-5 | sed s/^0// )
	MinutesTotal=$(echo $Times | cut -b 10-11 | sed s/^0// )
	SecondsTotal=$(echo $Times | cut -b 13-14 | sed s/^0// )

	Percentage=$(( ( ($MinutesNow * 60 + $SecondsNow) * 100) / ($MinutesTotal * 60 + $SecondsTotal) ))

	mfsplash /var/lib/mfsplash/icon/music.png $Percentage 1

done
