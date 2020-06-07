#include "defines.h"
#include "utils/err_exit.h"
#include "utils/fifo.h"
#include "utils/msg_queue.h"
#include "utils/print_utils.h"
#include <sys/stat.h>

// ./client pid_receiver message_id message max_distance
int main(int argc, char * argv[]) {
    Message msg;
    msg.pid_sender = getpid();

    // fifo path
    char fifoBasePath[20] = "/tmp/dev_fifo.";
    char fifoPath[50] = {0};
    sprintf(fifoPath, "%s", fifoBasePath);

    if(argc > 1){
        if(argc != 5)
            ErrExit("<client> ./client takes 4 parameters: ./client pid_receiver message_id message max_distance");
            
        strcat(fifoPath, argv[1]);
        msg.pid_receiver = atoi(argv[1]);
        msg.message_id = atoi(argv[2]);

        memcpy(msg.message, argv[3], strlen(argv[3]));
        msg.message[strlen(argv[3])] = '\0';
        
        msg.max_distance = atoi(argv[4]);

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

    key_t key = 123; // TODO: get from argv
    int msqid = getMsgQueue(key, IPC_EXCL | S_IRUSR | S_IWUSR);
    
    printf("<client %d> Waiting for akc of type %d\n", getpid(), msg.message_id);
    
    readMsgQueue(msqid, cm, mSize, msg.message_id, 0);
    // quarto parametro message_id --> filtro i messaggi per id
        
    printf("<client %d> ClientMessage received:\n---start---", getpid());
    
    Acknowledgment a;
    int counter; 
    for(counter = 0; counter < NDEVICES; counter++){
        a = cm.acks[counter];
        printAck(a, "client", "read");
    }

    // write to file

    exit(0);
}
