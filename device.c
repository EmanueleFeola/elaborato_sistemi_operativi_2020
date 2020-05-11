#include "device.h"

#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>

void setDeviceSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGTERM, deviceSigHandler);
}

void deviceSigHandler(int sig){
    // free devices resources

    // terminate
    printf("<device %d> terminating\n", getpid());

    exit(0);
}

void startDevice(){
    printf("<device %d> created and going to sleep\n", getpid());
    
    setDeviceSignalMask();
    pause();
}
