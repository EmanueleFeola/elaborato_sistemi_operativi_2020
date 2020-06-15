/// @file print_utils.c
/// @brief Contiene l'implementazione delle funzioni di supporto per stampare
///        la matrice e le strutture dati definite nel progetto

#include "print_utils.h"

void printMatrix(int *board_ptr, int iteration){
    int row, col;

    coloredPrintf("cyan", 0, "Iteration: %d\n", iteration);

    char divider[8 * COLS]; // 8 è il padding tra le celle
    memset(divider, '-', sizeof(divider));

    printf("%s\n", divider);

    for(row = 0; row < ROWS; row++){
        for(col = 0; col < COLS; col++){
            int offset = row * COLS + col;
            coloredPrintf("default", 0, "%8d", board_ptr[offset]);
        }
        printf("\n");
    }
    
    printf("%s\n\n", divider);
}

void printMessage(Message msg, char *who, char *mode){
    coloredPrintf("yellow", 0, "<%s %d> %s:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            message: |%s|\n\
            max_distance: %d\n",
            who, getpid(), mode, msg.pid_sender, msg.pid_receiver, msg.message_id, msg.message, msg.max_distance);
}

void printAck(Acknowledgment ack, char *who, char *mode){
    coloredPrintf("yellow", 0, "<%s %d> %s:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            timestamp: %ld\n",
            who, getpid(), mode, ack.pid_sender, ack.pid_receiver, ack.message_id, ack.timestamp);
}

void printAllMessageId(Message *messages, int nMessages){
    int counter;

    for(counter = 0; counter < nMessages; counter++)
        coloredPrintf("default", 1, "%d ", messages[counter].message_id);

    printf("\n");
    fflush(stdout);
}

void coloredPrintf (char *color, int bold, const char * format, ... )
{
    setPrintColor(color, bold);

    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, format, argptr);
    va_end(argptr);

    setPrintColor("default", 0);
}

void setPrintColor(char *color, int bold){
    if(strcmp("red", color) == 0)
        printf("\033[%d;31m", bold);
    else if(strcmp("green", color) == 0)
        printf("\033[%d;32m", bold);
    else if(strcmp("yellow", color) == 0)
        printf("\033[%d;33m", bold);
    else if(strcmp("blue", color) == 0)
        printf("\033[%d;34m", bold);
    else if(strcmp("magenta", color) == 0)
        printf("\033[%d;35m", bold);
    else if(strcmp("cyan", color) == 0)
        printf("\033[%d;36m", bold);
    else if(strcmp("default", color) == 0)
        printf("\033[0;0m");
    else
        printf("\033[0;0m");
}