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
    sigdelset(&set, SIGINT); // for debug purposes only
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGINT, serverSigHandler);
    signal(SIGTERM, serverSigHandler);
}

void serverSigHandler(int sig){
    kill(-getpid(), SIGTERM); // kill all children

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
            startDevice(positionFilePath, semid, nchild, board_shmid, acklist_shmid);
    }
}

void initAckManager(int msgQueueKey){
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
    setServerSigMask();

    // create sem set
    unsigned short semInitVal[] = {1, 0, 0, 0, 0, 1, 1}; // 5 sem per devices, 1 board, 1 ack_list 
    semid = create_sem_set(IPC_PRIVATE, NDEVICES + 2, semInitVal);
    
    // allocate and get board shared memory
    board_shmid = alloc_shared_memory(IPC_PRIVATE, sizeof(int) * ROWS * COLS);
    board_ptr = get_shared_memory(board_shmid, 0); 

    // allocate ack list shared memory
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
