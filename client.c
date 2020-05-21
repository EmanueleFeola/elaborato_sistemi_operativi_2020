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

#define SCRIPT_INPUT

int main(int argc, char * argv[]) {
    // ./client pid_receiver message_id message max_distance

    Message msg;
    msg.pid_sender = getpid();

    // fifo path
    char *fifoBasePath = "/tmp/dev_fifo.";
    char fname[50] = {0};
    strcat(fname, fifoBasePath);


    #ifdef SCRIPT_INPUT
        if(argc != 5){
            // perchè non va la errExit dc
            printf("<client> ./client takes 4 parameters: ./client pid_receiver message_id message max_distance\n");
            exit(1);
        }

        strcat(fname, argv[1]);
        msg.pid_receiver = atoi(argv[1]);
        msg.message_id = atoi(argv[2]);

        memcpy(msg.message, argv[3], strlen(argv[3]));//dato che e' una stringa devo fare una memcpy e il msg e' un puntatore ad un area di memoria, devo copiare da dove inizia msg alla sua fine e lo copio tramite memcpy
        msg.message[strlen(argv[3])] = '\0';//bisogna sempre aggiungere un carattere di fine stringa per sicurezza
        
        msg.max_distance = atoi(argv[4]);

    #else
        char pidString[7] = {0};
        printf("<client %d> Insert destination PID:\n", getpid());
        scanf("%s", pidString);
        strcat(fname, pidString);
        msg.pid_receiver = atoi(pidString);

        printf("<client %d> Insert message id: \n", getpid());
        scanf("%d", msg.message_id);

        printf("<client %d> Insert message: \n", getpid());
        scanf("%s", msg.message);

        printf("<client %d> Insert max distance: \n", getpid());
        scanf("%d", msg.max_distance);
    #endif

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
