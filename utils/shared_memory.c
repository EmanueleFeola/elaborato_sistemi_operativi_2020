/// @file shared_memory.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione della MEMORIA CONDIVISA.

#include "../defines.h"

#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>

#include "err_exit.h"
#include "shared_memory.h"


int alloc_shared_memory(key_t shmKey, size_t size) {
    // get, or create, a shared memory segment
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        ErrExit("shmget failed");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) {
    // attach the shared memory
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *)-1)
        ErrExit("shmat failed");

    return ptr_sh;
}

void free_shared_memory(void *ptr_sh) {
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        ErrExit("shmdt failed");
}

void remove_shared_memory(int shmid) {
    // delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("shmctl failed");
}

/*
typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    time_t timestamp;
} Acknowledgment;
*/

void write_ack(Acknowledgment *ptr, Acknowledgment ack){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];
        if(a->message_id == 0){
            a->pid_sender = ack.pid_sender;
            a->pid_receiver = ack.pid_receiver;
            a->message_id = ack.message_id;
            a->timestamp = ack.timestamp;
            printf("Writing on shm at %d: %d %d %d %ld\n", counter, a->pid_sender, a->pid_receiver, a->message_id, a->timestamp);
            break;
        }
    }
}

int acklist_contains(Acknowledgment *ptr, int message_id, pid_t pid_receiver){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];
        if(a->message_id == message_id && a->pid_receiver == pid_receiver)
            return 1;
    }

    return -1;
}

// debug purposes
void read_acks(Acknowledgment *ptr){
    int counter = 0;

    Acknowledgment *a;

    for(; counter < 100; counter++){
        a = &ptr[counter];

        if(a->message_id != 0){
            printf("Found at %d: %d %d %d %ld\n", counter, a->pid_sender, a->pid_receiver, a->message_id, a->timestamp);
        }
    }
}