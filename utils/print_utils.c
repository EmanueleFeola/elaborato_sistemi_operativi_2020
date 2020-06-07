#include "print_utils.h"

void printMatrix(int *board_ptr, int iteration){
    int row, col;

    printf("Iteration: %d\n", iteration);

    char divider[8 * COLS]; // 8 è il padding tra celle
    memset(divider, '-', sizeof(divider));

    printf("%s\n", divider);

    for(row = 0; row < ROWS; row++){
        for(col = 0; col < COLS; col++){
            int offset = row * COLS + col;
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

void printAck(Acknowledgment ack, char *who, char *mode){
    printf("<%s %d> %s:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            timestamp: %ld\n",
            who, getpid(), mode, ack.pid_sender, ack.pid_receiver, ack.message_id, ack.timestamp);
}

// printa i message_id di tutti i messaggi contenuti nell array passato come parametro
void printAllMessageId(Message *messages, int nMessages){
    int counter;

    for(counter = 0; counter < nMessages; counter++)
        printf("%d ", messages[counter].message_id);

    printf("\n");
    fflush(stdout);
}
