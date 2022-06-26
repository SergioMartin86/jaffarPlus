#!/bin/bash

while read p; do
 charArray=(`echo $p.`)

 buttonU="."; if [[ $charArray == *U* ]]; then buttonU=U; fi
 buttonD="."; if [[ $charArray == *D* ]]; then buttonD=D; fi
 buttonL="."; if [[ $charArray == *L* ]]; then buttonL=L; fi
 buttonR="."; if [[ $charArray == *R* ]]; then buttonR=R; fi
 buttonA="."; if [[ $charArray == *A* ]]; then buttonA=A; fi
 buttonB="."; if [[ $charArray == *B* ]]; then buttonB=B; fi
 buttonC="."; if [[ $charArray == *C* ]]; then buttonC=C; fi
 buttonS="."; if [[ $charArray == *S* ]]; then buttonS=S; fi

 echo "|..|${buttonU}${buttonD}${buttonL}${buttonR}${buttonA}${buttonB}${buttonC}${buttonS}|"
done < $1
