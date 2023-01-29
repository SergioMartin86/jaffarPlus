
folders=`ls -d */`

for f in $folders; do
 echo "$f"
 resultFiles=`find $f -name jaffar.best.sol`
 for r in ${resultFiles}; do
  if [ -f "${r}" ]; then 
   moveCount=`wc -l ${r} | head -n1 | cut -d' ' -f1`
   echo "   + $moveCount  ${r}"
  fi
 done
done
