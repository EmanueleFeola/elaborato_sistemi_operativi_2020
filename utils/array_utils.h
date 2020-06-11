/// @file array_utils.h
/// @brief Contiene la definizione delle funzioni di supporto per operare sugli array.

#pragma once

#include "../inc/defines.h"

/*
 * @descrizione: shifta a destra di una posizione tutti i messaggi, 
 * se l array è pieno, l ultimo messaggio viene perso (l'array è di dimensione statica!).
 * In tal caso aggiorna il parametro nMessages e lo decrementa di 1
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell'array
 */
void shiftRight(Message messages[], int *nMessages);

/*
 * @descrizione: shifta a destra e aggiunge in testa un nuovo msg; incrementa il parametro nMessages
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell array
 * @param msg: il nuovo messaggio da aggiungere
 */
void addAsHead(Message messages[], int *nMessages, Message msg);

/*
 * @descrizione: shifta a sinistra di una posizione tutti i messaggi a destra di un pivot.
 * Decrementa il parametro passato che indica il numero di elementi contenuti
 * @esempio: dato un array 1 2 3 4 5, con pivot 2, il risultato è: 1 2 4 5 5 
 * @param messages: array di Message
 * @param nMessages: numero di messaggi contenuti nell'array
 */
void shiftLeftPivot(Message messages[], int *nMessages, int pivot);

/*
 * @param arr: l array
 * @param n: l intero di cui verificare la presenza
 * @return: indice in cui si trova la prima occorrenza di n, oppure -1 se esso non è contenuto nell'array 
 */
int contains(int *arr, int n);