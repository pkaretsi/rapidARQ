#!/bin/bash

# Script for executing C-ARQ experiment 3 multiple times
#For a fixed, relatively-large value of propagation delay and bandwidth, change code rate and loss rate.
#For each experiment, give the experiment's number (1-4) in order to define filename. For the rest test cases, results will be stored in a carq.txt file.

SIMULATOR_PATH="./ns"
SIMULATION_FILE="arq/tcl_files/carq_cbr.tcl"
EXPERIMENT_ID="3"

#Number of iterations
NUM_ITERATIONS=5

#Number of retransmissions
num_rtx=(0 3)

#following error rates correspond to total loss rate of 2%, 5%, 10% and 20% respectively
err_rate=(0.01 0.025 0.05 0.105)
ratek_values=(49 19 9 4)

coding_depths0=(1 2 3 4 5) 
coding_depths1=(1 3 5 7 9 11 13)
coding_depths2=(1 3 5 7 10 13 16 20 24)
coding_depths3=(1 3 5 7 10 15 20 25 35 50)

arrays=(coding_depths0 coding_depths1 coding_depths2 coding_depths3)

#cd $(dirname $(readlink -f $SIMULATOR_PATH))

for rtx in "${num_rtx[@]}"; do
	for i in "${!ratek_values[@]}"; do 
		declare -n temp="${arrays[i]}"
		for cdepth in "${temp[@]}";
		do 
			for iter in $(seq 1 $NUM_ITERATIONS); do 
				echo "Experiment 3| num_rtx: $rtx Error rate: ${err_rate[$i]} rate_k: ${ratek_values[$i]} coding_depth: $cdepth Iteration $iter";
				RUN_SIMULATION="${SIMULATOR_PATH} ${SIMULATION_FILE} 10M 100ms 250 10M 1000 ${err_rate[$i]} ${err_rate[$i]} $rtx ${ratek_values[$i]} $cdepth 0 1000 $iter"			
				${RUN_SIMULATION} ${EXPERIMENT_ID};
			done 
		done
	done
done

