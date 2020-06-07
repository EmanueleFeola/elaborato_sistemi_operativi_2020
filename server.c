#include "server.h"

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

void initAckManager(){
    pid_t pid = fork();

    if (pid == -1){
        printf("<server> ackManager not created\n");
    }

    else if(pid == 0){
        startAckManager(semid, acklist_shmid, 123); // TODO: key from params, not magic number
    }
}

int main(int argc, char * argv[]) {
    printf("<server %d> created server\n", getpid());

    // set signal mask & handler
    setServerSigMask();

    // create sem set
    unsigned short semInitVal[] = {1, 0, 0, 0, 0, 1, 1}; // 5 sem per devices, 1 board, 1 ack_list 
    semid = create_sem_set(IPC_PRIVATE, NDEVICES + 2, semInitVal);
    
    // create board shared memory
    board_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(int) * ROWS * COLS);
    board_ptr = get_shared_memory(board_shmid, 0); 

    // create ack list shared memory
    acklist_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(Message) * 100); // TODO: niente magic numbers, #define ack_list_size 100

    initDevices();
    initAckManager();
    
    int iteration = 0;

    while(1){
        // ogni 2 secondi sblocco la board
        sleep(2);
        iteration++;
        semOp(semid, NDEVICES, -1); 
        printMatrix(board_ptr, iteration);
    }

    while (wait(NULL) != -1);

    freeResources();
}
