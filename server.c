/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#include "server.h"
#include "device.h"

// signals imports
#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>

#include <sys/wait.h>
#include <stdlib.h>

#include <sys/sem.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int semid;
int shmid;
int *shm_ptr;

void freeResources(){
    if (semctl(semid, 0 /*ignored*/, IPC_RMID, NULL) == -1)
        ErrExit("<server> semctl IPC_RMID failed");

    if(shmdt(shm_ptr) == -1)
        ErrExit("<server> shmdt failed\n");

    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("<server> shmctl failed\n");
}

void setServerSignalMask(){
    // added SIGINT handler for debug purposes
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGINT, serverSigHandler);
    signal(SIGTERM, serverSigHandler);
}

/*
 * Free resources and children on SIGTERM / SIGINT (debug purposes only)
 */
void serverSigHandler(int sig){
    kill(-getpid(), SIGTERM);

    freeResources();

    // terminate
    printf("<server %d> server exits\n", getpid());

    exit(0);
}

void initDevices(){
    int nchild = 0;
    for(; nchild < NDEVICES; nchild++){
        pid_t pid = fork();

        if (pid == -1){
            printf("<server> child %d not created\n", nchild);
        }

        else if(pid == 0){
            startDevice(semid, nchild, shmid);
        }
    }
}

void printMatrix(int iteration){
    int row, col;

    printf("Iteration: %d\n", iteration);

    char divider[8 * COLS]; // 8 è il padding tra celle
    memset(divider, '-', sizeof(divider));

    printf("%s\n", divider);

    for(row = 0; row < ROWS; row++){
        for(col = 0; col < COLS; col++){
            int offset = row*COLS+col;
            printf("%8d", shm_ptr[offset]);
        }
        printf("\n");
    }
    
    printf("%s\n\n", divider);
}

int main(int argc, char * argv[]) {
    printf("<server %d> created server\n", getpid());

    setServerSignalMask();

    // create semaphores set
    semid = semget(IPC_PRIVATE, NDEVICES + 1, S_IRUSR | S_IWUSR);
    if (semid == -1)
        ErrExit("semget failed");

    // ultimo semaforo è quello di accesso alla board (1 -> bloccato, 0 -> sbloccato)
    unsigned short semInitVal[] = {0, 0, 0, 0, 1, 1}; 
    union semun arg;
    arg.array = semInitVal;

    if (semctl(semid, 0, SETALL, arg) == -1)
        ErrExit("semctl SETALL failed");

    // create board shared memory
    shmid = shmget(1, sizeof(int) * ROWS * COLS, IPC_CREAT | S_IRUSR | S_IWUSR);
    if(shmid == -1){
        ErrExit("shared memory allocation failed\n");
    }

    shm_ptr = (int *)shmat(shmid, NULL, 0);

    initDevices();
    
    int iteration = 0;

    while(1){
        // ogni 2 secondi sblocco la board
        semOp(semid, NDEVICES, -1); 
        sleep(2);
        printMatrix(iteration);
        iteration++;
    }

    while (wait(NULL) != -1);

    freeResources();
}
