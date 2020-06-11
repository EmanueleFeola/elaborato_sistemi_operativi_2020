/// @file print_utils.c
/// @brief Contiene l'implementazione delle funzioni di supporto per stampare
///        la matrice e le strutture dati definite nel progetto

#include "print_utils.h"

/*
 * @descrizione: printa la board su cui si muovono i device per avere un riscontro grafico
 * @param board_ptr: puntatore al segmento di memoria condivisa un cui è ospitata la board
 * @param iteration: il numero della iterazione a cui si è arrivati 
 */
void printMatrix(int *board_ptr, int iteration){
    int row, col;

    printf("Iteration: %d\n", iteration);

    char divider[8 * COLS]; // 8 è il padding tra le celle
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

/*
 * @descrizione: printa una struttura Message specificandone il chiamante e la modalità (di solito read o write)
 * @param Message: il messaggio
 * @param who: specifica chi sta printando il messaggio (un device, un client, l'ack manager?) 
 * @param mode: specifica la modalità con cui il chiamante sta interagendo con il messaggio (lettura oppure scrittura)  
 * @notes: utilizzato per debug
 */
void printMessage(Message msg, char *who, char *mode){
    printf("<%s %d> %s:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            message: |%s|\n\
            max_distance: %d\n",
            who, getpid(), mode, msg.pid_sender, msg.pid_receiver, msg.message_id, msg.message, msg.max_distance);
}

/*
 * @descrizione: printa una struttura Acknowledgment specificandone il chiamante e la modalità (di solito read o write)
 * @param Acknowledgment: l'ack
 * @param who: specifica chi sta printando l'ack (un device, un client, l'ack manager?) 
 * @param mode: specifica la modalità con cui il chiamante sta interagendo con l'ack (lettura oppure scrittura)  
 * @notes: utilizzato per debug
 */
void printAck(Acknowledgment ack, char *who, char *mode){
    printf("<%s %d> %s:\n\
            pid_sender: %d\n\
            pid_receiver: %d\n\
            message_id: %d\n\
            timestamp: %ld\n",
            who, getpid(), mode, ack.pid_sender, ack.pid_receiver, ack.message_id, ack.timestamp);
}

/*
 * @descrizione: printa i message_id di tutti i messaggi contenuti nell array passato come parametro
 * @param Message: array di messaggi
 * @param nMessages: quanti messaggi ci sono dentro l array
 * @notes: utilizzato per debug
 */
void printAllMessageId(Message *messages, int nMessages){
    int counter;

    for(counter = 0; counter < nMessages; counter++)
        printf("%d ", messages[counter].message_id);

    printf("\n");
    fflush(stdout);
}