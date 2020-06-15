/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei semafori.

#include "err_exit.h"
#include "semaphore.h"

void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sop, 1) == -1)
        ErrExit("semop failed");
}

int create_sem_set(key_t semkey, int nsem, unsigned short *values) {
    int semid = semget(semkey, nsem, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        ErrExit("semget failed");

    union semun arg;
    arg.array = values;

    if (semctl(semid, 0, SETALL, arg) == -1)
        ErrExit("semctl SETALL failed");

    return semid;
}

void delete_sem_set(int semid){
    if (semctl(semid, 0 /*ignored*/, IPC_RMID, NULL) == -1)
        ErrExit("semctl IPC_RMID failed");
}

void waitP(int semid, int nchild){
    semOp(semid, (unsigned short)nchild, -1); // aspetto il mio turno
    semOp(semid, (unsigned short) NDEVICES, 0); // aspetto che board vada a 0
}

void signalV(int semid, int nchild){
    if (nchild < NDEVICES - 1)
        semOp(semid, (unsigned short)(nchild + 1), 1); // sblocco il device successivo
    else{
        // all indice NDEVICES del set di semafori c'è il sem della board
        // blocco la board (1 -> bloccato, 0 -> sbloccato), perche devo aspettare che sia il server a darmi il via di nuovo
        semOp(semid, (unsigned short) NDEVICES, 1);
        semOp(semid, (unsigned short) 0, 1); // sblocco il primo
        // l ordine è importante! prima blocco la board e poi sblocco il primo
        printf("\n");
    }
}