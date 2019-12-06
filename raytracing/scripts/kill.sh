#!/bin/bash
kill $(ps aux | grep $USER | grep "build/worker" | awk '{print $2}')
kill $(ps aux | grep $USER | grep "build/master" | awk '{print $2}')
kill $(ps aux | grep $USER | grep "build/client" | awk '{print $2}')
