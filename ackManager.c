#include "inc/ackManager.h"
#include "utils/msg_queue.h"

Acknowledgment *acklist_ptr;
int msgid;

void ackSigHandler(int sig){
    coloredPrintf("yellow", 0, "<ackManager %d> terminating\n", getpid());

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

void startAckManager(int semid, int acklist_shmid, key_t key){
    setAckSignalMask();//IL PROGETTO VUOLE CHE TUTTI I FIGLI VENGANO TERMINATI CON UN SEGNALE

    msgid = getMsgQueue(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);//QUI GIUSTAMENTE PRENDO LA MESSAGE QUEUE CHE MI FARA' DA TRAMITE TRA CLIENT E ackManager
    acklist_ptr = (Acknowledgment *)get_shared_memory(acklist_shmid, 0);//QUI FACCIO UN CAST A Acknowledgment PERCHE' LA get_shared_memory MI RITORNA UN PUNTATORE ALLA ZONA DI MEMORIA E DATO CHE IN QUELLA ZONA DI MEMORIA CI SONO DEGLI Acknowledgment E' COME SE INIZIALIZZASSI UN ARRAY DI STRUTTURE Acknowledgment. LA SHARED MEMORY PUO' ESSERE VISTA COME UN ARRAY CONDIVISO DA PIU' PROCESSI E CON QUESTO CAST STO DICENDO CHE QUESTO ARRAY E' FATTO DI Acknowledgment

    while(1){
        sleep(5);//L'ackManager DEVE ESSERE SBLOCCATO OGNI 5 SECONDI PERO' L'ackManager SI SBLOCCA DA SOLO, NON E' UNA DIRETTIVA CHE GLI VIENE INVIATA DAL SERVER. OGNI 5 SECONDI SI SVEGLIA E ANALIZZA L'ack_list.

        coloredPrintf("green", 1, "<ackManager> Routine check started!\n");

        semOp(semid, NDEVICES + 1, -1); // blocco la ack_list ---> QUI I SEMAFORI VENGONO IMPLEMENTATI LOCALMENTE ALL'INTERNO DELLO startAckManager MENTRE PER I DEVICE NO PERCHE' OGNI DEVICE E' A SE' STANTE E SI ANDAVA A "PRENDERE" IL SUO SEMAFORO 

        ackManagerRoutine(acklist_ptr, msgid);//PASSO IL PUNTATORE ALL'ack_list 

        semOp(semid, NDEVICES + 1, 1); // sblocco la ack_list ---> QUINDI COSA SUCCEDE ? OGNI 5 SECONDI BLOCCO, ANALIZZO E SBLOCCO
    }
}

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

//ackManager SCORRE TUTTA LA "ACK_LIST" --> HA VISTO QUALI SONO GLI Acknowledgment CHE SONO STATI RICEVUTI DA TUTTI I DEVICE --> INVIA GLI Acknowledgment (SE CE NE SONO) AL CLIENT

//VOGLIO SCORRERE L'ack_list UNA SOLA VOLTA PERCHE' SCORRERE UN ARRAY DALL'INIZIO ALLA FINE E' DISPENDIOSO
void ackManagerRoutine(Acknowledgment *ptr, int msgid){//GIUSTAMENTE ALL'INTERNO DELLA ackManagerRoutine PASSO ANCHE L'ID DELLA msg_queue PERCHE' SE TROVA I 5 ACK DI UN MESSAGGIO DOVRA' PASSARGLIELI
    int found_message_id_list[10] = {0};//SONO I MESSAGE ID CHE HO TROVATO DENTRO ACK LIST. found_message_id_list NON VIENE SCORSA ALL'INTERNO DI UN for PERCHE' STA TENENEDO CONTO DI QUANTI ELEMENTI STA INSERENDO DENTRO DI SE'. INFATTI CON found_message_id_counter IO VADO AD ASSEGNARE MANUALEMENTE IN QUALE CELLA DELL'ARRAY found_message_id_list INSERIRE IL message_id 
    int found_message_id_counter = 0;

    int occourrences[10] = {[0 ... 9] = 1};//RIEMPIO LE CELLE DI 1. "OCCORRENZE" = QUANTE VOLTE APPARE QUEL NUMERO LI

    Acknowledgment *ack;

    int counter = 0;
    for(; counter < MAX_ACK_LIST; counter++){
        ack = &ptr[counter];//PUNTATORE AL PRIMO ELEMENTO DI "ACK_LIST"

        if(ack->message_id != 0){ // guardo quelli solo con message_id diverso da 0, ovvero quelli validi. QUI PRENDO SOLO IL CAMPO message_id DELLA STRUTTURA PER POI COSTRUIRMI L'ARRAY DELLE OCCORRENZE  
            // printAck(*ack, "ackManager", "read");

            int index = contains(found_message_id_list, ack->message_id); // ritorna indice a cui l ha trovato, oppure -1. CON LA contains CONTROLLO SE DENTRO found_message_id_list C'E' ack->message_id (OSSIA L'ID CHE STO ATTUALMENTE CONTROLLANDO). 
            if(index == -1){//SE MI VIENE RITORNATO -1 (OSSIA NON C'E' ANCORA NESSUN message_id DENTRO found_message_id_list) INSERISCO IL message_id NELLA LISTA DEI message_id TROVATI
                found_message_id_list[found_message_id_counter] = ack->message_id;
                found_message_id_counter++;
            } else{
                // se il message_id corrente è già stato incontrato, aggiorno il contatore delle sue occorrenze
                occourrences[index]++;
            }
        }
    }
    
    // scorro le occorrenze e guardo se qualche msg è arrivato a tutti 
    int occCounter;
    for(occCounter = 0; occCounter < sizeof(occourrences) / sizeof(int); occCounter++){
        if(occourrences[occCounter] == NDEVICES){ 
            ClientMessage cm;
            cm.mtype = found_message_id_list[occCounter];//IL TIPO TRAMITE IL QUALE IL CLIENT FILTRA LA MESSAGE QUEUE 
            size_t mSize = sizeof(ClientMessage) - sizeof(long);//SERVE PER IDENTIFICARE QUANTO OCCUPA IL MESSAGGIO CHE VOGLIO INVIARE SULLA CODA TRANNE mtype, E' COME SE mtype FOSSE L'HEADER DEL MIO MESSAGGIO CHE INVIO SULLA MESSAGE QUEUE E I 5 ARRAY SONO IL BODY E IO VOGLIO SOLO LA DIMENSIONE DEL BODY E NON QUELLO DELL'ARRAY E QUINDI VA TOLTO L'HEADER

            // riempio cm.acks con i suoi 5 ack 
            int ackCounter;
            for(ackCounter = 0, counter = 0; counter < MAX_ACK_LIST && ackCounter < NDEVICES; counter++){//QUI MI RISCORRO L'ACK LIST PER ANDARMI A TROVARE GLI ACK CHE EFFETTIVAMENTE HANNO QUEL message_id
                ack = &ptr[counter];

                if(ack->message_id == found_message_id_list[occCounter]){//RICORDA CHE QUANDO ARRIVI QUI VUOL DIRE CHE UN MESSAGGIO E' ARRIVATO A TUTTI E QUINDI VAI A CERCARE NELL'ARRAY "ACK LIST" DOVE STANNO I message_id CHE SONO STATI SCRITTI 5 VOLTE E SOSTITUISCI IL NUMERO CHE C'E' DENTRO CON DEGLI 0 PER RENDERE LE CELLE SOVVRASCRIVIBILI
                    cm.acks[ackCounter] = *ack;//QUI FACCIO LA POPOLAZIONE DI ClientMessage DEL Acknowledgment CHE STO ANALIZZANDO IN QUEL MOMENTO LI. ackCounter QUANTI ACK HO GIA MESSO IN cm E GLI DICO CHE E' UGUALE ALL'*ack CHE STO ANALIZZANDO ADESSO E POI GLI DICO "L'ACK CHE STAI ANALIZZANDO ADESSO LO RESETTI METTENDOCI 0" IN MODO TALE DA VENIR SEGNATO COME SOVVRASCRIVIBILE E POI OVVIAMENTE MI AUMENTI IL CONTATORE PERCHE' SENNO' VADO A SCRIVERE SEMPRE NELLO STESSO acks DEL ClientMessage E INVECE IO DEVO RIEMPIRLI TUTTI E 5
                    ack->message_id = 0; //QUI RIEMPIO DI ZERI GLI ACK DEL ClientMessage CON GLI ACK CHE HO VISTO CHE SONO STATI RICEVUTI DA TUTTI 
                    ackCounter++;
                }
            }

            sortAck(&cm);//ORDINO GLI Acknowledgment, CONFRONTANDO IL timestamp, IN ORDINE DI TEMPO PERCHE' IL CLIENT VUOLE COSI

            writeMsgQueue(msgid, &cm, mSize, 0);//SCRIVO SULLA MESSAGE QUEUE IL cm OSSIA IL CLIENT MESSAGE 
        }
    }
}

void write_ack(Acknowledgment *ptr, Acknowledgment ack){//VIENE USATA DAL device.c PERCHE' E' LA FUNZIONE CHE A SCRIVERE SULL'ACK LIST UN Acknowledgment. COME PRIMO PARAMETRO PRENDO IL PUNTATORE AL SEGMENTO DI MEMORIA CONDIVISA CHE SAREBBE L'ACK LIST COME SECONDO UN Acknowledgment CHE DEVE ANDARE A SCRIVERE SULL'ACK LIST
    int counter = 0;

    Acknowledgment *a;

    for(; counter < MAX_ACK_LIST; counter++){//SCORRIAMO TUTTA LA ACK LIST
        a = &ptr[counter];//CAST AD Acknowledgment DELLA CELLETTA CHE STIAMO ANALIZZANDO IN QUESTO MOMENTO E GLI SCRIVIAMO LA STRUTTURINA DENTRO LA CELLA DELL'ARRAY PERO' GLIELA SCRIVE SE E SOLO SE TROVA UN BUCO LIBERO DOVE SCRIVERCI LA STRUTTURA. 
        if(a->message_id == 0){
            a->pid_sender = ack.pid_sender;
            a->pid_receiver = ack.pid_receiver;
            a->message_id = ack.message_id;
            a->timestamp = ack.timestamp;
            coloredPrintf("green", 0, "Writing on shm at %d: %d %d %d %ld\n", counter, a->pid_sender, a->pid_receiver, a->message_id, a->timestamp);
            break;
        }
    }
}

int acklist_countByMsgId(Acknowledgment *ptr, int message_id){
    int found = 0;
    int counter;

    Acknowledgment *a;

    for(counter = 0; counter < MAX_ACK_LIST; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id)
            found++;
    }

    return found;
}

