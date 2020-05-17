/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#include "server.h"
#include "device.h"

#include <signal.h> 
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>

int semid;
int board_shmid;
int *board_ptr;
int acklist_shmid;

void freeResources(){
    if (semctl(semid, 0 /*ignored*/, IPC_RMID, NULL) == -1)
        ErrExit("<server> semctl IPC_RMID failed");

    if(shmdt(board_ptr) == -1)
        ErrExit("<server> shmdt failed\n");

    if(shmctl(board_shmid, IPC_RMID, NULL) == -1)
        ErrExit("<server> shmctl failed\n");

    if(shmctl(acklist_shmid, IPC_RMID, NULL) == -1)
        ErrExit("<server> shmctl failed\n");
}

void setServerSigMask(){
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
            startDevice(semid, nchild, board_shmid, acklist_shmid);
        }
    }
}

int main(int argc, char * argv[]) {
    printf("<server %d> created server\n", getpid());

    // set signal mask & handler
    setServerSigMask();

    // create sem set
    unsigned short semInitVal[] = {0, 0, 0, 0, 1, 1}; 
    semid = create_sem_set(IPC_PRIVATE, NDEVICES + 1, semInitVal);
    
    // create board shared memory
    board_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(int) * ROWS * COLS);
    board_ptr = get_shared_memory(board_shmid, 0); 

    // create ack list shared memory
    acklist_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(Message) * 100);

    initDevices();
    
    int iteration = 0;

    while(1){
        // ogni 2 secondi sblocco la board
        sleep(2);
        printMatrix(board_ptr, iteration);
        iteration++;
        semOp(semid, NDEVICES, -1); 
    }

    while (wait(NULL) != -1);

    freeResources();
}
