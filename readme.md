## Elaborato Sistemi Operativi 2020

-  ***Gestione dell'univocità del message_id***

Per far sapere ad un device che il messaggio nella sua fifo non gli è stato mandato da un altro ma da un client, ho deciso di "nascondere" questa informazione nel message_id stesso.

Il bit meno significativo del message_id viene messo a 1 dal client per notificargli che è un client a mandargli il messaggio, e che quindi il device deve controllare nell'ack list che non ci sia già un message_id uguale a quello inviato. In tal caso il client viene notificato di questa incorenza.

Poi il device stesso setta a 0 il bit meno significativo prima di scriverlo nell'ack list e prima di inviarlo ad un altro device.

**Vantaggi**: senza implementare altri canali di comunicazione tra device e client si riesce con poche righe di codice e con una semplice manipolazione di bit a far passare questa informazione sia tra il client e il device, sia tra device e device

**Svantaggi**: il numero massimo del message_id è più basso perchè un bit viene speso per differenziare l'orgine del messaggio (client o device). 

Tuttavia dato che il message_id è di tipo int, si può ipotizzare che il numero massimo di message_id non sia importante, poichè in tal caso sarebbe stato più opportuno definirlo come uint32, in modo da non sprecare bit per i message_id negativi che non sono accettati nell'elaborato.  

- ***Gestione Board dei device***

La board dei device è una matrice ROWSxCOLS, tuttavia è stata rappresentata e gestita come se fosse un array monodimensionale di dimensione ROWS * COLS. 

Quindi per accedere alla cella nella posizione 4, 2 si usa l'indice 4 * COLS + 2
    
QUICK START
clear && make clean && make && ./server 123 ./input/file_posizioni.txt
clear && source client_hub.sh 123 1 19690 5 hola 5