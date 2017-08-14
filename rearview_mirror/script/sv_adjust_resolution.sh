#!/bin/sh

RESOLUTION="1200,1920"
pid=$1

#echo $pid
if [ -z $pid ];then
	echo "no such process found, exit!"
	exit 1;
fi
window_number="`ls /dev/screen/$pid | grep win`"
#echo $window_number
for i in $window_number
do
	#echo "$i"
	if [ -n "$i" ];then
		screencmd setiv $i SCREEN_PROPERTY_SIZE $RESOLUTION &
	fi
done


exit 0;
