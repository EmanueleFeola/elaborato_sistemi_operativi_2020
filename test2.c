#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int fd, fd_extra;
char *filename = "emptyFile.txt";

void startServer(char *filename);

void closeFIFO(int sig){
    if(sig == SIGALRM)
        printf("<Server> - Time expired!\n");
    else
        printf("<Server> - Stopped...\n");
    
    close(fd);

    if (fd != 0 && close(fd_extra) == -1)
        printf("close failed");

    unlink(filename);
    exit(0);
}


int main(){   
    if(mkfifo(filename, S_IRWXU) == -1)
        printf("mkfifo");

    printf("<Server> - Built FIFO\n");

    startServer(filename);
}

void startServer(char *filename){    
    printf("<Server> - FIFO created!\n");

    if(fd == -1)
        printf("open fifo");
    
    signal(SIGALRM, closeFIFO);

    alarm(10);

    printf("<Server> waiting for a client...\n");
    fd = open(filename, O_RDONLY);

    fd_extra = open(filename, O_WRONLY);
    if (fd_extra == -1)
        printf("open write-only failed");

    int bR = -1;
    int buffer[] = {0, 1};

    do{
        read(fd, buffer, sizeof(int) * 2);

        alarm(0);

        printf("<Server> - Received: %d %d\n", buffer[0], buffer[1]);

        alarm(5);
    } while(buffer[0] != buffer[1]);

    closeFIFO(0);
}


