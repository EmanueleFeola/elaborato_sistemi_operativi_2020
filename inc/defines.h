#pragma once

// sys imports
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/types.h> 
#include <errno.h>
#include <signal.h> 
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>

// utils
#include "../utils/semaphore.h"
#include "../utils/err_exit.h"

#define NDEVICES 5
#define ROWS 5
#define COLS 5
#define MAX_NMESSAGES 5 // numero di messaggi che ogni device può mettere nel buffer messaggi
#define MAX_ACK_LIST 100 // numero massimo di ack che la ack_list può contenere

// variabile globale che serve a più processi
extern char fifoBasePath[20]; 

// struttura usata dai device per memorizzare la loro posizione
typedef struct{
    int row;
    int col;
} Position;

typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    char message[256];
    int max_distance;
} Message;

typedef struct {
    pid_t pid_sender;
    pid_t pid_receiver;
    int message_id;
    time_t timestamp;
} Acknowledgment;

// struttura scambiata tra ackManager e client tramite la message queue
typedef struct {
    long mtype;
    Acknowledgment acks[5]; //ARRAY DI 5 Acknowledgment 
} ClientMessage;

/*
 * @description: popola nextLine[] con la riga successiva del file posizioni
 * @param fd: file descriptor del file posizioni
 * @param nextLine: buffer da popolare con la next line
 */
void fillNextLine(int fd, char nextLine[]);

/*
 * @description: parserizza il buffer nextLine e ricava la posizione del device
 * @param nchild: identificatore del device
 * @param nextPos: struttura da popolare con la posizione estratta da nextLine
 */
void fillNextPos(char *nextLine, int nchild, Position *nextPos);

/*
 * @description: popola scanPid[] con i pid dei device che distano al massimmo max_distance 
 * dalla posizione pos (ovvero dalla posizione del device)
 * @param board_ptr: puntatore alla board, ovvero al segmento di memoria condivisa
 * @param max_distance: distanza massima a cui un device può trovarsi per poter accogliere il messaggio
 * @param scanPid: array che conterrà i pid che possono ricevere il messaggio
 * @return: il numero di pid trovati
 * @notes: non scorre l'intera board, ma solo una sezione/intorno in base a max_distance
 */
int scanBoard(int *board_ptr, Position pos, int max_distance, int *scanPid);

/*
 * @description: manda i messaggi presenti nell array messages ai device vicini e ricompatta l'array messages evitando buchi
 * @param board_ptr: puntatore al segmento di memoria condivisa che rappresenta la board
 * @param acklist_ptr: puntatore al segmento di memoria condivisa che rappresenta la lista di ack
 * @param pos: posizione attuale del device
 * @param messages: array di messaggi memorizzati del device
 * @param nMessages: puntatore al numero di messaggi contenuti in messages
 */
void sendMessages(int *board_ptr, Acknowledgment *acklist_ptr, Position pos, Message messages[], int *nMessages);