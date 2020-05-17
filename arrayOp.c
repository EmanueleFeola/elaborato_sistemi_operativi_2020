#include "arrayOp.h"

/*
 * @descrizione: shifta a destra di una posizione tutti i messaggi, 
 * se l array è pieno, l ultimo messaggio viene perso
 * in tal caso aggiorna il parametro e lo decrementa di 1
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell array
 */
void shiftRight(Message messages[], int *nMessages){
    int msgIndex;

    for(msgIndex = *nMessages; msgIndex > 0; msgIndex--)
        messages[msgIndex] = messages[msgIndex - 1];

    if(*nMessages == MAX_NMESSAGES){
        printf("Oldest message eliminated\n"); 
        (*nMessages)--;
    }
}

/*
 * @descrizione: shifta a sinistra di una posizione tutti i messaggi a destra di un pivot.
 * Decrementa il parametro passato che indica il numero di elementi contenuti
 * @esempio: dato un array 1 2 3 4 5, con pivot 2, il risultato è: 1 2 4 5 5 
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell array
 */
void shiftLeftPivot(Message messages[], int *nMessages, int pivot){
    int index;

    for(index = pivot; index < *nMessages - (pivot + 1); index++)
        messages[index] = messages[index + 1];
    
    // se il pivot è l ultimo elemento, vuol dire che non devo shiftare niente,
    // però devo decrementare il numero di elementi dell array, in modo che 
    // l ultima posizione risulti libera
    (*nMessages)--;
}

/*
 * @descrizione: shifta a destra e aggiunge in testa un nuovo msg. Incrementa il parametro nMessages
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell array
 * @param msg: il nuovo messaggio
 */
void addHead(Message messages[], int *nMessages, Message msg){
    shiftRight(messages, nMessages);

    messages[0] = msg;
    (*nMessages)++;
}
