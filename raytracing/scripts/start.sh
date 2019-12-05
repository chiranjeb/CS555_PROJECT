#!/bin/bash
totalPanes=1
tmux new -s cs555proj -d
echo "Starting master@$(hostname)"
tmux send-keys -t cs555proj "ssh -t $(hostname).cs.colostate.edu 'cd $(pwd); \
  ./build/master properties/master_properties.txt'" Enter
while read machine; do
    if [ $totalPanes -eq "8" ]; then
      tmux new-window -t cs555proj
      totalPanes=0
    else
      tmux split-window -t cs555proj
      tmux select-layout -t cs555proj even-vertical
    fi
    echo "Starting worker@${machine}"
    tmux send-keys -t cs555proj "ssh -t ${machine}.cs.colostate.edu 'cd $(pwd); \
  			./build/worker properties/master_properties.txt properties/worker_properties.txt'" Enter
    totalPanes=$((totalPanes + 1))
done < $1
tmux select-layout -t cs555proj even-vertical
tmux split-window -t cs555proj
echo "Starting client@$(hostname)"
#sleep 10
tmux send-keys -t cs555proj "ssh -t $(hostname).cs.colostate.edu 'cd $(pwd); \
  ./build/client properties/master_properties.txt properties/client_properties.txt $5 $2 $3 $4'" #Enter
tmux select-layout -t cs555proj even-vertical
tmux attach -t cs555proj
