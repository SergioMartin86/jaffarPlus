#!/bin/bash

while read p; do
 charArray=(`echo $p.`)

 buttonU="."; if [[ $charArray == *U* ]]; then buttonU=U; fi
 buttonD="."; if [[ $charArray == *D* ]]; then buttonD=D; fi
 buttonL="."; if [[ $charArray == *L* ]]; then buttonL=L; fi
 buttonR="."; if [[ $charArray == *R* ]]; then buttonR=R; fi
 buttons="."; if [[ $charArray == *s* ]]; then buttons=s; fi
 buttonS="."; if [[ $charArray == *S* ]]; then buttonS=s; fi
 buttonY="."; if [[ $charArray == *Y* ]]; then buttonY=Y; fi
 buttonB="."; if [[ $charArray == *B* ]]; then buttonB=B; fi
 buttonX="."; if [[ $charArray == *X* ]]; then buttonX=X; fi
 buttonA="."; if [[ $charArray == *A* ]]; then buttonA=A; fi
 buttonl="."; if [[ $charArray == *l* ]]; then buttonl=l; fi
 buttonr="."; if [[ $charArray == *r* ]]; then buttonr=r; fi

 echo "|..|${buttonU}${buttonD}${buttonL}${buttonR}${buttons}${buttonS}${buttonY}${buttonB}${buttonX}${buttonA}${buttonl}${buttonr}|............|"
done < $1
