#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void getLine(int index, int fd, char input[]){

    lseek(fd, 0, SEEK_SET);

    int rowCounter = 0;

    char row[100] = {0};
    char buffer[10] = {0};

    int counter = 0;

    while(read(fd, buffer, 1) != 0){
        
        if(strcmp(buffer, "\n") != 0)
            strcat(row, buffer);
        
        else{
            rowCounter++;

            if(rowCounter > index)
                break;

            memset(row, 0, sizeof(row)); 
        }
    }

    memcpy(input, row, strlen(row)+1);
}

int main(){
    int fd = open("./input/file_posizioni.txt", O_RDONLY, 0 /* ignored */);
    if(fd == -1)
        printf("errore apertura file\n");

    char row[100] = {0}; 


    int counter = 0;

//    while(counter < 5){
        getLine(0, fd, row);
        printf("%s\n", row);
        counter++;
//    }

        getLine(1, fd, row);
        printf("%s\n", row);

    return 0;
}