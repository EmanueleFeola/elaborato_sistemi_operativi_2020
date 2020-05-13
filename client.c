/// @file server.c
/// @brief Contiene l'implementazione del SERVER.

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "err_exit.h"
//#include "message_fifo.h"
#define BUFFER_SZ 100 //100 CARATTERI
char buffer[BUFFER_SZ + 1];
#define MAX_READ 100 //100 CARATTERI
char buffer[MAX_READ + 1];

int main (int argc, char *argv[]) {

  //CREO IL FIGLIO "Ack_Manager"
  pid_t Ack_Manager;
  Ack_Manager = fork();
  if (Ack_Manager == -1)
        printf("Ack_Manager non creato!");
  if(Ack_Manager == 0) 
      { 
        printf("[Ack_Manager] pid %d from [parent] pid %d\n\n", getpid(), getppid()); 
        exit(0); 
      }

  sleep(1);

  pid_t Device1;
  pid_t Device2;
  pid_t Device3;
  pid_t Device4;
  pid_t Device5;

  Device1 = fork();
      if (Device1 == -1)
            printf("Device 1 non creato!");
      if(Device1 == 0) 
        { 
          printf("[Device 1] pid %d from [parent] pid %d\n", getpid(), getppid()); 
          exit(0); 
        } 

  sleep(1);

  Device2 = fork();
      if (Device2 == -1)
            printf("Device 2 non creato!");
      if(Device2 == 0) 
        { 
          printf("[Device 2] pid %d from [parent] pid %d\n", getpid(), getppid()); 
          exit(0); 
        } 

  sleep(1);

  Device3 = fork();
      if (Device3 == -1)
            printf("Device 3 non creato!");
      if(Device3 == 0) 
        { 
          printf("[Device 3] pid %d from [parent] pid %d\n", getpid(), getppid()); 
          exit(0); 
        } 

  sleep(1);

  Device4 = fork();
      if (Device4 == -1)
            printf("Device 4 non creato!");
      if(Device4 == 0) 
        { 
          printf("[Device 4] pid %d from [parent] pid %d\n", getpid(), getppid()); 
          exit(0); 
        } 
  
  sleep(1);

  Device5 = fork();
      if (Device5 == -1)
            printf("Device 5 non creato!");
      if(Device5 == 0) 
        { 
          printf("[Device 5] pid %d from [parent] pid %d\n", getpid(), getppid()); 
          exit(0); 
        } 

  sleep(1);

char *path2ServerFIFO = "/tmp/dev_fifo.DevicenumDev";

printf("<Server - Device> Making FIFO...\n");
if (mkfifo(path2ServerFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)//IN BASE A DOVE VIENE GENERATA LA FIFO VEDREMO un file
        errExit("mkfifo failed");

    printf("<Server - Device> FIFO %s created!\n", path2ServerFIFO);
 //alla fine di questo for mi aspetto, tramite -ls, di vedere le 5 fifo create con i pid dei 5 devices

printf("<Server - Device> waiting for a client...\n");
serverFIFO = open(path2ServerFIFO, O_RDONLY);
if (serverFIFO == -1)
    errExit("open failed");

char messaggio[256];
int len1 = strlen(messaggio); 

printf("<Server> waiting for message...\n");

int bR = read(serverFIFO, messaggio, len1);

    // Checking the number of bytes from the FIFO
    if (bR == -1)
        printf("<Server - Device> it looks like the FIFO is broken/n");
    if (bR != len1 /*|| bR == 0*/)
        printf("<Server> it looks like I did not receive the message\n");
    else
        printf("<Server> Il tuo messaggio e' %s \n", messaggio);
    

// Close the FIFO
if (close(serverFIFO) != 0)
    errExit("close failed");

printf("<Server> removing FIFO...\n");
// Remove the FIFO
if (unlink(path2ServerFIFO) != 0)
    errExit("unlink failed");

























/*for(int numDev = 1; numDev <= 5; numDev++)
  char *path2ServerFIFO = "/tmp/dev_fifo.DevicenumDev";
*/
/*
  //int NDEVICES = 5;
  //int bytesPerLine = 4 * 2 * NDEVICES + 1 * NDEVICES + 1 * (NDEVICES-1);
  int fd_fileposizioni = open("./input/file_posizioni.txt", O_RDONLY);
  if (fd_fileposizioni == -1) 
      printf("File %s does not exist\n", argv[0]); 

  // A MAX_READ bytes buffer. 
  char buffer[MAX_READ + 1];
  
  // Reading up to MAX_READ bytes from myfile.    
  ssize_t numRead = read(fd_fileposizioni, buffer, MAX_READ); 
  if (numRead == -1) 
    ErrExit("read");

  buffer[numRead] = '\0';
  printf("%s \n", buffer);*/


  /*char c[1000];
    FILE *fptr;
    if ((fptr = fopen("./input/file_posizioni.txt", "r")) == NULL) {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
        exit(1);
    }
    // reads text until newline is encountered
    fscanf(fptr, "%[^\n]", c);
    printf("\n%s\n", c);
  
    fclose(fptr);
 
  return 0;*/

/*int NDEVICES = 5;
int bytesPerLine = 4 * 2 * NDEVICES + 1 * NDEVICES + 1 * (NDEVICES-1);

int fd = open("./input/file_posizioni.txt", O_RDONLY, 0);
if(fd == -1)
    printf("errore apertura file\n");

char buffer[bytesPerLine + 1];
int bR = -1; 

while((bR = read(fd, buffer, bytesPerLine + 1) != 0)){
    printf("%s", buffer);
    // ogni volta resetto il browser,
    // se no rimane sporco e si sminchia quando arriva all ultima riga
    // probabilmente si può fare di meglio
    memset(buffer,0,sizeof(buffer)); 
}

printf("\nfinished reading\n");*/
}
