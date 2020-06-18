#include "inc/device.h"
#include "utils/print_utils.h"
#include "utils/array_utils.h"
#include "inc/ackManager.h"

int semid_global;
int *board_ptr;
Acknowledgment *acklist_ptr;
char fifoPath[25] = {0};
int positionFd;

void setDeviceSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGTERM, deviceSigHandler);
}

void deviceSigHandler(int sig){
    coloredPrintf("yellow", 0, "<device %d> terminating\n", getpid());

    free_shared_memory(board_ptr);
    free_shared_memory(acklist_ptr);
    unlink_fifo(fifoPath);

    if(close(positionFd) == -1)
        ErrExit("<device> closing file failed\n");

    exit(0);
}

// mette dentro messages il nuovo messaggio letto e shifta a destra tutti gli altri
void checkMessages(int fd, Message messages[], int *nMessages){
    int bR = -1;

    Message msg;

    do{
        bR = read(fd, &msg, sizeof(Message));
        if(bR != 0){
            int lsb = msg.message_id & 1;
            if(lsb){//SE E' == 1 VUOL DIRE CHE E' DEL CLIENT E LO MODIFICO A 0
                // setto il lsb a 0
                msg.message_id &= 0xfffffffe; // non influenza i primi 31 bit, ma solo il 32esimo, che viene messo a 0
                if(acklist_contains_message_id(acklist_ptr, msg.message_id) == 1){//IL DEVICE CONTROLLA DENTRO L'ack_list SE C'E' GIA' QUALCHE Acknowledgment CHE HA QUESTO msg.message_id E SE C'E' GIA' ALLORA AVVERTE IL CLIENT CHE IL message_id NON E' VALIDO QUINDI GLI MANDO UN SEGNALE SIGUSR1
                    kill(msg.pid_sender, SIGUSR1);
                    return;
                }                
            }         

            int howmany = acklist_countByMsgId(acklist_ptr, msg.message_id);//CONTO QUANTI SONO GLI Acknowledgment CON QUEL DETERMINATO message_id DENTRO L'ack_list PER VEDERE SE SONO L'ULTIMO PERCHE' SE NON SONO L'ULTIMO AGGIUNGO IL MIO MESSAGGIO ALL'ARRAY DI MESSAGGI TRAMITE addAsHead.SE SONO L'ULTIMO E' INUTILE CHE LO METTO NEL MIO ARRAY DI MESSAGGI PERCHE' DOPO LO DOVREI TOGLIERE PERCHE' NON DEVO INVIARLO PIU' A NESSUNO E QUINDI LO AGGIUNGO ALL'ack_list TRAMITE updateMyAcks
            
            if(howmany != NDEVICES - 1) // if non sono l ultimo a cui mancava
                addAsHead(messages, nMessages, msg);
            else
                updateMyAcks(&msg, 1);//RICHIAMO updateMyAcks DIRETTAMENTE SU &msg E GLI PASSO 1 COME nMessages. QUESTO NON FA ALTRO CHE AGGIUNGERE ALL'ack_list DIRETTAMENTE QUESTO MESSAGGIO &msg
        }

    } while(bR > 0);
}

