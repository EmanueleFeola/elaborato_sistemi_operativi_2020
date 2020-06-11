#include "client.h"
#include <time.h>

void setClientSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGUSR1, clientSigHandler);
}

void clientSigHandler(int sig){
    printf("<client %d> message_id is not unique, terminating\n", getpid());
    exit(0);
}

int main(int argc, char * argv[]) {
    setClientSignalMask();

    if(argc != 2 && argc != 6)
        ErrExit("<client> incorrect params: server usage is\n./client msg_queue\nor\n./client msg_queue_key pid_receiver message_id message max_distance");

    int msgQueueKey = atoi(argv[1]);
    if (msgQueueKey <= 0)
        ErrExit("<client> incorrect params: msg_queue_key must be greater than zero");

    Message msg;
    msg.pid_sender = getpid();

    // fifo path
    char fifoBasePath[20] = "/tmp/dev_fifo.";
    char fifoPath[50] = {0};
    sprintf(fifoPath, "%s", fifoBasePath);

    if(argc > 2){
        strcat(fifoPath, argv[2]);
        msg.pid_receiver = atoi(argv[2]);
        msg.message_id = atoi(argv[3]);

        memcpy(msg.message, argv[4], strlen(argv[4]));
        msg.message[strlen(argv[4])] = '\0';
        
        msg.max_distance = atoi(argv[5]);

        if(msg.pid_receiver < 1 || msg.message_id < 0 || msg.max_distance < 1){
            printf("<client %d> script input < 0\n", getpid());
            exit(1);
        }
    }
    else{
        // se scambio l ordine della scanf di message e una delle altre scanf non funzia.....dc
        // TODO: risolvere ...
        printf("<client %d> Insert message: \n", getpid());
        scanf("%[^\n]%*c", msg.message);

        do{
            char pidString[7] = {0};
            printf("<client> Insert destination PID:\n");
            scanf("%s", pidString);
            sprintf(fifoPath, "%s%s", fifoPath, pidString);
            msg.pid_receiver = atoi(pidString);
        } while(msg.pid_receiver < 1);

        do{
            printf("<client %d> Insert message id: \n", getpid());
            scanf("%d", &msg.message_id);
        } while(msg.message_id < 0);

        do{
            printf("<client %d> Insert max distance: \n", getpid());
            scanf("%d", &msg.max_distance);
        } while(msg.max_distance < 1);
    }

    // trick per rendere message_id univoco
    // il bit meno significativo è un 1 se il sender è un client
    // il bit meno significativo è uno 0 se il sender è un device
    msg.message_id <<= 1;
    msg.message_id |= 1;

    // printf("il bit nascosto è: %d\n", message_id&1);

    // message_id >>= 1; // ripristino il message_id originale
    // printf("message_id originale %d\n", message_id);

    printMessage(msg, "client", "write");

    printf("<client> Sending to fifo: %s\n", fifoPath);

    int fd = get_fifo(fifoPath, O_WRONLY);
    write_fifo(fd, msg);

    /* Waiting for ack from ackManager */

    ClientMessage cm;
    size_t mSize = sizeof(ClientMessage) - sizeof(long);

    int msqid = getMsgQueue(msgQueueKey, IPC_EXCL | S_IRUSR | S_IWUSR);
    
    printf("<client %d> Waiting for akc of type %d\n", getpid(), msg.message_id & 0xfffffffe);
    
    readMsgQueue(msqid, &cm, mSize, msg.message_id & 0xfffffffe, 0);
    // quarto parametro message_id --> filtro gli ack per id
        
    printf("\n<client %d> ClientMessage received\n", getpid());

    writeClientMessage(cm, msg.message_id >> 1, msg.message);
    
    exit(0);
}

void writeClientMessage(ClientMessage cm, int message_id, char *message){
    char filename[50];
    sprintf(filename, "out_%d.txt", message_id);

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if(fd == -1)
        printf("open failed: %s\n", filename);

    char header[150];
    sprintf(header, "Messaggio %d: %s\nLista acks:\n", message_id, message);

    if(write(fd, header, strlen(header)) == -1)
        ErrExit("failed write 1");

    char row[100];

    Acknowledgment a;
    int counter; 
    for(counter = NDEVICES; counter > 0; counter--){
        a = cm.acks[counter - 1];
        a.message_id >>= 1; // ripristino valore originale
        
        struct tm  ts;
        ts = *localtime(&a.timestamp);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts); // maggiori info su https://www.epochconverter.com/programming/c

        memset(row, 0, sizeof(row));
        sprintf(row, "%d, %d, %s\n", a.pid_sender, a.pid_receiver, buf);

        int bW = write(fd, row, strlen(row));
        if(bW == -1)
            ErrExit("failed write\n");
    }

    close(fd);
    printf("<client %d> Finished writing on file\n", getpid());
}