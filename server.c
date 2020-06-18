#include "inc/server.h"

int semid;
int board_shmid;
int *board_ptr;
int acklist_shmid;

void freeResources(){
    delete_sem_set(semid);
    free_shared_memory(board_ptr);
    remove_shared_memory(board_shmid);
    remove_shared_memory(acklist_shmid);
}

void setServerSigMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT); //CTRL-C 
    sigdelset(&set, SIGTERM);//kill --> DA TERMINALE SI SCRIVE MANUALMENTE SE NON VUOI FARE CTRL-C (CTRL-C PIU' COMODO)
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGINT, serverSigHandler);
    signal(SIGTERM, serverSigHandler);
}

void serverSigHandler(int sig){//QUANDO FACCIO QUESTA MANDO UN "SIGTERM" A TUTTI I CHILD (DEVICE E ackManager) E POI FACCIO freeResources (QUEST'ULTIMA ELIMINA IL SEMAFORO, PRIMA STACCA LA board E POI LA ELIMINA MENTRE ELIMINA DIRETTAMENTE ack_list PERCHE' DI QUELLO NON HA FATTO L'ATTACH, IL SERVER LO HA SOLAMENTE CREATO ---> IL SERVER NON UTILIZZA ack_list)
    kill(-getpid(), SIGTERM); // manda un kill a tutti i figli (-getpid vuol dire di mandare il kill a tutti i processi dello stesso gruppo)

    freeResources();

    coloredPrintf("yellow", 0, "<server %d> server exits\n", getpid());

    exit(0);
}

void initDevices(char *positionFilePath){
    int nchild = 0;
    for(; nchild < NDEVICES; nchild++){
        pid_t pid = fork();

        if (pid == -1)
            coloredPrintf("red", 0, "<server> child %d not created\n", nchild);

        else if(pid == 0)
            startDevice(positionFilePath, semid, nchild, board_shmid, acklist_shmid);// alla "startDevice" passo il set di semafori che e' uguale per tutti i device, nchild che all'inizio e' 0 alla prima esecuzione del for fino ad arrivare a 4 ossia il 5 figlio/device (sto dicendo al device quale figlio e' in modo tale da avere una "coscienza propria")e la shared memory.
    }
}

void initAckManager(int msgQueueKey){//fa una fork per il figlio ack_manager
    pid_t pid = fork();

    if (pid == -1)
        coloredPrintf("red", 0, "<server> ackManager not created\n");

    else if(pid == 0)
        startAckManager(semid, acklist_shmid, msgQueueKey);
}

int main(int argc, char * argv[]) {
    coloredPrintf("yellow", 0, "<server %d> created server\n", getpid());

    // check params
    if(argc != 3)
        ErrExit("<server> incorrect params: server usage is ./server msg_queue file_posizioni");

    int msgQueueKey = atoi(argv[1]);
    if (msgQueueKey <= 0)
        ErrExit("<server> incorrect params: msg_queue_key must be greater than zero");

    char *positionFilePath = argv[2];

    // set signal mask & handler
    setServerSigMask();//IL SERVER E' IL PRIMO CHE RICEVE IL SIGTERM E POI VIENE PROPAGATO AI DEVICE E ALL'ACK ackManager. OVVIAMENTE DOBBIAMO SETTARE CHE ANCHE I DEVICE E ackManager POSSANO GESTIRE QUESTO SIGTERM E GIUSTAMENTE QUANDO UN PROCESSO MUORE DEVE LIBERARE LE PROPRIE RISORSE E PURE I FIGLI DEVONO QUINDI POTER FARE QUESTO

    // create sem set
    unsigned short semInitVal[] = {1, 0, 0, 0, 0, 1, 1}; // 5 sem per devices, 1 board, 1 ack_list 
    semid = create_sem_set(IPC_PRIVATE, NDEVICES + 2, semInitVal);
    
    // allocate and get board shared memory
    board_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(int) * ROWS * COLS);//devo moltiplicare anche per la dimensione in bytes di un intero che e' 4 oltre che 5 * 5 xk senno' mi creerebbe una shared memory di 25 bytes che non e' sufficiente. IPC_PRIVATE --> l'ipc object non e' privato in questo singolo processo ma puo' essere acceduto da altri processi che hanno ereditato (figli --> devices) quell'ipc object dal parent (server). L'IPC_PRIVATE ci garantisce che nessun altro processo non relazionato abbia la stessa chiave. un ipc object creato con IPC_PRIVATE non lo posso condividere con altri processi eseguiti a partire da altri programmi ossia processi che non hanno nessuna relazione tra di loro.
    board_ptr = get_shared_memory(board_shmid, 0);//nella riga 97 quando noi facciamo "alloc_shared_memory" istanziamo questa shared memory che e' da qualche parte in memoria pero' dobbiamo dirgli di attaccare questo spazio di indirizzamento della shared memory al nostro processo tramite "shmat", quindi noi estendiamo lo spazio di indirizzamento virtuale del nostro processo attaccandogli lo spazio della shared memory xk senno' non possiamo accederci non sapendo dove e'. LA shmat CI RITORNA UN PUNTATORE (int *) IN MODO DA POTER ACCEDERE ALLA ZONA DI MEMORIA CREATA CON shmget 

    //create ack list shared memory, questa shared memory andra' condivisa con tutti i devices con la dimensione di 100 messaggi
    acklist_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(Message) * MAX_ACK_LIST);

    initDevices(positionFilePath);
    initAckManager(msgQueueKey);
    
    int iteration = 0;

    while(1){
        // ogni 2 secondi sblocco la board
        sleep(2);
        iteration++;
        semOp(semid, NDEVICES, -1); 
        // printMatrix(board_ptr, iteration);
        coloredPrintf("cyan", 0, "##### step %d: devices positions #####\n", iteration);
    }

    while (wait(NULL) != -1);

    freeResources();
}
