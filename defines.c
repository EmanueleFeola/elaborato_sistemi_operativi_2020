#include "defines.h"
#include "utils/array_utils.h"
#include "utils/fifo.h"
#include "utils/shared_memory.h"
#include "utils/print_utils.h"

char fifoBasePath[20] = "/tmp/dev_fifo.";

// mette dentro nextLine[] la riga successiva
void fillNextLine(int fd, char nextLine[]){
    char row[50] = {0};  
    char buffer[2] = {0};

    while(read(fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)
        strcat(row, buffer);

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
    int pipeCounter;

    for(pipeCounter = 0; *nextLine != '\0' && pipeCounter < nchild; nextLine++)
        if(*nextLine == '|')
            pipeCounter++;

    // fino alla virgola --> row
    char buffer[strlen(nextLine)];
    int index;
    
    for(index = 0; *nextLine != ','; nextLine++, index++)
        buffer[index] = *(nextLine);
    
    pos->row = atoi(buffer);

    // skippa la virgola
    nextLine++; 

    // reset stringa (altrimenti ottengo valore sporco in col)
    memset(buffer, 0, sizeof(buffer));     
    
    // dalla virgola alla | --> col
    for(index = 0; *nextLine != '\0' && *nextLine != '|'; nextLine++, index++)
        buffer[index] = *(nextLine);
    
    pos->col = atoi(buffer);
}

// mette dentro messages il nuovo messaggio letto
// e shifta a destra tutti gli altri
void checkMessages(int fd, Message messages[], int *nMessages){
    int bR = -1;

    Message msg;

    do{
        bR = read(fd, &msg, sizeof(Message));
        if(bR != 0){
            printMessage(msg, "device", "read");
            addHead(messages, nMessages, msg);
        }

    } while(bR > 0);
}

// mette dentro scanPid[] i pid dei device che distano al massimmo max_distance dalla posizione pos (ovvero la posizione del device)
int scanBoard(int *board_ptr, Position pos, int max_distance, int *scanPid){
    int row = pos.row;
    int col = pos.col;
    
    max_distance++;

    int startRow = (row - max_distance) > 0 ? (row - max_distance) : 0;
    int startCol = (col - max_distance) > 0 ? (col - max_distance) : 0;

    int endRow = (row + max_distance) < ROWS ? (row + max_distance - 1) : ROWS-1;
    int endCol = (col + max_distance) < COLS ? (col + max_distance - 1) : COLS-1;

    int counter = 0;
    int pid;
    double distance;

    int test = startCol;

    for(; startRow <= endRow; startRow++)
        for(startCol = test; startCol <= endCol; startCol++){
            distance = sqrt(pow(row - startRow, 2) + pow(col - startCol, 2)); 
            pid = board_ptr[startRow * COLS + startCol]; 
            // printf("%d %d --> %d\n", startRow, startCol, pid);
            if(pid != 0 && pid != getpid() && distance < max_distance){
                scanPid[counter] = pid;
                counter++;
            }
        }

    return counter;
}

void sendMessages(int *board_ptr, Acknowledgment *acklist_ptr, Position pos, Message messages[], int *nMessages){
    int scanPid[NDEVICES - 1];  
    int scanPidLen;
    int msgIndex; // indice msg che sto inviando
    int holes[*nMessages]; // array degli indici dei messaggi che ho inviato
    int sentMessages = 0; // numero di messaggi che ho inviato

    /*
    printf("<device %d> Dimensione array prima: %d\n ", getpid(), *nMessages);
    for(msgIndex = 0; msgIndex < *nMessages; msgIndex++)
        printf("%d ", messages[msgIndex].message_id);

    printf("\n");
    */

    for(msgIndex = 0; msgIndex < *nMessages; msgIndex++){
        scanPidLen = scanBoard(board_ptr, pos, messages[msgIndex].max_distance, scanPid);
        
        // --> se mi rimangono buchi nell array sono cazzi --> devo compattare l array ogni volta che elimino un elem
        Message msgToSend = messages[msgIndex];
        msgToSend.pid_sender = getpid(); 

        // scanPidLen: 4 --> 4, 3, 2, 1
        for(; scanPidLen > 0; scanPidLen--){
            // if pid lo ha gia ricevuto skippa al prossimo pid
            if(acklist_contains(acklist_ptr, msgToSend.message_id, scanPid[scanPidLen - 1]) == 1)
                continue; // skippo alla prossima iterazione

            msgToSend.pid_receiver = scanPid[scanPidLen - 1]; 

            printMessage(msgToSend, "device", "write");

            char fname[50] = {0};
            sprintf(fname, "%s%d", fifoBasePath, scanPid[scanPidLen - 1]);

            int fd = get_fifo(fname, O_WRONLY);
            write_fifo(fd, msgToSend);

            holes[sentMessages] = msgIndex;
            sentMessages++; // solo se poi glielo mando

            break; 
            // se ho trovato un device a cui mandarlo passo al prossimo messaggio, perchè ho già inviato il messaggio corrente ad un device
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