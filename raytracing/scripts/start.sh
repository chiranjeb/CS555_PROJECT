#!/bin/bash
totalPanes=0
i=0
tmux new -s cs555proj -d
while read machine
do
		echo "Starting worker@${machine}"
		tmux send-keys -t cs555proj "ssh -t ${machine}.cs.colostate.edu 'cd $(pwd); \
			./build/worker properties/master_properties.txt properties/worker_properties.txt'" Enter
		totalPanes=$((totalPanes+1))
		i=$((i+1))
		if [ $totalPanes -eq "8" ]; then
		  tmux new-window -t cs555proj
		  totalPanes=0
		else
		  tmux split-window -t cs555proj
		  tmux select-layout -t cs555proj even-vertical
		fi
		if [ $i -eq $1 ]; then
		  break
		fi
done < "scripts/275machines.txt"
tmux send-keys -t cs555proj "exit" Enter
tmux select-layout -t cs555proj even-vertical
tmux new-window -t cs555proj
echo "Starting master@$(hostname)"
tmux send-keys -t cs555proj "ssh -t $(hostname).cs.colostate.edu 'cd $(pwd); \
  ./build/master properties/master_properties.txt'" Enter
tmux split-window -t cs555proj
echo "Starting client@$(hostname)"
sleep 2
tmux send-keys -t cs555proj "ssh -t $(hostname).cs.colostate.edu 'cd $(pwd); \
  ./build/client properties/master_properties.txt properties/client_properties.txt random $2 $3 $4'" Enter
tmux attach -t cs555proj