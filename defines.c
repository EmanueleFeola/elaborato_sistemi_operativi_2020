#include "inc/defines.h"
#include "utils/array_utils.h"
#include "utils/fifo.h"
#include "utils/shared_memory.h"
#include "utils/print_utils.h"
#include "inc/ackManager.h"

char fifoBasePath[20] = "/tmp/dev_fifo.";

// mette dentro nextLine[] la riga successiva
void fillNextLine(int fd, char nextLine[]){//char input[] --> gli passo un puntatore alla zona di memoria del buffer e lui andra' a scrivere da quel puntatore in poi per contenere la linea successiva.
    char row[50] = {0};     // contiene la riga successiva
    char buffer[2] = {0};// contiene byte successivo,si poteva mettere anche [1] ma meglio 2 per essere sicuri

    while(read(fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)// leggo fino al \n (all'inizio leggo la prima riga),finche' non hai finito di leggere i bytes (!=0) && fino a che non incontro \n, 1 e' il numero di bytes che leggo alla volta e viene salvato nel buffer
        strcat(row, buffer);//ogni volta che leggo un carattere lo vado a salvare nella stringa row (row e' tutta la riga "0,0|2,1|2,0|0,1|1,2") che andra' a definirmi la mia effettiva riga. TEORIA!!! -->  "char *strcat(char *dest, const char *src);" --> La funzione strcat() concatena la stringa src aggiungendola al termine della stringa dest. La concatenazione avviene copiando la stringa src a partire dal terminatore della stringa desc. La copia di src e' comprensiva del suo terminatore. L'area di memoria puntata da dest deve essere sufficientemente ampia da accogliere entrambe le stringhe ed il terminatore '\0'

    // (debug purposes only) se ho finito le righe ricomincia da capo 
    if(strlen(row) == 0){
        lseek(fd, 0, SEEK_SET); 
        fillNextLine(fd, nextLine);
    } else{
        memcpy(nextLine, row, strlen(row)+1);
    } 
}

// mette dentro pos.row la nuova row e in pos.col la nuova col
void fillNextPos(char *nextLine, int nchild, Position *pos){
    int pipeCounter;// serve per contare quante barre ci sono (mi conta quante | ho gia' visto nella mia stringa, ogni volta per scorrere la stringa pero' devo aumentare il contatore che punta alla stringa ovvero nextLine perche' se non incremento rimango sempre sullo stesso carattere), se sono il child 0 non devo aspettarmi nessuna barra (RICORDA CHE LEGGI COSI I DATI --> figlio 0 --> 0,0 --> figlio 1 --> |2,1 (il counter sara' a 1 e quindi a 1 barra, cosi capisco che con 1 barra sono al figlio 2) --> figlio 2 --> |2,0 (il counter sara' a 2 e quindi a 2 barre, cosi capisco che con 2 barre sono al figlio 3)e cosi via, insomma la barra la usi come INIZIO per capire il figlio successivo non come FINE per capire il figlio precedente perche' se usassi quest'ultima tecnica per l'ultimo figlio saresti fregato dato che non avresti barre finali)

    for(pipeCounter = 0; *nextLine != '\0' && pipeCounter < nchild; nextLine++)//'\0' nel nostro caso e' fine riga perche' nel buffer salviamo solo una riga non tutte quante. ogni volta che scorro la stringa nel for e non ho incontrato abbastanza | allora vado avanti con il mio puntatore della stringa. All'inizio il mio *nextLine punta al primo carattere della stringa quindi a 0 --> *nextLine sotto e' == '|' ?? no perche' e' uguale a 0 come abbiamo appena detto --> con nextLine++ passo quindi alla virgola --> *nextLine e' == '|' ?? no e' uguale a virgola. Quando arrivo a '|' vuol dire che sono arrivato nel mio blocco, devo fermarmi e analizzare quello che viene dopo.
        if(*nextLine == '|')
            pipeCounter++;

    //printf("%s\n", nextLine);//qui ti stampa:   1,2
                             //                   0,1|1,2
                             //                   2,0|0,1|1,2
                             //                   2,1|2,0|0,1|1,2
                             //                   0,0|2,1|2,0|0,1|1,2
                             //0,0|2,1|2,0|0,1|1,2 --> si riferisce al processo con nchild = 0 xk quando faccio la printf mi stampa appunto l'intera stringa e vuol dire che lui non ha mosso il puntatore e che l'ha scritta tutta mentre la prima riga 1,2 e' l'ultimo processo con nchild maggiore perche' lui ha mosso il puntatore nextLine su tutta la stringa fino ad incontrare l'ultima pipe e quando facciamo la "printf("%s\n", nextLine);", dato che il puntatore si e' mangiato tutta la stringa, ci rimangono solo quei 3 caratteri da stampare. MI STAMPA AL CONTRARIO PERCHE' I SEMAFORI PARTONO DALL'ULTIMO FIGLIO. L'ALGORITMO SAREBBE ANCHE CORRETTO E MI ASPETTEREI INVECE QUESTO OUTPUT: 
                             //                 0,0|2,1|2,0|0,1|1,2
                             //                 2,1|2,0|0,1|1,2
                             //                 2,0|0,1|1,2
                             //                 0,1|1,2
                             //                 1,2

    // fino alla virgola --> row
    char buffer[strlen(nextLine)];// al massimo la riga è lunga strlen(nextLine) ossia massimo 19 caratteri. E' un buffer che prima mi conterra' il valore della row e poi il valore della col, riutilizzo sempre lo stesso buffer
    int index;
    
    for(index = 0; *nextLine != ','; nextLine++, index++)//per ogni carattere che c'e' prima della virgola metti il mio carattere letto dentro str. NOI STIAMO LAVORANDO A BLOCCHI NEL SENSO CHE QUESTO E' IL PRIMO nchild:
    //0,0|
    //0,0|
    //0,0|
    //QUI QUINDI NEL str[index] SALVEREMO 000 (QUELLI PRIMA DELLA VIRGOLA) RISPETTIVAMENTE IN str[0], str[1] E str[2]. PRENDENDO IL FIGLIO 1 AVREMO IL BLOCCO:
    //|2,1|
    //|1,0|
    //|2,0|
    //QUI QUINDI NEL str[index] SALVEREMO 212 (QUELLI PRIMA DELLA VIRGOLA)

    /*-----------------------------------------------
              MOLTO IMPORTANTE!!!!!!!!!!!
      -----------------------------------------------*/
        buffer[index] = *(nextLine);
    
    pos->row = atoi(buffer);

    // skippa la virgola
    nextLine++; // skippa la virgola in modo tale da andare, negli esempi sopra rispettivamente, in 0 0 0 e 1 0 0

    memset(buffer, 0, sizeof(buffer));// reset stringa (altrimenti ottengo valore sporco in col) dato che dentro str ho ancora la riga per adesso     
    
    // dalla virgola alla | --> col
    for(index = 0; *nextLine != '\0' && *nextLine != '|'; nextLine++, index++)
        buffer[index] = *(nextLine);
    
    pos->col = atoi(buffer);
}

// mette dentro scanPid[] i pid dei device che distano al massimo max_distance dalla posizione pos (ovvero la posizione del device)
int scanBoard(int *board_ptr, Position pos, int max_distance, int *scanPid){
    int row = pos.row;
    int col = pos.col;
    
    max_distance++;

    //QUI MI DEFINISCO I LATI DEL QUADRAO BASANDOMI SUI SUOI ANGOLI
    int startRow = (row - max_distance) > 0 ? (row - max_distance) : 0;
    int startCol = (col - max_distance) > 0 ? (col - max_distance) : 0;

    int endRow = (row + max_distance) < ROWS ? (row + max_distance - 1) : ROWS-1;
    int endCol = (col + max_distance) < COLS ? (col + max_distance - 1) : COLS-1;

    int counter = 0;
    int pid;
    double distance;

    int test = startCol;

    for(; startRow <= endRow; startRow++)//QUI VIENE FATTO LO SCORRIMENTO QUADRATO/INTORNO, (RICORDA: SI SCORRONO IN VERTICALE LE RIGHE)
        for(startCol = test; startCol <= endCol; startCol++){
            distance = sqrt(pow(row - startRow, 2) + pow(col - startCol, 2)); 
            pid = board_ptr[startRow * COLS + startCol]; 
            // printf("%d %d --> %d\n", startRow, startCol, pid);
            if(pid != 0 && pid != getpid() && distance < max_distance){// pid != 0 --> SE LA CELLA CHE STIAMO ANALIZZANDO NON E' VUOTA && E SE pid != getpid() --> IL PID E' DIVERSO DAL MIO STESSO PID (PERCHE' OVVIAMENTE NON VOGLIO MANDARE IL MESSAGGIO A ME STESSO) && E SE distance < max_distance --> distance L'HO APPENA CALCOLATA CON EUCLIDEA INSERISCO IL PID INTERESSATO NEL MIO ARRAY scanPid
                scanPid[counter] = pid;
                counter++;
            }
        }

    return counter;
}

void sendMessages(int *board_ptr, Acknowledgment *acklist_ptr, Position pos, Message messages[], int *nMessages){
    int scanPid[NDEVICES - 1]; //QUI DENTRO VERRANNO INSERITI PROPRIO I PID DEI DEVICES, OVVIAMENTE POSSO AVERE MASSIMO 4 DEVICES VICINI DATO CHE IL 5 SONO IO MEDESIMO   
    int scanPidLen; //CONTIENE IL NUMERO DI PID CHE HA TROVATO LA scanBoard
    int msgIndex; // indice msg che sto inviando
    int holes[*nMessages]; // array degli indici dei messaggi che ho inviato
    int sentMessages = 0; // numero di messaggi che ho inviato

    /*
    printf("<device %d> Dimensione array prima: %d\n ", getpid(), *nMessages);
    for(msgIndex = 0; msgIndex < *nMessages; msgIndex++)
        printf("%d ", messages[msgIndex].message_id);

    printf("\n");
    */

    for(msgIndex = 0; msgIndex < *nMessages; msgIndex++){//QUESTO for SCORRE TUTTI I MESSAGGI PERCHE' IL MIO OBIETTIVO E' QUELLO DI MANDARE PIU' MESSAGGI POSSIBILI AI DEVICES CHE HO VICINO 
        scanPidLen = scanBoard(board_ptr, pos, messages[msgIndex].max_distance, scanPid);//QUINDI FACCIO UNA scanBoard RISPETTO ALLA MIA POSIZIONE pos E DANDOGLI COME MAX DISTANCE (messages[msgIndex].max_distance) LA MAX DISTANCE DEL MESSAGGIO CHE STO ANALIZZANDO IN QUESTO MOMENTO. DENTRO scanPidLen MI TROVERO' QUANTI PID QUESTA scanBoard HA TROVATO E DENTRO scanPid TROVERO' GLI EFFETTIVI PID TROVATI
        
        // --> se mi rimangono buchi nell array sono cazzi --> devo compattare l array ogni volta che elimino un elem
        Message msgToSend = messages[msgIndex];
        msgToSend.pid_sender = getpid(); //PRENDO IL MESSAGGIO CORRENTE CHE STO CERCANDO DI INVIARE  E SCORRO L'ARRAY DI scanPid PERCHE' VOGLIO INVIARE IL MIO MESSAGGIO A TUTTI QUELLI CHE HO TROVATO NEL MIO INTORNO RISPETTO ALLA MIA POSIZIONE SULLA MATRICE 

        // scanPidLen: 4 --> 4, 3, 2, 1
        for(; scanPidLen > 0; scanPidLen--){//SCORRO ALL'INDIETRO PERCHE' SENNO' AVREI DOVUTO CREARMI UN'ALTRA VARIABILE
            // if pid lo ha gia ricevuto skippa al prossimo pid
            if(acklist_contains_tupla(acklist_ptr, msgToSend.message_id, scanPid[scanPidLen - 1]) == 1)//PASSIAMO IL PUNTATORE ALLA LISTA DI ACK, L'ID DEL MESSAGGIO E PASSIAMO PURE IL PID DEL DEVICE (scanPid[scanPidLen - 1])E DOBBIAMO CONTROLLARE CHE QUELLO SPECIFICO MESSAGGIO IDENTIFICATO DA msgToSend.message_id NON SIA GIA' STATO RICEVUTO DAL DEVICE CON PID CHE STIAMO ANALIZZANDO IN QUESTO MOMENTO E COME FACCIAMO A SAPERE SE NON LO HA GIA' RICEVUTO ? PERCHE' SE LO HA GIA' RICEVUTO SICURAMENTE LO HA GIA' SCRITTO NELLA ACK LIST E QUINDI ANDIAMO A VEDERE NELL' ACK LIST SE C'E' GIA' QUESTA TUPLA (message_id E PID) CHE IDENTIFICA UNIVOCAMENTE LA RICEZIONE DI UN MESSAGGIO DA PARTE DI UN DEVICE E SE APPUNTO C'E' GIA' FACCIAMO UN continue CHE CI FA PASSARE A ALL'ITERAZIONE SUCCESSIVA DEL for SKIPPANDO TUTTE LE ISTRUZIONI SOTTO------------------------ QUI DOBBIAMO CONTROLLARE SE IL DEVICE IDENTIFICATO DA UN CERTO PID HA GIA' RICEVUTO IL MESSAGGIO PERCHE' NON VOGLIAMO INVIARGLIELO DUE VOLTE E SKIPPIAMO AL PROSSIMO MESSAGGIO (QUINDI ALLA PROSSIMA ITERAZIONE DEL for) E SE IL PROSSIMO MESSAGGIO DELL'ARRAY E' IDONEO ALL'INVIO LO INVIAMO AL DEVICE NUOVO E SE NON TROVIAMO NESSUN DEVICE A CUI MANDARE I VARI MESSAGGI NON NE INVIAMO NESSUNO
                continue; // skippo alla prossima iterazione

            msgToSend.pid_receiver = scanPid[scanPidLen - 1];//NOI STIAMO PASSANDO IL MESSAGGIO AL DEVICE SUCCESSIVO QUINDI DOBBIAMO AGGIORNARE IL pid_receiver CON IL PID DEL NUOVO DEVICE 

            //SE SIAMO ARRIVATI QUI VUOL DIRE CHE ABBIAMO TROVATO UN DEVICE CHE NON HA ANCORA RICEVUTO IL MESSAGGIO E STIAMO PROCEDENDO PER INVIARGLIELO 
            printMessage(msgToSend, "device", "write");//QUESTA FUNZIONE STAMPA IL MESSAGGIO

            char fname[50] = {0};
            sprintf(fname, "%s%d", fifoBasePath, scanPid[scanPidLen - 1]);

            int fd = get_fifo(fname, O_WRONLY);//FACCIO LA get_fifo DI QUELLO CHE MI SONO CREATO IMMEDIATAMENTE SOPRA E LA APRO IN SCRITTURA PERCHE' DEVO SCRIVERE SUL NUOVO DEVICE CHE E' PRONTO AD ACCOGLIERE IL MESSAGGIO
            write_fifo(fd, msgToSend);

            holes[sentMessages] = msgIndex;//UNA VOLTA CHE HO MANDATO MI SEGNO L'INDICE DEL MESSAGGIO CHE HO ELIMINATO DALL'ARRAY E QUINDI DOPO DOVRO' FARE UNO SHIFT A SX DI TUTTO QUELLO CHE C'E' A DX
            sentMessages++; // solo se poi glielo mando

            break; //se ho trovato un device a cui mandarlo passo al prossimo messaggio, perchè ho già inviato il messaggio corrente ad un device, DATO CHE NEL PDF DEL PROGETTO C'E' SCRITTO CHE UN DEVICE DEVE MANDARE IL MESSAGGIO SOLO AD UN ALTRO DEVICE E NON A PIU' DEVICES E CON QUESTO break NON FACCIO ALTRO CHE FERMARE QUESTO for PIU' INTERNO IN MODO TALE DA TORNARE AL for ESTERNO CHE MI FA ANDARE AL MESSAGGIO SUCCESSIVO
        }
        fflush(stdout);
    }

    // se mi rimangono buchi nell array sono cazzi --> devo compattare l array ogni volta che elimino un elem
    // shift verso sinistra, partendo dalla fine dei blocchi liberati
    // --> ricompatto l array
    for(; sentMessages > 0; sentMessages--)
        shiftLeftPivot(messages, nMessages, holes[sentMessages - 1]);

    /*
    printf("<device %d> Dimensione array dopo: %d\n ", getpid(), *nMessages);
    for(msgIndex = 0; msgIndex < *nMessages; msgIndex++)
        printf("%d ", messages[msgIndex].message_id);
    
    printf("\n");
    */
}