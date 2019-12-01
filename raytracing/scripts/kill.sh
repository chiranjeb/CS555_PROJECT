#!/bin/bash
while read machine
do
  kill $(ps aux | grep $USER | grep "build/worker" | awk '{print $2}')
done < $1
