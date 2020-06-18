#include "defines.h"

#include "../utils/array_utils.h"
#include "../utils/shared_memory.h"
#include "../utils/print_utils.h"

void deviceSigHandler(int sig);
void setDeviceSignalMask();
void write_ack(Acknowledgment *ptr, Acknowledgment ack);
void read_acks(Acknowledgment *ptr); // debug purposes only
void startAckManager(int semid, int acklist_shmid, key_t key);

/*
 * @description: ordina gli ack per timestamp (decrescente)
 * @param cm: client message di cui riordinare gli ack
 * @notes: come primo Ack di cm->acks troverò l ack con il timestamp più grande, ovvero quello più recente
 */
void sortAck(ClientMessage *cm);

/*
 * description: controlla il segmento di shared memory per vedere se qualche messaggio è stato ricevuto da tutti i device,
 * in caso positivo manda la lista di ack al client
 * @param ptr: puntatore al segmento di memoria condivisa (ack list)
 * @param msgid: id della message queue tra ackManager e Client
 * @notes: body dell'ackManager
 */
void ackManagerRoutine(Acknowledgment *ptr, int msgid);

/*
 * @description: ritorna il numero di occorrenze di un message_id nella acklist
 * @param ptr: puntatore al segmento di memoria condivisa (ack list)
 * @param message_id: message id da cercare
 * @return: numero di Acknowledgment con message_id passato come parametro
 */
int acklist_countByMsgId(Acknowledgment *ptr, int message_id);

/*
 * @param ptr: puntatore al segmento di memoria condivisa (ack list)
 * @param message_id: message id da cercare
 * @return: 1 se è presente un Acknowledgment con message_id passato come parametro, altrimenti -1
 * @notes: meglio chiamare acklist_countByMsgId invece che acklist_countByMsgId se 
 * ho bisogno solo di sapere se un certo message_id è già stato inserito, perchè evito di scorrere tutta la ack_list
 * (apparte nel caso peggiore in cui l unica occorrenza è all'ultimo indice dell'ack_list) 
 */
int acklist_contains_message_id(Acknowledgment *ptr, int message_id);

/*
 * @descrizione: cerca se un certo device identificato da pid_receiver ha scritto o meno un Ack con message_id passato come parametro
 * @param ptr: puntatore al segmento di memoria condivisa (ack list)
 * @param message_id: message id da cercare
 * @param pid_receiver: identifica il device da cercare
 * @return: 1 se il device con pid <pid_receiver> ha scritto un Acknowledgment con message_id passato come parametro, altrimenti -1
 */
int acklist_contains_tupla(Acknowledgment *ptr, int message_id, pid_t pid_receiver);