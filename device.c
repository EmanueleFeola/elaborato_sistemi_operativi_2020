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
#include <unistd.h>
#include <string.h>

int *shm_ptr;

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

void startDevice(int semid, int nchild, int shmid){
    printf("<device %d> created new device \n", getpid());
    
    // shm attach
    shm_ptr = (int *)shmat(shmid, NULL, 0);
    if(shm_ptr == (void *)-1)
        ErrExit("shmat failed\n");
    
    // open position file
    int fd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);

    // signal mask & signal handler
    setDeviceSignalMask();

    char nextLine[100] = {0};
    nextMove_t nextMove = {0, 0};

    while(1){
        waitP(semid, nchild);

        int oldMatrixIndex = nextMove.row * COLS + nextMove.col;

        fillNextLine(fd, nextLine);
        fillNextMove(nextLine, nchild, &nextMove);

        // printf("<device %d> my turn --> %d, %d --> ", getpid(), nextMove.row, nextMove.col);

        int matrixIndex = nextMove.row * COLS + nextMove.col;
        
        // mi sposto sulla cella giusta
        // se la cella prossima è occupata sto fermo (?)
        if(shm_ptr[matrixIndex] == 0){
            shm_ptr[oldMatrixIndex] = 0;     // libero precedente
            shm_ptr[matrixIndex] = getpid(); // occupo nuova
        }

        signalV(semid, nchild);
    }
}
