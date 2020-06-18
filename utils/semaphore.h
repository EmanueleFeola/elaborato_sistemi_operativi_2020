/// @file semaphore.h
/// @brief Contiene la definizione delle funzioni
///         specifiche per la gestione dei semafori.
#pragma once

#include <sys/stat.h>
#include <sys/sem.h>
#include "../inc/defines.h"

union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void semOp (int semid, unsigned short sem_num, short sem_op);
int create_sem_set(key_t semkey, int nsem, unsigned short *values);
void delete_sem_set(int semid);

/*
 * @description: aspetta finchè non è il turno del device nchild-esimo e finchè la board è occupata
 * @param semid: identificatore del set di semafori su cui operare
 * @param nchild: l'indice del device
 */
void waitP(int semid, int nchild);

/*
 * @description: libera le risorse che il device nchild-esimo aveva bloccato prima di entrare nelle sezione critica di accesso a risorse condivise
 * @param semid: identificatore del set di semafori su cui operare
 * @param nchild: l'indice del device
 */
void signalV(int semid, int nchild);
