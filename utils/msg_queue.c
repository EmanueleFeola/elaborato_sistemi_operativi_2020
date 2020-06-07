#include "../defines.h"
#include "msg_queue.h"

int getMsgQueue(key_t key, int flags){
    int msqid = msgget(key, flags);

    if(msqid == -1)
        ErrExit("msgget failed");

    return msqid;
}

void readMsgQueue(int msqid, ClientMessage *msgp, size_t msgsz, long msgtype, int msgflg){
    if (msgrcv(msqid, msgp, msgsz, msgtype, msgflg) == -1)
        ErrExit("msgrcv failed");
}

void writeMsgQueue(int msqid, ClientMessage *msgp, size_t msgsz, int msgflg){
    if (msgsnd(msqid, msgp, msgsz, msgflg) == -1)
        ErrExit("msgsnd failed");
}