#include "defines.h"
#include "utils/err_exit.h"
#include "utils/fifo.h"
#include "utils/msg_queue.h"
#include "utils/print_utils.h"
#include <sys/stat.h>

// ./client msg_queue_key pid_receiver message_id message max_distance
int main(int argc, char * argv[]) {
    // check params
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

    printMessage(msg, "client", "write");

    printf("<client> Sending to fifo: %s\n", fifoPath);

    int fd = get_fifo(fifoPath, O_WRONLY);
    write_fifo(fd, msg);

    /* Waiting for ack from ackManager */

    ClientMessage cm;
    size_t mSize = sizeof(ClientMessage) - sizeof(long);

    int msqid = getMsgQueue(msgQueueKey, IPC_EXCL | S_IRUSR | S_IWUSR);
    
    printf("<client %d> Waiting for akc of type %d\n", getpid(), msg.message_id);
    
    readMsgQueue(msqid, &cm, mSize, msg.message_id, 0);
    // quarto parametro message_id --> filtro gli ack per id
        
    printf("<client %d> ClientMessage received:\n", getpid());
    
    Acknowledgment a;
    int counter; 
    for(counter = 0; counter < NDEVICES; counter++){
        a = cm.acks[counter];
        printAck(a, "client", "read");
        // write to file
    }

    exit(0);
}
