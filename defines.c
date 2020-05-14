/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>     /* atoi */

void fillNextLine(int fd, char input[]){
    char row[50] = {0};   // contiene la riga successiva
    char buffer[2] = {0}; // contiene byte successivo

    while(read(fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)
        strcat(row, buffer);

    // (debug purposes only) se ho finito le righe ricomincia da capo 
    if(strlen(row) == 0){
        lseek(fd, 0, SEEK_SET); 
        fillNextLine(fd, input);
    } else{
        memcpy(input, row, strlen(row)+1);
    } 
}

// 0,0|1,0|2,0|0,1|1,0
void fillNextMove(char *nextLine, int nchild, nextMove_t *nextMove){
    int counter = 0;
    // printf("fillNextMove (%d): %s --> ", nchild, nextLine);

    // dopo nchild barre verticali
    for(; *nextLine != '\0' && counter < nchild; ){
        if(*nextLine == '|')
            counter++;
    
        nextLine++;
    }

    // printf("%s\n", nextLine);

    // fino alla virgola --> row
    // dalla virgola alla | --> col

    char str[strlen(nextLine)]; // al massimo la riga è lunga strlen(nextLine)
    int index;
    
    for(index = 0; *nextLine != ','; nextLine++, index++)
        str[index] = *(nextLine);
    
    nextMove->row = atoi(str);

    nextLine++; // skippa la virgola

    memset(str, 0, sizeof(str)); // reset stringa (altrimenti ottengo valore sporco in col)
    
    for(index = 0; *nextLine != '\0' && *nextLine != '|'; nextLine++, index++)
        str[index] = *(nextLine);
    
    nextMove->col = atoi(str);
}