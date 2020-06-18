/// @file print_utils.h
/// @brief Contiene la definizione delle funzioni di supporto per stampare
///        la matrice e le strutture dati definite nel progetto

#pragma once

#include <stdio.h>
#include "../inc/defines.h"
#include "string.h"
#include "stdio.h"
#include <stdarg.h>

/*
 * @descrizione: printa la board su cui si muovono i device per avere un riscontro grafico
 * @param board_ptr: puntatore al segmento di memoria condivisa un cui è ospitata la board
 * @param iteration: il numero della iterazione a cui si è arrivati 
 */
void printMatrix(int *board_ptr, int iteration);

/*
 * @descrizione: printa una struttura Message specificandone il chiamante e la modalità (di solito read o write)
 * @param Message: il messaggio
 * @param who: specifica chi sta printando il messaggio (un device, un client, l'ack manager?) 
 * @param mode: specifica la modalità con cui il chiamante sta interagendo con il messaggio (lettura oppure scrittura)  
 * @notes: utilizzato per debug
 */
void printMessage(Message msg, char *who, char *mode);

/*
 * @descrizione: printa una struttura Acknowledgment specificandone il chiamante e la modalità (di solito read o write)
 * @param Acknowledgment: l'ack
 * @param who: specifica chi sta printando l'ack (un device, un client, l'ack manager?) 
 * @param mode: specifica la modalità con cui il chiamante sta interagendo con l'ack (lettura oppure scrittura)  
 * @notes: utilizzato per debug
 */
void printAck(Acknowledgment ack, char *who, char *mode);

/*
 * @descrizione: printa i message_id di tutti i messaggi contenuti nell array passato come parametro
 * @param Message: array di messaggi
 * @param nMessages: quanti messaggi ci sono dentro l array
 * @notes: utilizzato per debug
 */
void printAllMessageId(Message *messages, int nMessages);

/*
 * @descrizione: come una printf, però colorata
 * @param color: il colore da settare
 * @param bold: da settare a 1 se la scritta deve essere in grassetto, 0 altrimenti
 * @param format: format della printf
 */
void coloredPrintf(char *color, int bold, const char * format, ... );

/*
 * @descrizione: setta il colore della printf
 * @param color: il colore da settare
 * @param bold: da settare a 1 se la scritta deve essere in grassetto, 0 altrimenti
 * @notes: i colori supportati sono red, green, yellow, blue, magenta, cyan
 */
void setPrintColor(char *color, int bold);