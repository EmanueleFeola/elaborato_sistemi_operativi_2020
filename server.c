/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"

#include "server.h"
// signals imports
#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>

#include <stdlib.h>

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

int main(int argc, char * argv[]) {
    setServerSignalMask();

    while(1){
        printf("<Server> Running with PID %d\n", getpid());
        sleep(5);
    }    

    return 0;
}

void serverSigHandler(int sig){
    // free resources
    printf("freeing resources\n");
    // free children
    printf("freeing children\n");

    // terminate
    printf("terminating...bye bye\n");

    exit(0);
}
