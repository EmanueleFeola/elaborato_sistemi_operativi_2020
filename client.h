#include "defines.h"
#include "utils/err_exit.h"
#include "utils/fifo.h"
#include "utils/msg_queue.h"
#include "utils/print_utils.h"
#include <sys/stat.h>
#include <unistd.h>

// scrive su file il messaggio mandatogli da ackmanager
void writeClientMessage(ClientMessage cm, int message_id, char *message);
void setClientSignalMask();
void clientSigHandler(int sig);