/// @file array_utils.c
/// @brief Contiene l'implementazione di funzioni di supporto per operare sugli array.

#include "array_utils.h"

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

void shiftLeftPivot(Message messages[], int *nMessages, int pivot){
    int index;

    for(index = pivot; index < *nMessages - (pivot + 1); index++)
        messages[index] = messages[index + 1];
    
    /*
    se il pivot è l ultimo elemento, vuol dire che non devo shiftare niente, però devo 
    decrementare il numero di elementi dell array, in modo che l'ultima posizione risulti libera
    */
    (*nMessages)--;//GIUSTAMENTE DEVO SHIFTARE A SX DI sentMessages (QUI SI CHIAMA nMessages) SENNO' AVREI UN VALORE SBAGLIATO DI MESSAGGI TOTALI CONTENUTI NELL'ARRAY
}

/*
 * @descrizione: shifta a destra e aggiunge in testa un nuovo msg. Incrementa il parametro nMessages
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell array
 * @param msg: il nuovo messaggio
 */
void addAsHead(Message messages[], int *nMessages, Message msg){
    shiftRight(messages, nMessages);

    messages[0] = msg;
    (*nMessages)++;//AVENDO AGGIUNTO UN ELEMENTO IN TESTA OVVIAMENTE IL NUMERO DI MESSAGGI NELL'ARRAY AUMENTA DI UNO 
}

// ritorna l indice in cui si trova l intero n all interno di arr
// ritorna -1 se n non si trova in arr
int contains(int *arr, int n){//DEVO CONTROLLARE SE DENTRO L'ARRAY DI MESSAGGI C'E' GIA' UN DETERMINATO MESSAGGIO E SE C'E' GIA' VOGLIO SAPERE IN CHE INDICE DELL'ARRAY SI TROVA
    int index = -1;

    int counter;
    for(counter = 0; counter < sizeof(arr) / sizeof(int); counter++)
        if(arr[counter] == n) // se ho trovato il numero
            index = counter;

    return index;
}