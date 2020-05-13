/// @file client.c
/// @brief Contiene l'implementazione del client.

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

#include "defines.h"
//#include "message_fifo.h"

int main(int argc, char * argv[]) {

   if (argc != 1) {
        printf("Il nome dell'eseguibile e' sbagliato\n");//DUE PROCESSI, IN QUESTO CASO SCOLLEGATI L'UNO DALL'ALTRO, NON POSSONO CONDIVIDERE IL NOME DI UN file SE NON GLIELO PASSIAMO A MENO CHE NON UTILIZZIAMO LA MEMORIA CONDIVISA.
        return 0;
    }

  //LEGGE IL PATHNAME DELLA FIFO
  char *path2ServerFIFO = "/tmp/dev_fifo.DevicenumDev";

  printf("<Client - Device> opening FIFO %s...\n", path2ServerFIFO);

  int serverFIFO = open(path2ServerFIFO, O_WRONLY);
  if (serverFIFO == -1)
      errExit("open failed");


  /*int rows = 0;  
  char c;
  do {  
      printf("\nCaro utente, inserisci un numero da 1 a 5 (il numero del device a cui vuoi inviare il tuo messaggio):\n ");
  } while (((scanf("%d%c", &rows, &c)!=2 || c!='\n') && clean_stdin()) || rows<1 || rows>5);//Usare scanf("%d",&rows) invece di scanf("%s",input) ci consente di ottenere direttamente il valore intero da stdin senza la necessità di convertirlo in int. Se l'utente immette una stringa contenente caratteri non numerici è necessario pulire lo stdin prima del successivo scanf("%d",&rows).

 // struct messagefifo MessageFifo;*/

  char messaggio[256];  

  printf("Caro utente, inserisci il tuo messaggio: \n ");
  scanf("%s ", messaggio);

  int len1 = strlen(messaggio);

  printf("<Client - Device> sending '%s'\n", messaggio);

  if (write(serverFIFO, messaggio, len1) != len1)
        errExit("write failed");

  // Close the FIFO
  if (close(serverFIFO) != 0)
      errExit("close failed");
}
