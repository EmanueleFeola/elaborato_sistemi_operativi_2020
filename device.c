#include "device.h"

#include <signal.h> 
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>

#include <sys/shm.h>

#include "semaphore.h"
#include "defines.h"
#include "err_exit.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int *shm_ptr;
char fifoPath[25] = {0};
int positionFd;

void setDeviceSignalMask(){
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGTERM, deviceSigHandler);
}

void deviceSigHandler(int sig){
    printf("<device %d> terminating\n", getpid());

    if(shmdt(shm_ptr) == -1)
        ErrExit("<device> shmdt failed\n");

    if(unlink(fifoPath) == -1)
        ErrExit("<device> unlink fifo failed\n");
    
    if(close(positionFd) == -1)
        ErrExit("<device> closing file failed\n");

    exit(0);
}

/*
 * wait until it is my turn and board is available 
 */
void waitP(int semid, int nchild){//serve per sincronizzare il processo rispetto agli altri devices e rispetto alla board. ALL'INIZIO LA board SARA' A 1 QUINDI BLOCCATA, ASPETTIAMO CHE IL FIGLIO 0 ESEGUA PERO' LA board E' BLOCCATA E CON LA SECONDA semOp LA SBLOCCO COSI IL FIGLIO POTRA' ANDARE AD AGIRE, NEL SECONDO for AVREMO IL FIGLIO 2 CON LA board SEMPRE SBLOCCATA (!!!!)(IL SERVER LA SBLOCCA, PARTONO TUTTI I PROCESSI CHE SI SBLOCCANO A VICENDA PERO' SE QUALCUNO BLOCCA LA board DOPO DEVE ASPETTARE UN TOT DI TEMPO PRIMA CHE IL SERVER LA SBLOCCHI DI NUOVO E QUESTO E' UN ERRORE !!! IO DEVO BLOCCARLA SOLO ALLA FINE QUANDO TUTTI HANNO FATTO LE LORO OPERAZIONI E DEVO ASPETTARE CHE IL SERVER ME LA SBLOCCHI DOPO E QUESTO E' CORRETTO XK VUOL DIRE CHE TUTTI I PROCESSI FIGLIO HANNO ESEGUITO I LORO COMPITI E QUINDI MI METTO IN ATTESA CHE IL SERVER MI DICA "va bene la board e' di nuovo accessibile, muovetevi, inviatevi i messaggi etc"), LA board LA BLOCCO IN FONDO CON IL signalV E LA BLOCCO SOLO SE SONO L'ULTIMO FIGLIO
    semOp(semid, (unsigned short)nchild, -1); // aspetto il mio stesso turno, qui nchild passato dal server e' fondamentale perche' il figlio deve appunto aspettare il suo stesso turno
    semOp(semid, (unsigned short) 5, 0); // aspetto che board vada a 0 --> sblocco
}

/*
 * signal to other devices that I completed my tasks
 */
void signalV(int semid, int nchild){
    if (nchild > 0)
        semOp(semid, (unsigned short)(nchild - 1), 1); // sblocco il device successivo
    else{
        semOp(semid, (unsigned short) NDEVICES, 1); // blocco la board (1 -> bloccato, 0 -> sbloccato)
        semOp(semid, (unsigned short) NDEVICES - 1, 1); // sblocco il primo
        // check per errore accesso board
        printf("\n");
    }
}

void readFifo(int fd){
    int bR = -1;

    Message msg;

    do{
        bR = read(fd, &msg, sizeof(Message));
        if(bR != 0)
            printf("<device %d> Read:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            message: |%s|\n\
            max_distance: %d\n",
            getpid(), msg.pid_sender, msg.pid_receiver, msg.message_id, msg.message, msg.max_distance);

    } while(bR > 0);

}

