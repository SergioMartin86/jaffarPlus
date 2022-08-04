#!/bin/bash

while read p; do
 charArray=(`echo $p.`)

 buttonU="."; if [[ $charArray == *U* ]]; then buttonU=U; fi
 buttonD="."; if [[ $charArray == *D* ]]; then buttonD=D; fi
 buttonL="."; if [[ $charArray == *L* ]]; then buttonL=L; fi
 buttonR="."; if [[ $charArray == *R* ]]; then buttonR=R; fi
 buttonT="."; if [[ $charArray == *T* ]]; then buttonT=S; fi
 buttonS="."; if [[ $charArray == *S* ]]; then buttonS=s; fi
 buttonB="."; if [[ $charArray == *B* ]]; then buttonB=B; fi
 buttonA="."; if [[ $charArray == *A* ]]; then buttonA=A; fi

 echo "|..|${buttonU}${buttonD}${buttonL}${buttonR}${buttonT}${buttonS}${buttonB}${buttonA}|"
done < $1
