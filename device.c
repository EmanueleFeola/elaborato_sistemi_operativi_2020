#include "device.h"

#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>

#include "semaphore.h"

void setDeviceSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGTERM, deviceSigHandler);
}

void deviceSigHandler(int sig){
    printf("<device %d> terminating\n", getpid());
    exit(0);
}

void startDevice(int semid, int nchild){
    printf("<device %d> created new device \n", getpid());
    
    setDeviceSignalMask();
    
    while(1){
        semOp(semid, (unsigned short)nchild, -1); // aspetto il mio turno
        semOp(semid, (unsigned short) 5, 0); // aspetto che board vada a 0

        printf("<device %d> è il mio turno\n", getpid());
        fflush(stdout);

        if (nchild > 0)
            semOp(semid, (unsigned short)(nchild - 1), 1);
        else{
            semOp(semid, (unsigned short) NDEVICES, 1); // blocco la board (1 -> bloccato, 0 -> sbloccato)
            semOp(semid, (unsigned short) NDEVICES - 1, 1);
            printf("\n");
        }
    }
}
