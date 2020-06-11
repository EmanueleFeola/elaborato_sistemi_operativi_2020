/// @file array_utils.c
/// @brief Contiene l'implementazione di funzioni di supporto per operare sugli array.

#include "array_utils.h"

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
    (*nMessages)--;
}

void addAsHead(Message messages[], int *nMessages, Message msg){
    shiftRight(messages, nMessages);

    messages[0] = msg;
    (*nMessages)++;
}

int contains(int *arr, int n){
    int index = -1;

    int counter;
    for(counter = 0; counter < sizeof(arr) / sizeof(int); counter++)
        if(arr[counter] == n) // se ho trovato il numero
            index = counter;

    return index;
}