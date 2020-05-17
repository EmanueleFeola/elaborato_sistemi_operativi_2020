/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <sys/types.h> 
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
// #include "fifo.h"

#define NDEVICES 5
#define ROWS 5
#define COLS 5
#define MAX_NMESSAGES 3

extern char fifoBasePath[20];

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

void fillNextLine(int fd, char nextLine[]);
void fillNextPos(char *nextLine, int nchild, Position *nextPos);

void checkMessages(int fd, Message messages[], int *nMessages);
void sendMessages(int *board_ptr, Acknowledgment *acklist_ptr, Position pos, Message messages[], int *nMessages);
int scanBoard(int *shm_ptr, Position pos, int max_distance, int *scanPid);

void printMatrix(int *shm_ptr, int iteration);
void printMessage(Message msg, char *who, char *mode);