#include "inc/ackManager.h"
#include "utils/msg_queue.h"

Acknowledgment *acklist_ptr;
int msgid;

void ackSigHandler(int sig){
    printf("<ackManager %d> terminating\n", getpid());

    if(shmdt(acklist_ptr) == -1)
        ErrExit("<device> shmdt failed\n");

    if (msgctl(msgid, IPC_RMID, NULL) == -1)
        ErrExit("msgctl failed");

    exit(0);
}

void setAckSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGTERM, ackSigHandler);
}

// ordina gli ack per timestamp (decrescente)
// quindi come primo Ack di cm->acks troverò l ack con il timestamp più grande, ovvero quello più recente)
void sortAck(ClientMessage *cm){
	for (int i = 0; i < NDEVICES; i++)                     //Loop for descending ordering
	{
		for (int j = 0; j < NDEVICES; j++)             //Loop for comparing other values
		{
			if (cm->acks[j].timestamp < cm->acks[i].timestamp)                //Comparing other array elements
			{
				Acknowledgment tmp = cm->acks[i];         //Using temporary variable for storing last value
				cm->acks[i] = cm->acks[j];            //replacing value
				cm->acks[j] = tmp;             //storing last value
			}
		}
	}
}

// controlla il segmento di shared memory per vedere se qualche messaggio è stato ricevuto da tutti i device
// in caso positivo manda la lista di ack al client
// scorre il segmento di memoria condivisa una sola volta e calcola l array delle occorrenze per i message_id che trova 
void ackManagerRoutine(Acknowledgment *ptr, int msgid){
    int found_message_id_list[10] = {0}; // message_id degli ack che sono stati ricevuti da tutti
    int found_message_id_counter = 0;

    int occourrences[10] = {[0 ... 9] = 1};

    Acknowledgment *ack;

    int counter = 0;
    for(; counter < 100; counter++){
        ack = &ptr[counter];

        if(ack->message_id != 0){ // guardo quelli solo con message_id diverso da 0, ovvero quelli validi
            // printAck(*ack, "ackManager", "read");

            int index = contains(found_message_id_list, ack->message_id); // ritorna indice a cui l ha trovato, oppure -1
            if(index == -1){
                // inserisco message_id nella lista dei message_id trovati
                found_message_id_list[found_message_id_counter] = ack->message_id;
                found_message_id_counter++;
            } else{
                // se il message_id corrente è già stato incontrato, aggiorno il contatore delle sue occorrenze
                // message_id gia presente nella lista di message_id trovati, quindi aggiorno le sue occorrenze
                occourrences[index]++;
            }
        }
    }
    
    // scorro le occorrenze e guardo se qualche msg è arrivato a tutti 
    int occCounter;
    for(occCounter = 0; occCounter < sizeof(occourrences) / sizeof(int); occCounter++){
        /*
        if(occourrences[occCounter] > 1)
            printf("#DEBUG: %d (%d times)\n", found_message_id_list[occCounter], occourrences[occCounter]);
        */
       
        // se tutti i device lo hanno ricevuto --> invia al client e segna tutti gli ack come sovrascrivibili
        if(occourrences[occCounter] == NDEVICES){ 
            // printf("<ackManager> Il messaggio %d è arrivato a tutti\n", found_message_id_list[occCounter]);

            ClientMessage cm;
            cm.mtype = found_message_id_list[occCounter];
            size_t mSize = sizeof(ClientMessage) - sizeof(long);

            // riempio cm.acks con i suoi ack
            int ackCounter;
            for(ackCounter = 0, counter = 0; counter < 100 && ackCounter < NDEVICES; counter++){
                ack = &ptr[counter];

                if(ack->message_id == found_message_id_list[occCounter]){
                    cm.acks[ackCounter] = *ack; 
                    ack->message_id = 0; //reset
                    ackCounter++;
                    // printAck(*ack, "ackManagerRoutine", "loop");
                }
            }

            // funzione di ordinamento ack per timestamp
            sortAck(&cm);

            Acknowledgment test;
            int counter; 
            for(counter = 0; counter < NDEVICES; counter++){
                test = cm.acks[counter];
                printAck(test, "ackManager", "test");
            }
            
            writeMsgQueue(msgid, &cm, mSize, 0);

            // if (msgsnd(msgid, &cm, mSize, 0) == -1)
            //     ErrExit("msgsnd failed");
        }
    }
}

void startAckManager(int semid, int acklist_shmid, key_t key){
    setAckSignalMask();

    msgid = getMsgQueue(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    acklist_ptr = (Acknowledgment *)get_shared_memory(acklist_shmid, 0);

    // dormo 5 sec
    // blocco sem ack_list
    // check acks in ack_list
    // ce ne sono 5 con lo stesso message id?
    // si --> invia msg a client e libera spazio, no --> nothing
    // sblocco sem ack_list

    while(1){
        sleep(5);

        printf("<ackManager> Routine check started!\n");

        semOp(semid, NDEVICES + 1, -1); // blocco la ack_list

        ackManagerRoutine(acklist_ptr, msgid);

        semOp(semid, NDEVICES + 1, 1); // sblocco la ack_list
    }
}

void write_ack(Acknowledgment *ptr, Acknowledgment ack){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];
        if(a->message_id == 0){
            a->pid_sender = ack.pid_sender;
            a->pid_receiver = ack.pid_receiver;
            a->message_id = ack.message_id;
            a->timestamp = ack.timestamp;
            printf("Writing on shm at %d: %d %d %d %ld\n", counter, a->pid_sender, a->pid_receiver, a->message_id, a->timestamp);
            break;
        }
    }
}

// ritorna il numero di occorrenze di un message_id nella acklist
int acklist_countByMsgId(Acknowledgment *ptr, int message_id){
    int found = 0;
    int counter;

    Acknowledgment *a;

    for(counter = 0; counter < 100; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id)
            found++;
    }

    return found;
}

// ritorna 1 se c'è già un ack identificato da message_id
// meglio chiamare questa invece che la funzione sopra se 
// ho bisogno di sapere solo se un certo message_id è già stato inserito 
int acklist_contains_message_id(Acknowledgment *ptr, int message_id){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id)
            return 1;
    }

    return -1;
}

// ritorna 1 se il device con pid pid_receiver ha gia scritto l ack identificato da message_id
int acklist_contains_tupla(Acknowledgment *ptr, int message_id, pid_t pid_receiver){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id && a->pid_receiver == pid_receiver)
            return 1;
    }

    return -1;
}

// debug purposes
void read_acks(Acknowledgment *ptr){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];

        if(a->message_id != 0){
            printf("Found at %d: %d %d %d %ld\n", counter, a->pid_sender, a->pid_receiver, a->message_id, a->timestamp);
        }
    }
}