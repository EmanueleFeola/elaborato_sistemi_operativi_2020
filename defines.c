/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>     //atoi
#include <math.h>

#define max_distance sqrt(2)   

double CalculateDistance(int row1, int col1, int row2, int col2){
    //double diffx = x1 - x2;
    double diffx = row2 - row1;
    double diffy = y1 - y2;
    double diffx_sqr = diffx * diffx;
    double diffy_sqr = diffy * diffy;
    double distance = sqrt (diffx_sqr + diffy_sqr);

return distance;
}

void fillNextLine(int fd, char input[]){//char input[] --> gli passo un puntatore alla zona di memoria del buffer e lui andra' a scrivere da quel puntatore in poi per contenere la linea successiva. 
    char row[50] = {0};   // contiene la riga successiva
    char buffer[2] = {0}; // contiene byte successivo,si poteva mettere anche [1] ma meglio 2 per essere sicuri

    while(read(fd, buffer, 1) != 0 && strcmp(buffer, "\n") != 0)// leggo fino al \n (all'inizio leggo la prima riga),finche' non hai finito di leggere i bytes (!=0) && fino a che non incontro \n, 1 e' il numero di bytes che leggo alla volta e viene salvato nel buffer
        strcat(row, buffer);//ogni volta che leggo un carattere lo vado a salvare nella stringa row (row e' tutta la riga "0,0|2,1|2,0|0,1|1,2") che andra' a definirmi la mia effettiva riga. TEORIA!!! -->  "char *strcat(char *dest, const char *src);" --> La funzione strcat() concatena la stringa src aggiungendola al termine della stringa dest. La concatenazione avviene copiando la stringa src a partire dal terminatore della stringa desc. La copia di src e' comprensiva del suo terminatore. L'area di memoria puntata da dest deve essere sufficientemente ampia da accogliere entrambe le stringhe ed il terminatore '\0'

    // (debug purposes only) se ho finito le righe ricomincia da capo 
    //if(strlen(row) == 0){
    //    lseek(fd, 0, SEEK_SET); 
    //    fillNextLine(fd, input);
    //} else{
        memcpy(input, row, strlen(row)+1);
    } 
}

