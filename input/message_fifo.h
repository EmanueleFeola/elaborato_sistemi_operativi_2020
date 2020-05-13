#ifndef _MESSAGE_FIFO_HH
#define _MESSAGE_FIFO_HH

#include <sys/types.h>

struct message_fifo {     // message_fifo (client --> server) 
    pid_t cPid;           // PID of client               
    pid_t Device1;        // PID Device1
    pid_t Device2;        // PID Device2
    pid_t Device3;        // PID Device3
    pid_t Device4;        // PID Device4
    pid_t Device5;        // PID Device5
    char message[256];    // messaggio scritto dall'utente
    int message_id;       // ID univoco del messaggio (scritto dall'utente)
    int max_dist;         // max_dist entro la quale si puo' passare il message 
                         
};



#endif