void updateMyAcks(Message *messagesToSend, int nMessages){//Message *messagesToSend --> PUNTATORE ALLA STRUTTURA Message MA ALLA FIN FINE E' UN ARRAY PERCHE' CI SONO PIU' STRUTTURE Message ALL'INTERNO DI messagesToSend
    int msgIndex = 0;

    for(; msgIndex < nMessages; msgIndex++){//SCORRO L'ARRAY DEI MESSAGGI CHE E' CONTENUTO IN QUESTO DEVICE E SE NON LO HO GIA' MESSO NELL'ACK LIST (TRAMITE L'ISTRUZIONE if(acklist_contains(acklist_ptr, msg.message_id, getpid()) == -1) ALLORA LO SCRIVO ). QUINDI SE HA RICEVUTO MESSAGGI DA ALTRI DEVICE (E SE NE ACCORGE NEL PASSAGGIO PRIMA QUANDO FACCIO LA checkMessages) LI ANDRA' A SCRIVERE NELL'ACK LIST
        Acknowledgment ack;
        Message msg = messagesToSend[msgIndex];

        //IN QUESTE 5 RIGHE SUCCESSIVE POPOLO LA STRUTTURA ack (CASTATA AD Acknowledgment) CON LA STRUTTURINA DENTRO L'ARRAY messagesToSend 
        time_t seconds = time(NULL); //MI PRENDO IL NUMERONE DEI SECONDI E LO METTO DENTRO seconds, LO INVIO AL CLIENT E POI IL CLIENT LO CONVERTE
        ack.pid_sender = msg.pid_sender;
        ack.pid_receiver = getpid();
        ack.message_id = msg.message_id;
        ack.timestamp = seconds;

        semOp(semid_global, NDEVICES + 1, -1); //blocco la ack_list --> RICORDA!!! QUANDO DOBBIAMO ACCEDERE AD UNA RISORSA CONDIVISA DOBBIAMO ASSICURARCI SEMPRE DI ESSERE GLI UNICI CHE VI STANNO ACCEDENDO. SE MI RENDO CONTO CHE C'E' GIA' QUALCUNO CHE HA BLOCCATO IL SEMAFORO ALLORA MI FERMO E RIPARTO QUANDO EFFETTIVAMENTE RIUSCIRO' A FARE -1 SUL SEMAFORO (QUINDI QUEL QUALCUN ALTRO HA SBLOCCATO IL SEMAFORO E RIESCO A FARE -1 PER BLOCCARLO PER ME STESSO) ED ENTRERO' QUINDI NELLA "SEZIONE CRITICA" E LI SONO SICURO CHE SONO SOLAMENTE IO CHE ACCEDO ALLA RISORSA CONDIVISA CHE IN QUESTO CASO E' APPUNTO L'ack_list.

        if(acklist_contains_tupla(acklist_ptr, msg.message_id, getpid()) == -1)//CONTROLLA SE C'E' GIA' DENTRO L'ack_list UN CERTO message_id RICEVUTO DA UN CERTO DEVICE. QUINDI CONTROLLO SE IL DEVICE IDENTIFICATO DAL PID HA GIA' RICEVUTO IL message_id IDENTIFICATO DA msg.message_id, QUINDI PRATICAMENTE GLI STO DICENDO (DATO CHE COME PID GLI MANDO getpid() OSSIA IL MIO PID) "CONTROLLA CHE IO STESSO NON ABBIA GIA' RICEVUTO QUESTO MESSAGE ID IN PRECEDENZA" E SE NON LO HO RICEVUTO E QUINDI SE MI RITORNA -1 (QUINDI SE NON LO HO RICEVUTO) ALLORA FAI LA write_ack E GLI PASSO IL PARAMETRO ack CHE E' L'Acknowledgment CHE CI SIAMO CREATI PRIMA. SE RITORNA 1 VUOL DIRE CHE LA TUPLA E' GIA' NELL'ack_list E SE MI RITORNA -1 NON C'E'
            write_ack(acklist_ptr, ack);// SE VEDO CHE MI MANCA UN MESSAGGIO DA SCRIVERE NELL'ack_list LO SCRIVO TRAMITE write_ack IL QUALE SE VEDE CHE LA CELLETTA E' A 0 METTERA' DENTRO IL NOSTRO message_id  

        semOp(semid_global, NDEVICES + 1, 1); // sblocco la ack_list
    }
}

