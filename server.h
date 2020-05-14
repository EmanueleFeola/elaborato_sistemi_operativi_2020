/*
Il processo server può essere terminato solamente per mezzo di un segnale SIGTERM. 
La ricezione di tale segnale da parte del server gestisce la terminazione di tutti i processi device,
del processo ack_manager e la chiusura di tutti i meccanismi di comunicazione/sincronizzazione tra processi (memoria condivisa, semafori, fifo, etc.).
Ogni altro segnale non strettamente necessario per l’esecuzione del programma deve essere bloccato.
*/

void setServerSignalMask();
void serverSigHandler(int sig);

void initDevices();

void freeResources();
