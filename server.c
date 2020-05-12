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


  char c[1000];
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
 
  return 0;

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
