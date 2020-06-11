#include "defines.h"
#include "../utils/fifo.h"
#include "../utils/shared_memory.h"

void startDevice(char *positionFilePath, int semid, int nchild, int board_shmid, int acklist_shmid);
void setDeviceSignalMask();
void deviceSigHandler(int sig);
void waitP(int semid, int nchild);
void signalV(int semid, int nchild);
void updateMyAcks(Message *messagesToSend, int nMessages);
void checkMessages(int fd, Message messages[], int *nMessages);