// 0,0|1,0|2,0|0,1|1,0
void fillNextMove(char *nextLine, int nchild, nextMove_t *nextMove){
    int pipeCounter;// serve per contare quante barre ci sono (mi conta quante | ho gia' visto nella mia stringa, ogni volta per scorrere la stringa pero' devo aumentare il contatore che punta alla stringa ovvero nextLine perche' se non incremento rimango sempre sullo stesso carattere), se sono il child 0 non devo aspettarmi nessuna barra (RICORDA CHE LEGGI COSI I DATI --> figlio 0 --> 0,0 --> figlio 1 --> |2,1 (il counter sara' a 1 e quindi a 1 barra, cosi capisco che con 1 barra sono al figlio 2) --> figlio 2 --> |2,0 (il counter sara' a 2 e quindi a 2 barre, cosi capisco che con 2 barre sono al figlio 3)e cosi via, insomma la barra la usi come INIZIO per capire il figlio successivo non come FINE per capire il figlio precedente perche' se usassi quest'ultima tecnica per l'ultimo figlio saresti fregato dato che non avresti barre finali)
    // printf("fillNextMove (%d): %s --> ", nchild, nextLine);

    for(pipeCounter = 0; *nextLine != '\0' && pipeCounter < nchild; nextLine++)//'\0' nel nostro caso e' fine riga perche' nel buffer salviamo solo una riga non tutte quante. ogni volta che scorro la stringa nel for e non ho incontrato abbastanza | allora vado avanti con il mio puntatore della stringa. All'inizio il mio *nextLine punta al primo carattere della stringa quindi a 0 --> *nextLine sotto e' == '|' ?? no perche' e' uguale a 0 come abbiamo appena detto --> con nextLine++ passo quindi alla virgola --> *nextLine e' == '|' ?? no e' uguale a virgola. Quando arrivo a '|' vuol dire che sono arrivato nel mio blocco, devo fermarmi e analizzare quello che viene dopo.
        if(*nextLine == '|')
            pipeCounter++;

    //printf("%s\n", nextLine);//qui ti stampa:   1,2
                             //                   0,1|1,2
                             //                   2,0|0,1|1,2
                             //                   2,1|2,0|0,1|1,2
                             //                   0,0|2,1|2,0|0,1|1,2
                             //0,0|2,1|2,0|0,1|1,2 --> si riferisce al processo con nchild = 0 xk quando faccio la printf mi stampa appunto l'intera stringa e vuol dire che lui non ha mosso il puntatore e che l'ha scritta tutta mentre la prima riga 1,2 e' l'ultimo processo con nchild maggiore perche' lui ha mosso il puntatore nextLine su tutta la stringa fino ad incontrare l'ultima pipe e quando facciamo la "printf("%s\n", nextLine);", dato che il puntatore si e' mangiato tutta la stringa, ci rimangono solo quei 3 caratteri da stampare. MI STAMPA AL CONTRARIO PERCHE' I SEMAFORI PARTONO DALL'ULTIMO FIGLIO. L'ALGORITMO SAREBBE ANCHE CORRETTO E MI ASPETTEREI INVECE QUESTO OUTPUT: 
                             //                 0,0|2,1|2,0|0,1|1,2
                             //                 2,1|2,0|0,1|1,2
                             //                 2,0|0,1|1,2
                             //                 0,1|1,2
                             //                 1,2

    // printf("%s\n", nextLine);

    // fino alla virgola --> row
    char buffer[strlen(nextLine)]; // al massimo la riga è lunga strlen(nextLine) ossia massimo 19 caratteri. E' un buffer che prima mi conterra' il valore della row e poi il valore della col, riutilizzo sempre lo stesso buffer
    int index;
    
    for(index = 0; *nextLine != ','; nextLine++, index++)//per ogni carattere che c'e' prima della virgola metti il mio carattere letto dentro str. NOI STIAMO LAVORANDO A BLOCCHI NEL SENSO CHE QUESTO E' IL PRIMO nchild:
    //0,0|
    //0,0|
    //0,0|
    //QUI QUINDI NEL str[index] SALVEREMO 000 (QUELLI PRIMA DELLA VIRGOLA) RISPETTIVAMENTE IN str[0], str[1] E str[2]. PRENDENDO IL FIGLIO 1 AVREMO IL BLOCCO:
    //|2,1|
    //|1,0|
    //|2,0|
    //QUI QUINDI NEL str[index] SALVEREMO 212 (QUELLI PRIMA DELLA VIRGOLA)

    /*-----------------------------------------------
              MOLTO IMPORTANTE!!!!!!!!!!!
      ----------------------------------------------- 
      
    */
        buffer[index] = *(nextLine);
    
    nextMove->row = atoi(buffer);

    
    nextLine++; // skippa la virgola in modo tale da andare, negli esempi sopra rispettivamente, in 0 0 0 e 1 0 0
    

    memset(buffer, 0, sizeof(buffer));// reset stringa (altrimenti ottengo valore sporco in col) dato che dentro str ho ancora la riga per adesso     
    
    // dalla virgola alla | --> col
    for(index = 0; *nextLine != '\0' && *nextLine != '|'; nextLine++, index++)
        buffer[index] = *(nextLine);
    
    nextMove->col = atoi(buffer);
}

void checkEuclideanDistance(int nchild, char *fifoPath, nextMove_t *nextMove, nextMove_t *nextMove_nchild){

  double distance1 = CalculateDistance(nextMove->row,  nextMove->col, nextMove_nchild->row, nextMove_nchild->col);   

  printf("Distance\n");  
  printf("%6.1f\n", distance1);   

  printf("\n%4.3f\n\n", max_distance);   

  if(distance1 > max_distance)  
    printf("La distanza non permette di scambiare il messaggio!\n");
  else
    printf("La distanza ci permette di scambiare il messaggio!\n");  
}
