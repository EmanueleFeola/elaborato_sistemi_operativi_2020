#include "defines.h"

#include "utils/array_utils.h"
#include "utils/shared_memory.h"
#include "utils/print_utils.h"

void deviceSigHandler(int sig);
void setDeviceSignalMask();
void startAckManager(int semid, int acklist_shmid, key_t key);
void ackManagerRoutine(Acknowledgment *ptr, int msgid);
