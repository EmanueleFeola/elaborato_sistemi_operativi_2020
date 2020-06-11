/// @file msg_queue.h
/// @brief Contiene la definizione delle funzioni per la gestione delle message queues.

#pragma once

#include "../inc/defines.h"
#include <sys/msg.h>
#include "err_exit.h"

int getMsgQueue(key_t key, int flags);
void readMsgQueue(int msqid, ClientMessage *msgp, size_t msgsz, long msgtype, int msgflg);
void writeMsgQueue(int msqid, ClientMessage *msgp, size_t msgsz, int msgflg);