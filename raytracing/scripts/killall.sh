#!/bin/bash
while read machine
do
	echo $machine
	ssh -n $machine "$(pwd)/scripts/kill.sh" &
done < $1
./scripts/kill.sh
