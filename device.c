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

void checkMessages(int fd, Message messages[], int *nMessages){
    int bR = -1;

    Message msg;

    do{
        bR = read(fd, &msg, sizeof(Message));
        if(bR != 0){
            int lsb = msg.message_id & 1;
            if(lsb){
                // setto il lsb a 0
                msg.message_id &= 0xfffffffe; // non influenza i primi 31 bit, ma solo il 32esimo, che viene messo a 0
                if(acklist_contains_message_id(acklist_ptr, msg.message_id) == 1){
                    // avverti il clent che il message_id non è valido perchè c'è già
                    kill(msg.pid_sender, SIGUSR1);
                    return;
                }                
            }         

            int howmany = acklist_countByMsgId(acklist_ptr, msg.message_id);
            
            if(howmany != NDEVICES - 1) // if non sono l ultimo a cui mancava
                addAsHead(messages, nMessages, msg);
            else
                updateMyAcks(&msg, 1);
        }

    } while(bR > 0);
}

void updateMyAcks(Message *messagesToSend, int nMessages){
    int msgIndex = 0;

    for(; msgIndex < nMessages; msgIndex++){
        Acknowledgment ack;
        Message msg = messagesToSend[msgIndex];

        // Build ack
        time_t seconds = time(NULL); 
        ack.pid_sender = msg.pid_sender;
        ack.pid_receiver = getpid();
        ack.message_id = msg.message_id;
        ack.timestamp = seconds;

        semOp(semid_global, NDEVICES + 1, -1); // blocco la ack_list

        if(acklist_contains_tupla(acklist_ptr, msg.message_id, getpid()) == -1)
            write_ack(acklist_ptr, ack);    

        semOp(semid_global, NDEVICES + 1, 1); // sblocco la ack_list
    }
}

void startDevice(char *positionFilePath, int semid, int nchild, int board_shmid, int acklist_shmid){
    coloredPrintf("yellow", 0, "<device %d> created new device \n", getpid());

    // signal mask & signal handler
    setDeviceSignalMask();

    semid_global = semid; // mettere come parametro di funzione invece che come globale

    // get shm pointer
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

        coloredPrintf("default", 0, "%d %d %d msgs:", getpid(), pos.row, pos.col);
        printAllMessageId(messagesToSend, nMessages);

        signalV(semid, nchild);
    }
}
