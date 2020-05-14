/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

// number of devices
#define NDEVICES 5

// board settings
#define ROWS 5
#define COLS 5

typedef struct nextMove{
    int row;
    int col;
} nextMove_t;

void fillNextLine(int fd, char input[]);
void fillNextMove(char *nextLine, int nchild, nextMove_t *nextMove);