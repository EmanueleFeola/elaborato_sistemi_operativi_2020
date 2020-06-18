// I campi del messaggio da inviare ad un device possono essere inseriti da linea di comando
// richiamando lo script in ./bash_scripts/client_hub.sh
// ulteriori info sullo script nel readme in tale cartella

#include "defines.h"
#include "../utils/err_exit.h"
#include "../utils/fifo.h"
#include "../utils/msg_queue.h"
#include "../utils/print_utils.h"
#include <sys/stat.h>
#include <unistd.h>

void setClientSignalMask();
void clientSigHandler(int sig);

/*
 * @description: scrive su file il resoconto degli ack scambiati tra i device 
 * @param cm: messaggio letto dalla message queue che contiene gli ack inviati dall'ackManager
 * @param message_id: message_id da scrivere su file
 * @param message: message da scrivere su file
 */
void writeClientMessage(ClientMessage cm, int message_id, char *message);

/*
 * @description: converte un timestamp in una data human readable 
 * @param date: buffer stringa da popolare con la data convertita in formato hr
 * @param size: dimensione del buffer
 */
void timestampToDate(time_t timestamp, char date[], int size);