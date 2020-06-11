#include "defines.h"

#include "../utils/array_utils.h"
#include "../utils/shared_memory.h"
#include "../utils/print_utils.h"

void deviceSigHandler(int sig);
void setDeviceSignalMask();
void startAckManager(int semid, int acklist_shmid, key_t key);
void ackManagerRoutine(Acknowledgment *ptr, int msgid);

void write_ack(Acknowledgment *ptr, Acknowledgment ack);
void read_acks(Acknowledgment *ptr);
int acklist_countByMsgId(Acknowledgment *ptr, int message_id);
int acklist_contains_message_id(Acknowledgment *ptr, int message_id);
int acklist_contains_tupla(Acknowledgment *ptr, int message_id, pid_t pid_receiver);