#include "device.h"
#include "utils/print_utils.h"

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
    printf("<device %d> terminating\n", getpid());

    if(shmdt(board_ptr) == -1)
        ErrExit("<device> shmdt failed\n");

    if(shmdt(acklist_ptr) == -1)
        ErrExit("<device> shmdt failed\n");

    if(unlink(fifoPath) == -1)
        ErrExit("<device> unlink fifo failed\n");
    
    if(close(positionFd) == -1)
        ErrExit("<device> closing file failed\n");

    exit(0);
}

void updateMyAcks(Message *messagesToSend, int nMessages){
    int msgIndex = 0;

    for(; msgIndex < nMessages; msgIndex++){
        Acknowledgment ack;
        // --> se mi rimangono buchi nell array sono cazzi --> devo compattare l array ogni volta che elimino un elem
        Message msg = messagesToSend[msgIndex];

        // Build ack
        time_t seconds = time(NULL); 
        ack.pid_sender = msg.pid_sender;
        ack.pid_receiver = getpid();
        ack.message_id = msg.message_id;
        ack.timestamp = seconds;

        // if sem NDEVICES + 2 --> lo occupo --> scrivo 
        // else occupato --> aspetto, lo occupo --> scrivo 
        semOp(semid_global, NDEVICES + 1, -1); // blocco la ack_list

        // se non l ho gia messo nell ack list, lo mando
        if(acklist_contains(acklist_ptr, msg.message_id, getpid()) == -1)
            write_ack(acklist_ptr, ack);    
        
        semOp(semid_global, NDEVICES + 1, 1); // sblocco la ack_list
    }
}

void startDevice(char *positionFilePath, int semid, int nchild, int board_shmid, int acklist_shmid){
    // TODO: eliminare magic numbers!
    printf("<device %d> created new device \n", getpid());

    // signal mask & signal handler
    setDeviceSignalMask();

    semid_global = semid; // mettere come parametro di funzione invece che come globale

    // shm attach
    board_ptr = get_shared_memory(board_shmid, 0);
    acklist_ptr = (Acknowledgment *)get_shared_memory(acklist_shmid, 0);

    // create fifo
    sprintf(fifoPath, "%s%d", fifoBasePath, getpid());
    create_fifo(fifoPath);
    int fifoFd = get_fifo(fifoPath, O_RDONLY | O_NONBLOCK);

    // list of messages to send
    Message messagesToSend[MAX_NMESSAGES]; // al massimo può contenere MAX_NMESSAGES messaggi alla volta!
    int nMessages = 0;                     // mi conta quanti messaggi ci sono nell array

    // open position file
    // positionFd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);
    positionFd = open(positionFilePath, O_RDONLY, 0 /* ignored */);

    // board position buffers
    char nextLine[100] = {0};
    Position pos = {0, 0};

    while(1){
        waitP(semid, nchild);
        
        // manda i miei msg, controlla nuovi msg, aggiorna ack list in caso ho ricevuto nuovi msg
        sendMessages(board_ptr, acklist_ptr, pos, messagesToSend, &nMessages);
        checkMessages(fifoFd, messagesToSend, &nMessages);
        updateMyAcks(messagesToSend, nMessages);
        
        // movimento sulla board
        int oldMatrixIndex = pos.row * COLS + pos.col;

        fillNextLine(positionFd, nextLine);
        fillNextPos(nextLine, nchild, &pos);

        int currentMatrixIndex = pos.row * COLS + pos.col;
        
        if(board_ptr[currentMatrixIndex] == 0){
            board_ptr[oldMatrixIndex] = 0;     // libero precedente
            board_ptr[currentMatrixIndex] = getpid(); // occupo nuova
        }
        else{
            pos.row = oldMatrixIndex / COLS;
            pos.col = oldMatrixIndex % COLS;
        }

        printf("%d %d %d msgs:", getpid(), pos.row, pos.col);
        printAllMessageId(messagesToSend, nMessages);

        signalV(semid, nchild);
    }
}
