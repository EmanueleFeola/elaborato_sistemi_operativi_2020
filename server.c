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

int semid;

void freeResources(){
    // per adesso c'è solo il semaforo
    // più avanti ci sarà anche fifo, shm

    if (semctl(semid, 0 /*ignored*/, IPC_RMID, NULL) == -1)
        ErrExit("semctl IPC_RMID failed");
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

void initDevices(int semid){
    int nchild = 0;
    for(; nchild < NDEVICES; nchild++){
        pid_t pid = fork();

        if (pid == -1){
            printf("<server> child %d not created\n", nchild);
        }

        else if(pid == 0){
            startDevice(semid, nchild);
        }
    }
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

    initDevices(semid);
    
    while(1){
        // ogni 2 secondi sblocco la board 
        semOp(semid, 5, -1); 
        sleep(2);
    }

    while (wait(NULL) != -1);

    freeResources();
}
