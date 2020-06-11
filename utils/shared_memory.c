/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della memoria condivisa.

#include "../inc/defines.h"

#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>

#include "err_exit.h"
#include "shared_memory.h"


// get, or create, a shared memory segment
int alloc_shared_memory(key_t shmKey, size_t size) {
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        ErrExit("shmget failed");

    return shmid;
}

// attach the shared memory
void *get_shared_memory(int shmid, int shmflg) {
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *)-1)
        ErrExit("shmat failed");

    return ptr_sh;
}

// detach the shared memory segments
void free_shared_memory(void *ptr_sh) {
    if (shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}

// delete the shared memory segment
void remove_shared_memory(int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("shmctl failed");
}