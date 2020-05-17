#!/bin/bash

# TODO: readme

:'
- SCRIPT USAGE:
-- source client_hub times_to_send pid_receiver message_id message max_distance
- NB: se il messaggio inserito ha spazi in mezzo, mettere " all inizio e alla fine
-- e.g: source client_hub.sh 1 2 3 "messaggio con spazi" 4
Alcuni parametri inviati allo script vengono modificati per rendere unique ogni messaggio inviato senza avere doppioni:
    - message id: ad ogni iterazione viene aggiunto 1
    - messaggio: viene aggiunto in append il message id
e.g: lanciando source client_hub.sh 5 3545 1 "messaggio con spazi" 4 si ottiene
    - ../client 3545 1 "messaggio con spazi 1" 4
    - ../client 3545 2 "messaggio con spazi 2" 4
    - ../client 3545 3 "messaggio con spazi 3" 4
    - ../client 3545 4 "messaggio con spazi 4" 4
    - ../client 3545 5 "messaggio con spazi 5" 4
'


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