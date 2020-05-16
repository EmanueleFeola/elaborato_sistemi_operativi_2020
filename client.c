/// @file client.c
/// @brief Contiene l'implementazione del client.

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

int main(int argc, char * argv[]) {
    // ./client pid_receiver message_id message max_distance

    Message msg;
    msg.pid_sender = getpid();

    // fifo path
    char *fifoBasePath = "/tmp/dev_fifo.";
    char fname[50] = {0};
    strcat(fname, fifoBasePath);

    if(argc > 1){
        if(argc != 5){
            // perchè non va la errExit dc
            printf("<client> ./client takes 4 parameters: ./client pid_receiver message_id message max_distance\n");
            exit(1);
        }

        strcat(fname, argv[1]);
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
            printf("<client %d> Insert destination PID:\n", getpid());
            scanf("%s", pidString);
            strcat(fname, pidString);
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

    printf("<client> Created message:\n\
    pid_sender: %d\n\
    pid_receiver: %d\n\
    message_id: %d\n\
    message: |%s|\n\
    max_distance: %d\n",
    msg.pid_sender, msg.pid_receiver, msg.message_id, msg.message, msg.max_distance);
    
    printf("<client> Sending to fifo: %s\n", fname);

    int fd = open(fname, O_WRONLY);
    if(fd == -1)
        printf("<client %d> failed to open fifo\n", getpid());

    write(fd, &msg, sizeof(msg));
    close(fd);

    return 0;
}