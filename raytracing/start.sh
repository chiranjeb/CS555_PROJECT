#!/bin/bash
projHome=/s/red/b/nobackup/data/portable/CS555_PROJECT/raytracing
me=$(hostname)
tmux new -s cs555proj -d
echo "Starting client@$(hostname)"
tmux send-keys -t cs555proj "ssh -t $(hostname).cs.colostate.edu 'cd $projHome; ./build/client properties/master_properties.txt properties/client_properties.txt random 10 10 10'" Enter
while read machine
do
		if [ $totalPanes -eq "8" ]; then
		  tmux new-window -t cs555proj
		  totalPanes=0
		else
		  tmux split-window -t cs555proj
		  tmux select-layout -t cs555proj even-vertical
		fi
		echo "Starting worker@${machine}"
    tmux send-keys -t cs555proj "ssh -t ${machine}.cs.colostate.edu 'cd $projHome; ./build/worker properties/master_properties.txt properties/worker_properties.txt'" Enter
		totalPanes=$((totalPanes+1))
done < $1
tmux split-window -t cs555proj
tmux select-layout -t cs555proj even-vertical
echo "Starting master@$(hostname)"
tmux send-keys -t cs555proj "ssh -t $(hostname).cs.colostate.edu 'cd $projHome; ./build/master properties/master_properties.txt'" Enter
tmux attach -t cs555proj