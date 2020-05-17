#include "device.h"
#include "defines.h"
#include "fifo.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "err_exit.h"

#include <sys/shm.h>
#include <sys/stat.h>
#include <signal.h> 
#include <errno.h>
#include <time.h>

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

void startDevice(int semid, int nchild, int board_shmid, int acklist_shmid){
    // TODO: eliminare magic numbers!

    printf("<device %d> created new device \n", getpid());

    // signal mask & signal handler
    setDeviceSignalMask();

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
    positionFd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);

    // board position buffers
    char nextLine[100] = {0};
    Position pos = {0, 0};

    while(1){
        waitP(semid, nchild);

        int oldMatrixIndex = pos.row * COLS + pos.col;
        
        checkMessages(fifoFd, messagesToSend, &nMessages);

        read_acks(acklist_ptr);

        if(nMessages > 0){
            Acknowledgment ack;
            Message msg = messagesToSend[0];

            time_t seconds = time(NULL); 
            ack.pid_sender = msg.pid_sender;
            ack.pid_receiver = getpid();
            ack.message_id = msg.message_id;
            ack.timestamp = seconds;
            if(acklist_contains(acklist_ptr, ack) == -1)
                write_ack(acklist_ptr, ack);    
        }

        sendMessages(board_ptr, acklist_ptr, pos, messagesToSend, &nMessages);

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

        signalV(semid, nchild);
    }
}
