#include "defines.h"
#include "../utils/fifo.h"
#include "../utils/shared_memory.h"

void startDevice(char *positionFilePath, int semid, int nchild, int board_shmid, int acklist_shmid);
void setDeviceSignalMask();
void deviceSigHandler(int sig);

/*
 * description: se ha messaggi nuovi, scrive il suo ack nella ack list
 * @param messagesToSend: array dei messaggi memorizzati dal device
 * @param nMessages: quanti messaggi ci sono nell'array di messaggi
 */
void updateMyAcks(Message *messagesToSend, int nMessages);

/*
 * description: controlla nella propria fifo se qualche device ha mandato un nuovo messaggio,
 * in tal caso se lo salva nel suo array di messaggi
 * @param messages: array dei messaggi memorizzati dal device
 * @param nMessages: quanti messaggi ci sono nell'array
 */
void checkMessages(int fd, Message messages[], int *nMessages);