void startDevice(char *positionFilePath, int semid, int nchild, int board_shmid, int acklist_shmid){//dobbiamo passare come parametri sia la memoria condivisa della board che la memoria condivisa dell'ack_manager perche' devono entrambe interagire con i devices
    coloredPrintf("yellow", 0, "<device %d> created new device \n", getpid());//mi stampa tutti i device insieme invece che uno alla volta perche' nel while(1) sotto faccio una waitP

    // signal mask & signal handler
    setDeviceSignalMask();//ri-setto la maschera di segnali perche' essendo device.c un processo a parte rispetto al server devo settare la maschera. Di default un processo figlio eredita quella del padre pero' noi vogliamo che i figli si comportino in maniera differente perche' se utilizziamo la stessa maschera del padre vuol dire che quando un figlio muore lui fa comunque una kill verso tutti i suoi devices/figli che pero' non ne ha di figli e quindi non ha senso --> sono due processi diversi e i figli devono rispondere in maniera diversa in base ai segnali che ricevono

    semid_global = semid;

    // get shm pointer
    board_ptr = get_shared_memory(board_shmid, 0);
    acklist_ptr = (Acknowledgment *)get_shared_memory(acklist_shmid, 0);//VALE PER ENTRAMBE LE SHARED MEMORY APPENA CREATE --------> stesso motivo per cui lo ho fatto nel server, nel server avevo bisogno di un modo per agganciarmi alla shared memory e devo farlo anche qui perche' senno' non posso accedere alla shared memory. Devo fare l'attach anche qui xk senno' non potrei lavorare sui devices per spostarli nella matrice (infatti se non facessi l'attach anche qui il server mi stamperebbe matrici di soli 0, perche' se i device non fanno l'attach e non scrivono la shared memory lascia tutti 0, quando fai una shmget inizializzo un segmento di memoria condivisa con tutti 0), questo shm_ptr deve essere uguale per tutti per puntare alla stessa shared memory. RIASSUMENDO IL MOTIVO PER CUI FACCIO shmat ANCHE QUI E' XK QUESTO device.c RAPPRESENTE IL DEVICE E IL DEVICE E' UN PROCESSO A PARTE, E' FIGLI DEL SERVER PERO' GLI SPAZI DI INDIRIZZAMENTO SONO DIVERSI QUINDI VA FATTO shmat PURE QUI. shm_ptr deve essere uguale per tutti per puntare alla stessa shared memory

    // create fifo
    sprintf(fifoPath, "%s%d", fifoBasePath, getpid());
    create_fifo(fifoPath);
    int fifoFd = get_fifo(fifoPath, O_RDONLY | O_NONBLOCK);

    // list of messages to send
    Message messagesToSend[MAX_NMESSAGES]; // al massimo può contenere MAX_NMESSAGES messaggi alla volta! ogni device ha il suo array che contiene i suoi messaggi che deve inviare agli altri. OGNI DEVICE HA IL PROPRIO ARRAY DI MESSAGGImessaggi alla volta!
    int nMessages = 0;// mi conta quanti messaggi ci sono nell array

    // open position file
    // positionFd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);
    positionFd = open(positionFilePath, O_RDONLY, 0 /* ignored */);

    // board position buffers
    char nextLine[100] = {0};//buffer/stringa di 100 caratteri che rappresenta la dimensione della riga successiva (lavoriamo sulle righe singolarmente invece che su tutte insieme). e' come se dessi 100 caratteri ad ogni riga, sono 19 i caratteri, si poteva mettera anche a 19 volendo
    Position pos = {0, 0};// vado a inizializzare i valori row e col della struttura contenuta in device.h a 0

    while(1){
        waitP(semid, nchild);        
      
        sendMessages(board_ptr, acklist_ptr, pos, messagesToSend, &nMessages);//manda i messaggi agli altri devices vicini 
        checkMessages(fifoFd, messagesToSend, &nMessages);//controllo sulla fifo che non mi siano arrivati altri messaggi nuovi 
        updateMyAcks(messagesToSend, nMessages);//aggiorna ack list in caso di ricezione di nuovi messaggi (giustamente se un messaggio e' stato ricevuto da un nuovo device bisogna buttarlo dentro nella lista degli ack)
        
        // movimento sulla board
        int oldMatrixIndex = pos.row * COLS + pos.col;//all'inizio e' 0

        fillNextLine(positionFd, nextLine);//passo fd del file_posizioni.txt e la nextLine che e' un buffer di caratteri pero' quando torno dalla fillNextLine dentro il secondo argomento "nextLine" trovo la prossima riga xk dentro fillNextLine vado ad agire sul buffer che ho passato come parametro e lo popolo con la nextLine. QUINDI ALLA PRIMA ESECUZIONE DI QUESTA fillNextLine ABBIAMO IL PUNTATORE AL PRIMO CARATTERE DELLA PRIMA LINEA DERIVATO DALLA open FATTA SOPRA "int fd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);" E CON LA fillNextLine ANDRA' ALL'ULTIMO CARATTERE DELLA PRIMA LINEA.

        //MOLTO IMPORTANTE !!! IL SECONDO ARGOMENTO nextLine E' COME DIRE CHE SONO PRONTO A LEGGERE LA LINEA SUCCESSIVA, ALLA SECONDA ESECUZIONE DEL while NOI ABBIAMO FINITO DI LEGGERE CARATTERE PER CARATTERE LA PRIMA RIGA "(read(fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)". IL PROCESSO HA MEMORIA DI DOVE E' ARRIVATO A LEGGERE file AUTOMATICAMENTE (NOI NON DOBBIAMO FARE NIENTE !!!!!!!!!!) RICOMINCIA DAL PUNTO IN CUI SI ERA FERMATO PRIMA E VA AVANTI TRANQUILLAMENTE CON LA SECONDA RIGA
        fillNextPos(nextLine, nchild, &pos);

        //TEORIA !!! --> sarebbe una matrice bidimensionale rappresentata come un array monodimensionale perchè alla fine una matrice è un array di array, quindi in sostanza è un array

        int currentMatrixIndex = pos.row * COLS + pos.col;//qui effettivamente per quanto riguarda il primo figlio ci troveremo in 0,0 quindi 0 * 5 + 0
        
        if(board_ptr[currentMatrixIndex] == 0){//se il valore contenuto nella cella e' 0
            board_ptr[oldMatrixIndex] = 0;     // libero precedente
            board_ptr[currentMatrixIndex] = getpid(); // occupo nuova
        }
        else{//se e' occupato ripristino i valori precedenti perche' non mi sono mosso --> devo ripristinare nextMove con i valori precedenti
            pos.row = oldMatrixIndex / COLS;
            pos.col = oldMatrixIndex % COLS;
            //esempio: se oldMatrixIndex = 8; allora facendo 8 / COLS = 8 / 5 = 1 con resto 3 mi ricalcolo la row (8/5), col(8%5) precedente
        }

        coloredPrintf("default", 0, "%d %d %d msgs:", getpid(), pos.row, pos.col);
        printAllMessageId(messagesToSend, nMessages);

        signalV(semid, nchild);
    }
}
