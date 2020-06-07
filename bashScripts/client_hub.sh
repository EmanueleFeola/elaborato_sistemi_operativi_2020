#!/bin/bash

args=("$@")

SCRIPT_NAME=$(basename ${BASH_SOURCE[0]})

if [ "$#" != 5 ]; then
    echo -e "\e[31mERROR: $SCRIPT_NAME takes 5 params: times_to_send pid_receiver message_id message max_distance\e[0m"
    return
fi

TIMES_TO_SEND=${args[0]}
PID_RECEIVER=${args[1]}
MESSAGE_ID=${args[2]}
MESSAGE=${args[3]} 
MAX_DISTANCE=${args[4]} 

echo "<$SCRIPT_NAME> starting $TIMES_TO_SEND time(s) ../client $PID_RECEIVER $MESSAGE_ID $MESSAGE $MAX_DISTANCE" 

for (( c = 0; c < ${args[0]}; c++ ))
do    
    ../client $PID_RECEIVER $MESSAGE_ID "$MESSAGE $MESSAGE_ID" $MAX_DISTANCE
    ((MESSAGE_ID++))
    # sleep 1
done