int acklist_contains_message_id(Acknowledgment *ptr, int message_id){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < MAX_ACK_LIST; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id)
            return 1;
    }

    return -1;
}

// ritorna 1 se il device con pid pid_receiver ha gia scritto l ack identificato da message_id
int acklist_contains_tupla(Acknowledgment *ptr, int message_id, pid_t pid_receiver){//PRENDE IL PUNTATORE ALLA LISTA DI ACK (Acknowledgment *ptr), IL message_id E IL pid_receiver, QUESTI ULTIMI DUE SONO LA FAMOSA TUPLA CHE CONTROLLA SE C'E' UN DETERMINATO MESSAGGIO RICEVUTO DA UN DETERMINATO PID E MI SERVE PER VEDERE SE UN DEVICE HA RICEVUTO UN DETERMINATO MESSAGGIO OPPURE NO. CONTROLLA SE C'E' GIA' UN message_id CON UN DETERMINATO PID DENTRO L'ACK LIST. TUPLA --> PERCHE' FACCIO UN CONTROLLO INCROCIATO TRA ID E PID 
    int counter = 0;

    Acknowledgment *a;

    for(; counter < MAX_ACK_LIST; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id && a->pid_receiver == pid_receiver)
            return 1;
    }

    return -1;
}

//debug purposes
void read_acks(Acknowledgment *ptr){//SCORRE TUTTA L'ack_list E PRINT OGNI STRUTTURA CHE TROVA. STAMPA ACK PER ACK QUELLO CHE C'E' DENTRO LA MEMORIA CONDIVISA 
    int counter = 0;

    Acknowledgment *a;

    for(; counter < MAX_ACK_LIST; counter++){
        a = &ptr[counter];

        if(a->message_id != 0){
            printf("Found at %d: %d %d %d %ld\n", counter, a->pid_sender, a->pid_receiver, a->message_id, a->timestamp);
        }
    }
}