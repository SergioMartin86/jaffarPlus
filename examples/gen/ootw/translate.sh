#!/bin/bash

while read p; do
 charArray=(`echo $p | grep -o .`)
 echo "|..|"${charArray[0]}${charArray[1]}${charArray[2]}${charArray[3]}${charArray[4]}${charArray[5]}${charArray[6]}${charArray[7]}${charArray[8]}${charArray[9]}${charArray[10]}${charArray[11]}${charArray[12]}${charArray[13]}${charArray[14]}${charArray[15]}"|"
done < $1
