
folders=`ls -d */`

for f in $folders; do
 echo "$f"
 for r in ${f}*.o*; do
  if [ -f "${r}" ]; then 
   cat $r | grep Moves
  fi
  echo ""
 done
done
