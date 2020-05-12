/// @file semaphore.h
/// @brief Contiene la definizioni di variabili e funzioni
///         specifiche per la gestione dei SEMAFORI.

#pragma once

#include <sys/stat.h>
#include <sys/sem.h>
#include "defines.h"

void semOp (int semid, unsigned short sem_num, short sem_op);
int create_sem_set(key_t semkey, int nsem, unsigned short *values);

union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

