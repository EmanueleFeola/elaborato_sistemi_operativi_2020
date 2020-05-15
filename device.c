#include "device.h"

#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>

#include <sys/shm.h>

#include "semaphore.h"
#include "defines.h"
#include "err_exit.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int *shm_ptr;
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

    if(shmdt(shm_ptr) == -1)
        ErrExit("<device> shmdt failed\n");

    if(unlink(fifoPath) == -1)
        ErrExit("<device> unlink fifo failed\n");
    
    if(close(positionFd) == -1)
        ErrExit("<device> closing file failed\n");

    exit(0);
}

/*
 * wait until it is my turn and board is available 
 */
void waitP(int semid, int nchild){
    semOp(semid, (unsigned short)nchild, -1); // aspetto il mio turno
    semOp(semid, (unsigned short) 5, 0); // aspetto che board vada a 0
}

/*
 * signal to other devices that I completed my tasks
 */
void signalV(int semid, int nchild){
    if (nchild > 0)
        semOp(semid, (unsigned short)(nchild - 1), 1); // sblocco il device successivo
    else{
        semOp(semid, (unsigned short) NDEVICES, 1); // blocco la board (1 -> bloccato, 0 -> sbloccato)
        semOp(semid, (unsigned short) NDEVICES - 1, 1); // sblocco il primo
        // check per errore accesso board
        printf("\n");
    }
}

void readFifo(int fd){
    int bR = -1;

    Message msg;

    do{
        bR = read(fd, &msg, sizeof(Message));
        if(bR != 0)
            printf("<device %d> Read:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            message: |%s|\n\
            max_distance: %d\n",
            getpid(), msg.pid_sender, msg.pid_receiver, msg.message_id, msg.message, msg.max_distance);

    } while(bR > 0);

}

void startDevice(int semid, int nchild, int shmid){
    printf("<device %d> created new device \n", getpid());

    // signal mask & signal handler
    setDeviceSignalMask();

    // shm attach
    shm_ptr = (int *)shmat(shmid, NULL, 0);
    if(shm_ptr == (void *)-1)
        ErrExit("<device> shmat failed\n");

    // create fifo
    char *basePath = "/tmp/dev_fifo.";
    strcat(fifoPath, basePath);
    
    char pidbuf[5];
    sprintf(pidbuf, "%d", getpid());
    strcat(fifoPath, pidbuf);

    printf("<device %d> Created fifo: %s\n", getpid(), fifoPath);

    int res = mkfifo(fifoPath, S_IRUSR | S_IWUSR);
    if(res == -1)
        ErrExit("<device> failed creating fifo\n");
    int fifoFd = open(fifoPath, O_RDONLY | O_NONBLOCK); // non lo abbiamo usato in classe
    if(fifoFd == -1)
        ErrExit("<device> open fifo failed\n");

    // open position file
    positionFd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);

    // board position buffers
    char nextLine[100] = {0};
    nextMove_t nextMove = {0, 0};

    while(1){
        waitP(semid, nchild);

        readFifo(fifoFd);
        
        int oldMatrixIndex = nextMove.row * COLS + nextMove.col;

        fillNextLine(positionFd, nextLine);
        fillNextMove(nextLine, nchild, &nextMove);

        // printf("<device %d> my turn --> %d, %d --> ", getpid(), nextMove.row, nextMove.col);

        int matrixIndex = nextMove.row * COLS + nextMove.col;
        
        // mi sposto sulla cella giusta
        // se la cella prossima è occupata sto fermo (?)
        if(shm_ptr[matrixIndex] == 0){
            shm_ptr[oldMatrixIndex] = 0;     // libero precedente
            shm_ptr[matrixIndex] = getpid(); // occupo nuova
        }
        else{
            // se è occupato ripristino i valori precedenti
            // perchè NON mi sono mosso 
            // --> devo ripristinare nextMove con i valori precedenti
            nextMove.row = oldMatrixIndex / COLS;
            nextMove.col = oldMatrixIndex % COLS;
            // esempio: se oldMatrixIndex = 8,
            // allora facendo 8 / COLS = 8 / 5 = 1 con resto 3
            // mi ricalcolo la row(8/5), col(8%5) precedente
        }


        signalV(semid, nchild);
    }
}
