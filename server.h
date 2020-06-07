#include "defines.h"
#include "utils/shared_memory.h"
#include "utils/fifo.h"
#include "utils/print_utils.h"
#include "device.h"
#include "ackManager.h"

void setServerSigMask();
void serverSigHandler(int sig);
void initDevices(char *positionFilePath);
void freeResources();
void initAckManager(int msgQueueKey);
