stateNames=`ls *.state`
for state in $stateNames
do
 echo "Running $state..."
 cp $state birdskip.state
 jaffar birdskip.jaffar | tee ${state}.log
done
