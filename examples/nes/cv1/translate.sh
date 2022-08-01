#!/bin/bash

while read p; do
 charArray=(`echo $p | grep -o .`)
 echo "|..|"${charArray[3]}${charArray[2]}${charArray[1]}${charArray[0]}${charArray[4]}${charArray[5]}${charArray[6]}${charArray[7]}"|"
done < $1
