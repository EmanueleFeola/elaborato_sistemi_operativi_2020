#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

void startClient(char *filename);

/*
Estendere Ese_1 affinche’ il server continui a leggere messaggi dalla FIFO creata.
Il server rimuove la FIFO ed infine termina solo quando i due numeri ricevuti sono uguali, o dall’ultimo messaggio ricevuto sono
passati piu’ di 30 secondi.
*/

int main(){
    startClient("emptyFile.txt");
}

void startClient(char *filename){
    int fd = open(filename, O_WRONLY);

    printf("<Client> Opened FIFO\n");

    int buffer[] = {0, 0};
    int wBytes;

    do{
        printf("<Client> Inserisci due numeri: \n");
        scanf("%d %d", &buffer[0], &buffer[1]);

        wBytes = write(fd, buffer, sizeof(int) * 2);

        if(wBytes < 0)
            printf("<Client> - FIFO has been closed by server\n");
        else{
            printf("<Client> - Sending: %d %d\n", buffer[0], buffer[1]);
        }
    }while(wBytes > 0);

    printf("<Client> - Closing my side\n");

    close(fd);
    unlink(filename);
    exit(0);
}