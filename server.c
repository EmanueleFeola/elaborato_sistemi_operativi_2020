/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include <sys/types.h>
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

#include <sys/sem.h>
#include <sys/shm.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int semid;
int shmid;
int *shm_ptr;

void freeResources(){
    if (semctl(semid, 0 /*ignored*/, IPC_RMID, NULL) == -1)
        ErrExit("<server> semctl IPC_RMID failed");

    if(shmdt(shm_ptr) == -1)
        ErrExit("<server> shmdt failed\n");

    if(shmctl(shmid, IPC_RMID, NULL) == -1)
        ErrExit("<server> shmctl failed\n");
}

void setServerSignalMask(){
    // added SIGINT handler for debug purposes
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGINT, serverSigHandler); //questi li ho creati perche' mi serviranno in futuro
    signal(SIGTERM, serverSigHandler);//questi li ho creati perche' mi serviranno in futuro
}

/*
 * Free resources and children on SIGTERM / SIGINT (debug purposes only)
 */
void serverSigHandler(int sig){
    kill(-getpid(), SIGTERM);//manda un kill a tutti i figli (-getpid vuol dire di mandare il kill a tutti i processi dello stesso gruppo)

    freeResources();

    // terminate
    printf("<server %d> server exits\n", getpid());

    exit(0);
}

void initDevices(){
    int nchild = 0;
    for(; nchild < NDEVICES; nchild++){//creo i figli dallo 0 al 4
        pid_t pid = fork();

        if (pid == -1){
            printf("<server> child %d not created\n", nchild);
        }

        else if(pid == 0){
            startDevice(semid, nchild, shmid);// alla "startDevice" passo il set di semafori che e' uguale per tutti i device, nchild che all'inizio e' 0 alla prima esecuzione del for fino ad arrivare a 4 ossia il 5 figlio/device (sto dicendo al device quale figlio e' in modo tale da avere una "coscienza propria")e la shared memory.
        }
    }
}

void printMatrix(int iteration){
    int row, col;

    printf("Iteration: %d\n", iteration);

    char divider[8 * COLS]; // 8 è il padding tra celle
    memset(divider, '-', sizeof(divider));

    printf("%s\n", divider);

    for(row = 0; row < ROWS; row++){
        for(col = 0; col < COLS; col++){
            int offset = row*COLS+col;
            printf("%8d", shm_ptr[offset]);
        }
        printf("\n");
    }
    
    printf("%s\n\n", divider);
}

int main(int argc, char * argv[]) {
    printf("<server %d> created server\n", getpid());

    setServerSignalMask();

    // create semaphores set
    semid = semget(IPC_PRIVATE, NDEVICES + 1, S_IRUSR | S_IWUSR);//5 figli + board per ora, poi manca ack quindi andremo a 7
    if (semid == -1)
        ErrExit("semget failed");

    // ultimo semaforo è quello di accesso alla board (1 -> bloccato, 0 -> sbloccato)
    unsigned short semInitVal[] = {0, 0, 0, 0, 1, 1}; 
    union semun arg;
    arg.array = semInitVal;//inizializzo l'array della struttura semun con i valori dei semafori

    if (semctl(semid, 0, SETALL, arg) == -1)//SETALL --> inizializzo tutti i semafori di un set identificato da semid utilizzando i valori forniti dall'array pointer di arg.array (array --> tutti i semafori). Se invece che 0 mettevo 1,2,3,4 non cambiava nulla xk veniva ignorato
        ErrExit("semctl SETALL failed");

    // create board shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(int) * ROWS * COLS, IPC_CREAT | S_IRUSR | S_IWUSR);//devo moltiplicare anche per la dimensione in bytes di un intero che e' 4 oltre che 5 * 5 xk senno' mi creerebbe una shared memory di 25 bytes che non e' sufficiente. IPC_PRIVATE --> l'ipc object non e' privato in questo singolo processo ma puo' essere acceduto da altri processi che hanno ereditato (figli --> devices) quell'ipc object dal parent (server). L'IPC_PRIVATE ci garantisce che nessun altro processo non relazionato abbia la stessa chiave. un ipc object creato con IPC_PRIVATE non lo posso condividere con altri processi eseguiti a partire da altri programmi ossia processi che non hanno nessuna relazione tra di loro.
    if(shmid == -1){
        ErrExit("shared memory allocation failed\n");
    }

    shm_ptr = (int *)shmat(shmid, NULL, 0);//nella riga 128 quando noi facciamo "shmget" istanziamo questa shared memory che e' da qualche parte in memoria pero' dobbiamo dirgli di attaccare questo spazio di indirizzamento della shared memory al nostro processo tramite "shmat", quindi noi estendiamo lo spazio di indirizzamento virtuale del nostro processo attaccandogli lo spazio della shared memory xk senno' non possiamo accederci non sapendo dove e'. LA shmat CI RITORNA UN PUNTATORE (int *) IN MODO DA POTER ACCEDERE ALLA ZONA DI MEMORIA CREATA CON shmget

    initDevices();
    
    int iteration = 0;

    while(iteration != 9){
        // ogni 2 secondi sblocco la board
        semOp(semid, NDEVICES, -1); 
        sleep(1);
        printMatrix(iteration);
        iteration++;
    }

    //while (wait(NULL) != -1);

    freeResources();
}
