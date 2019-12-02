#!/bin/bash
filename=$1
rm -rf logs
mkdir -p logs
while read line; do
	# reading each line
	echo "cd /s/chopin/k/grad/mondal/CS555/raytracing" > remote.cmd

	echo "./build/worker  ./properties/master_properties.txt ./properties/worker_properties.txt" >> remote.cmd
	#echo $line | awk '{print $2}'
	#echo $line | awk '{print $3}'
	#cat remote.cmd
	n=$((n+1))
	machine=`echo $line | awk '{print $1}'`
	#echo $machine
	cmdbase="start putty.exe -sessionlog logs/"
	executecmd=" -ssh -2 -l mondal  -m remote.cmd "
	cmd="$cmdbase$machine.log$executecmd$machine.cs.colostate.edu"
	echo $cmd
	$cmd
	sleep 1
done < $filename
#rm -rf remote.cmd
