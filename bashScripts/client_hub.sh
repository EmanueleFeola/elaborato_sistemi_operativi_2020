#!/bin/bash

:'
- SCRIPT USAGE:
-- source client_hub times_to_send pid_receiver message_id message max_distance
- NB: se il messaggio inserito ha spazi in mezzo, mettere " all inizio e alla fine
-- e.g: client_hub.sh 1 2 3 "messaggio con spazi" 4
Al messaggio inviato allo script client viene aggiunto l indice del for per differenziare i messaggi se times_to_send > 1
'

clear

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
    ../client $PID_RECEIVER $MESSAGE_ID "$MESSAGE $c" $MAX_DISTANCE
    # sleep 1
done