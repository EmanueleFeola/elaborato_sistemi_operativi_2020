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

void serverSigHandler(int sig){
    // free resources

    kill(-getpid(), SIGUSR1);

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
            startDevice();
        }
    }
}

int main(int argc, char * argv[]) {
    printf("<server %d> created server\n", getpid());

    initDevices();

    setServerSignalMask();

    sleep(1);

    while(1){
        printf("sending signal\n");
        kill(-getpid(), SIGUSR2);
        sleep(2);
    }

    /*
    // spiegazione signal queuing 
    https://stackoverflow.com/questions/5285414/signal-queuing-in-c
    https://www.quora.com/Why-cant-signal-handlers-block-i-e-go-to-sleep-in-Linux-Isnt-invoking-a-signal-handler-similar-to-calling-a-function-in-which-sleep-is-allowed
    */

    // Free resources if child terminates
    int status;
    pid_t endedPid;
    while((endedPid = wait(&status)) != -1){
        printf("<server> Ended child with PID %d with status: %d\n", endedPid, WEXITSTATUS(status));
    }

    return 0;
}
