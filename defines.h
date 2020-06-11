#pragma once

// sys
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
#include "utils/semaphore.h"
// #include "utils/msg_queue.h"
#include "utils/err_exit.h"

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

typedef struct {
    long mtype;
    Acknowledgment acks[5]; // acks[NDEVICES]
} ClientMessage;

void fillNextLine(int fd, char nextLine[]);
void fillNextPos(char *nextLine, int nchild, Position *nextPos);

void sendMessages(int *board_ptr, Acknowledgment *acklist_ptr, Position pos, Message messages[], int *nMessages);
int scanBoard(int *shm_ptr, Position pos, int max_distance, int *scanPid);