void startDevice(int semid, int nchild, int shmid){
    printf("<device %d> created new device \n", getpid());// mi stampa tutti i device insieme invece che uno alla volta perche' nel while(1) sotto faccio una waitP

    // signal mask & signal handler
    setDeviceSignalMask();//ri-setto la maschera di segnali perche' essendo device.c un processo a parte rispetto al server devo settare la maschera. Di default un processo figlio eredita quella del padre pero' noi vogliamo che i figli si comportino in maniera differente perche' se utilizziamo la stessa maschera del padre vuol dire che quando un figlio muore lui fa comunque una kill verso tutti i suoi devices/figli che pero' non ne ha di figli e quindi non ha senso --> sono due processi diversi e i figli devono rispondere in maniera diversa in base ai segnali che ricevono

    // shm attach
    shm_ptr = (int *)shmat(shmid, NULL, 0);//stesso motivo per cui lo ho fatto nel server, nel server avevo bisogno di un modo per agganciarmi alla shared memory e devo farlo anche qui perche' senno' non posso accedere alla shared memory. Devo fare l'attach anche qui xk senno' non potrei lavorare sui devices per spostarli nella matrice (infatti se non facessi l'attach anche qui il server mi stamperebbe matrici di soli 0, perche' se i device non fanno l'attach e non scrivono la shared memory lascia tutti 0, quando fai una shmget inizializzo un segmento di memoria condivisa con tutti 0), questo shm_ptr deve essere uguale per tutti per puntare alla stessa shared memory. RIASSUMENDO IL MOTIVO PER CUI FACCIO shmat ANCHE QUI E' XK QUESTO device.c RAPPRESENTE IL DEVICE E IL DEVICE E' UN PROCESSO A PARTE, E' FIGLI DEL SERVER PERO' GLI SPAZI DI INDIRIZZAMENTO SONO DIVERSI QUINDI VA FATTO shmat PURE QUI. shm_ptr deve essere uguale per tutti per puntare alla stessa shared memory
    if(shm_ptr == (void *)-1)//(void *)-1 se ti viene ritornato un pointer uguale a -1
        ErrExit("<device> shmat failed\n");

    // create fifo
    char *basePath = "/tmp/dev_fifo.";
    strcat(fifoPath, basePath);
    
    char pidbuf[5];
    sprintf(pidbuf, "%d", getpid());
    strcat(fifoPath, pidbuf);

    printf("<device %d> Created fifo: %s\n", getpid(), fifoPath);

    int res = mkfifo(fifoPath, S_IRUSR | S_IWUSR);
    if(res == -1)
        ErrExit("<device> failed creating fifo\n");
    int fifoFd = open(fifoPath, O_RDONLY | O_NONBLOCK); // non lo abbiamo usato in classe
    if(fifoFd == -1)
        ErrExit("<device> open fifo failed\n");

    // open position file
    positionFd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);// open position file (MOLTO IMPORTANTE!!!! E' TRAMITE QUESTA open CHE MI POSIZIONO SUL PRIMO CARATTERE DEL file_posizioni)

    // board position buffers
    char nextLine[100] = {0};//buffer/stringa di 100 caratteri che rappresenta la dimensione della riga successiva (lavoriamo sulle righe singolarmente invece che su tutte insieme). e' come se dessi 100 caratteri ad ogni riga, sono 19 i caratteri, si poteva mettera anche a 19 volendo
    nextMove_t nextMove = {0, 0};// vado a inizializzare i valori row e col della struttura contenuta in device.h a 0

    while(1){
        waitP(semid, nchild);

        readFifo(fifoFd);
        
        int oldMatrixIndex = nextMove.row * COLS + nextMove.col;//all'inizio e' 0

        fillNextLine(positionFd, nextLine);//passo fd del file_posizioni.txt e la nextLine che e' un buffer di caratteri pero' quando torno dalla fillNextLine dentro il secondo argomento "nextLine" trovo la prossima riga xk dentro fillNextLine vado ad agire sul buffer che ho passato come parametro e lo popolo con la nextLine. QUINDI ALLA PRIMA ESECUZIONE DI QUESTA fillNextLine ABBIAMO IL PUNTATORE AL PRIMO CARATTERE DELLA PRIMA LINEA DERIVATO DALLA open FATTA SOPRA "int fd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);" E CON LA fillNextLine ANDRA' ALL'ULTIMO CARATTERE DELLA PRIMA LINEA.

        //MOLTO IMPORTANTE !!! IL SECONDO ARGOMENTO nextLine E' COME DIRE CHE SONO PRONTO A LEGGERE LA LINEA SUCCESSIVA, ALLA SECONDA ESECUZIONE DEL while NOI ABBIAMO FINITO DI LEGGERE CARATTERE PER CARATTERE LA PRIMA RIGA "(read(fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)". IL PROCESSO HA MEMORIA DI DOVE E' ARRIVATO A LEGGERE file AUTOMATICAMENTE (NOI NON DOBBIAMO FARE NIENTE !!!!!!!!!!) RICOMINCIA DAL PUNTO IN CUI SI ERA FERMATO PRIMA E VA AVANTI TRANQUILLAMENTE CON LA SECONDA RIGA
        fillNextMove(nextLine, nchild, &nextMove);

        // printf("<device %d> my turn --> %d, %d --> ", getpid(), nextMove.row, nextMove.col);

        //TEORIA !!! --> sarebbe una matrice bidimensionale rappresentata come un array monodimensionale perchè alla fine una matrice è un array di array, quindi in sostanza è un array
         for(; nchild < NDEVICES; nchild++){
            fillNextMove(nextLine, nchild, &nextMove);
            checkEuclideanDistance(nchild, fifoPath, nextMove);

         }

        int matrixIndex = nextMove.row * COLS + nextMove.col;//qui effettivamente per quanto riguarda il primo figlio ci troveremo in 0,0 quindi 0 * 5 + 0
        
        // mi sposto sulla cella giusta
        // se la cella prossima è occupata sto fermo (?)
        if(shm_ptr[matrixIndex] == 0){//se il valore contenuto nella cella e' 0
            shm_ptr[oldMatrixIndex] = 0;     // libero precedente
            shm_ptr[matrixIndex] = getpid(); // occupo nuova
        }
        else{
            //se e' occupato ripristino i valori precedenti perche' non mi sono mosso --> devo ripristinare nextMove con i valori precedenti
            nextMove.row = oldMatrixIndex / COLS;
            nextMove.col = oldMatrixIndex % COLS;
            //esempio: se oldMatrixIndex = 8; allora facendo 8 / COLS = 8 / 5 = 1 con resto 3 mi ricalcolo la row (8/5), col(8%5) precedente
        }

        signalV(semid, nchild);
    }
}
