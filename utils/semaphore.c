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

    // Initialize the semaphore set
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

/*
 * @descrizione: aspetta finchè non è il turno del device nchild-esimo e finchè la board è occupata
 * @param semid: identificatore del set di semafori su cui operare
 * @param nchild: l'indice del device
 */
void waitP(int semid, int nchild){//serve per sincronizzare il processo rispetto agli altri devices e rispetto alla board. ALL'INIZIO LA board SARA' A 1 QUINDI BLOCCATA, ASPETTIAMO CHE IL FIGLIO 0 ESEGUA PERO' LA board E' BLOCCATA E CON LA SECONDA semOp LA SBLOCCO COSI IL FIGLIO POTRA' ANDARE AD AGIRE, NEL SECONDO for AVREMO IL FIGLIO 2 CON LA board SEMPRE SBLOCCATA (!!!!)(IL SERVER LA SBLOCCA, PARTONO TUTTI I PROCESSI CHE SI SBLOCCANO A VICENDA PERO' SE QUALCUNO BLOCCA LA board DOPO DEVE ASPETTARE UN TOT DI TEMPO PRIMA CHE IL SERVER LA SBLOCCHI DI NUOVO E QUESTO E' UN ERRORE !!! IO DEVO BLOCCARLA SOLO ALLA FINE QUANDO TUTTI HANNO FATTO LE LORO OPERAZIONI E DEVO ASPETTARE CHE IL SERVER ME LA SBLOCCHI DOPO E QUESTO E' CORRETTO XK VUOL DIRE CHE TUTTI I PROCESSI FIGLIO HANNO ESEGUITO I LORO COMPITI E QUINDI MI METTO IN ATTESA CHE IL SERVER MI DICA "va bene la board e' di nuovo accessibile, muovetevi, inviatevi i messaggi etc"), LA board LA BLOCCO IN FONDO CON IL signalV E LA BLOCCO SOLO SE SONO L'ULTIMO FIGLIO
    semOp(semid, (unsigned short)nchild, -1); // aspetto il mio stesso turno, qui nchild passato dal server e' fondamentale perche' il figlio deve appunto aspettare il suo stesso turno
    semOp(semid, (unsigned short) NDEVICES, 0); // aspetto che board vada a 0 --> sblocco
}

/*
 * @descrizione: libera le risorse che il device nchild-esimo aveva bloccato prima di entrare nelle sezione critica di accesso a risorse condivise
 * @param semid: identificatore del set di semafori su cui operare
 * @param nchild: l'indice del device
 */
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