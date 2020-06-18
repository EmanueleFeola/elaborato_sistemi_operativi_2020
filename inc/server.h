#include "defines.h"
#include "../utils/shared_memory.h"
#include "../utils/fifo.h"
#include "../utils/print_utils.h"
#include "device.h"
#include "ackManager.h"

void setServerSigMask();
void serverSigHandler(int sig);
void freeResources();
void initDevices(char *positionFilePath);
void initAckManager(int msgQueueKey);
