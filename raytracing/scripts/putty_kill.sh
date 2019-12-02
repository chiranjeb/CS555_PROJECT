#!/bin/bash
filename=$1
while read line; do
	# reading each line
	echo "killall master worker" > kill_remote.cmd

	machine=`echo $line | awk '{print $1}'`
	#echo $machine
	cmd="start putty.exe -ssh -2 -l mondal -m kill_remote.cmd "
	cmd="$cmd$machine.cs.colostate.edu"
	rm -rf *.log
	echo $cmd
	$cmd
done < $filename
