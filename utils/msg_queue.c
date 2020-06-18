/// @file msg_queue.c
/// @brief Contiene l'implementazione delle funzioni per la gestione delle message queues.

#include "../inc/defines.h"
#include "msg_queue.h"

int getMsgQueue(key_t key, int flags){//QUI CREO UNA NUOVA MESSAGE QUEUE SE NON NE ESISTE GIA' UNA CON I PARAMETRI CHE IO LE PASSO OPPURE ME NE RESTITUISCE UNA CHE ESISTE GIA' SE COMBACIA CON I PARAMETRI CHE LE HO PASSATO
    int msqid = msgget(key, flags);

    if(msqid == -1)
        ErrExit("msgget failed");

    return msqid;
}


void readMsgQueue(int msqid, ClientMessage *msgp, size_t msgsz, long msgtype, int msgflg){//ClientMessage *msgp PUNTA ALLA STRUTTURA ClientMessage CHE CONTIENE Acknowledgment acks[5] 
    if (msgrcv(msqid, msgp, msgsz, msgtype, msgflg) == -1)//SYSTEM CALL CHE LEGGE DA UNA MESSAGE QUEUE UN MESSAGGIO E GLI DICO DI FARE UN CAST A ClientMessage E QUELLO CHE LEGGE E' EFFETTIVAMENTE UN CLIENT MESSAGE
        ErrExit("msgrcv failed");
}

void writeMsgQueue(int msqid, ClientMessage *msgp, size_t msgsz, int msgflg){
    if (msgsnd(msqid, msgp, msgsz, msgflg) == -1)
        ErrExit("msgsnd failed");
}