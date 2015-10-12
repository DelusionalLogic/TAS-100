#!/bin/bash

mapName="font.map"
echo -n "" > $mapName

i=0
for l in {{a..z},{A..Z},{0..9},:,!,.}; do
	echo "$i -> $l" >> $mapName
	echo $l
	convert -font ./ter-x12n.pcf -pointsize 12 label:"$l" "font_$i.xbm"
	((i++))
done
