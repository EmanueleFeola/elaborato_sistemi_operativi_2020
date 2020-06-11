#!/bin/bash

args=("$@")

SCRIPT_NAME=$(basename ${BASH_SOURCE[0]})

if [ "$#" != 7 ]; then
    echo -e "\e[31mERROR: $SCRIPT_NAME takes 6 params: msq_queue_key times_to_send pid_receiver message_id message max_distance delay\e[0m"
    return
fi

MSG_QUEUE_KEY=${args[0]}
TIMES_TO_SEND=${args[1]}
PID_RECEIVER=${args[2]}
MESSAGE_ID=${args[3]}
MESSAGE=${args[4]} 
MAX_DISTANCE=${args[5]} 
DELAY=${args[6]}

echo "<$SCRIPT_NAME> starting $TIMES_TO_SEND time(s) ../client $MSG_QUEUE_KEY $PID_RECEIVER $MESSAGE_ID $MESSAGE $MAX_DISTANCE" 

for (( c = 0; c < TIMES_TO_SEND; c++ ))
do    
    ../client $MSG_QUEUE_KEY $PID_RECEIVER $MESSAGE_ID "$MESSAGE $MESSAGE_ID" $MAX_DISTANCE &
    ((MESSAGE_ID++))
    sleep $DELAY
done