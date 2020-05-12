#include "device.h"

#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>

void setDeviceSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGUSR2);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGUSR2, deviceSigHandler);
    signal(SIGUSR1, deviceSigHandler);
}

void deviceSigHandler(int sig){
    if(sig == SIGUSR1){
        // free devices resources

        // terminate
        printf("<device %d> terminating\n", getpid());

        exit(0);
    }
    else if(sig == SIGUSR2){
        printf("<device %d> SIGUSR2 received\n", getpid());
        pause();
    }

}

void startDevice(){
    printf("<device %d> created and going to sleep\n", getpid());
    
    setDeviceSignalMask();
    pause();
}
