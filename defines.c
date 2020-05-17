/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"
#include "arrayOp.h"
#include "fifo.h"
#include "shared_memory.h"
#include <time.h>

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

// mette dentro scanPid[] i pid dei device che distano al massimmo max_distance
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

        for(; scanPidLen > 0; scanPidLen--){
            // if pid lo ha gia ricevuto skippa al prossimo pid
            Acknowledgment ack;
            ack.pid_sender = 0;
            ack.pid_receiver = scanPid[scanPidLen - 1];
            ack.message_id = messages[msgIndex].message_id;
            ack.timestamp = 0;

            if(acklist_contains(acklist_ptr, ack) == 1)
                continue;

            // quando trovi un device che non lha ricevuto e glielo hai mandato, skippa next message --> break
            char fname[50] = {0};
            sprintf(fname, "%s%d", fifoBasePath, scanPid[scanPidLen - 1]);

            printMessage(messages[msgIndex], "device", "write");

            int fd = get_fifo(fname, O_WRONLY);
            write_fifo(fd, messages[msgIndex]);

            holes[sentMessages] = msgIndex;
            sentMessages++; // solo se poi glielo mando

            break; // se ho trovato un device a cui mandarlo passo al prossimo messaggio
        }

        fflush(stdout);
    }

    // shift verso sinistra, partendo dalla fine dei blocchi liberati
    for(; sentMessages > 0; sentMessages--)
        shiftLeftPivot(messages, nMessages, holes[sentMessages - 1]);

    /*
    printf("<device %d> Dimensione array dopo: %d\n ", getpid(), *nMessages);
    for(msgIndex = 0; msgIndex < *nMessages; msgIndex++)
        printf("%d ", messages[msgIndex].message_id);
    
    printf("\n");
    */
}

void printMatrix(int *board_ptr, int iteration){
    int row, col;

    printf("Iteration: %d\n", iteration);

    char divider[8 * COLS]; // 8 è il padding tra celle
    memset(divider, '-', sizeof(divider));

    printf("%s\n", divider);

    for(row = 0; row < ROWS; row++){
        for(col = 0; col < COLS; col++){
            int offset = row*COLS+col;
            printf("%8d", board_ptr[offset]);
        }
        printf("\n");
    }
    
    printf("%s\n\n", divider);
}

void printMessage(Message msg, char *who, char *mode){
    printf("<%s %d> %s:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            message: |%s|\n\
            max_distance: %d\n",
            who, getpid(), mode, msg.pid_sender, msg.pid_receiver, msg.message_id, msg.message, msg.max_distance);
}