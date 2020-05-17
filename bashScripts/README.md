# **Breve descrizione script bash**

kill_ipcs.sh

- libera tutte le ipcs aperte dall'utente che esegue lo script

------

client_hub.sh

##### Sommario

- serve per automatizzare la creazione dei processi client
- dati i parametri passati da linea di comando crea n processi client a cui vengono passati gli stessi parametri
  - message_id e message vengono modificati in modo da non avere messaggi doppioni con lo stesso message_id e lo stesso message, altrimenti i messaggi sono indistinguibili e difficili da analizzare nella fase di debug 

##### Parametri

- times_to_send (int): quanti client devo avviare
- pid_receiver (int): pid del device a cui mandare il messaggio creato dal client
- message_id (int): id del messaggio
  - se vengono avviati più client, message_id rappresenta il message_id di partenza, per ogni client aggiuntivo il message_id verrà incrementato di 1
- message (string): il messaggio che il client manda al device
  - se vengono avviati più client, ad ogni messaggio viene aggiunto in append il message_id
- max_distance (int): massima distanza a cui il device può inviare messaggi

##### Esempio di esecuzione

![image-20200516214321032](/home/emanuele/Documents/so/progetto/elaborato_sistemi_operativi_2020/bashScripts/client)