#include "inc/client.h"
#include <time.h>

void setClientSignalMask(){//ASSEGNA AL SEGNALE SIGUSR1 LA FUNZIONE clientSigHandler. QUINDI QUANDO SIGUSR1 VIENE RICEVUTO VIENE RICHIAMATA clientSigHandler E QUEST'ULTIMA TERMINA IL PROCESSO
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigprocmask(SIG_SETMASK, &set, NULL);

    signal(SIGUSR1, clientSigHandler);
}

void clientSigHandler(int sig){//E QUEST'ULTIMA TERMINA IL PROCESSO
    coloredPrintf("red", 1, "<client %d> message_id is not unique, terminating\n", getpid());
    exit(0);
}

int main(int argc, char * argv[]) {
    setClientSignalMask();//SETTA LA MASCHERA DI SEGNALI CHE CI INTERESSANO

    if(argc != 2)
        ErrExit("<client> incorrect params: server usage is\n./client msg_queue\nor\n./client msg_queue_key pid_receiver message_id message max_distance");

    int msgQueueKey = atoi(argv[1]);//CONVERTE LA CHIAVE INSERITA DA STRINGA AD INTERO
    if (msgQueueKey <= 0)
        ErrExit("<client> incorrect params: msg_queue_key must be greater than zero");

    Message msg; //INIZIALIZZO LA STRUTTURA Message 
    msg.pid_sender = getpid();//DO' UN PID AL CLIENT CHE E' IL NOSTRO CAMPO pid_sender NELLA NOSTRA STRUTTURA Message CHIAMATA QUI LOCALMENTE msg 

    // fifo path
    char fifoBasePath[20] = "/tmp/dev_fifo.";
    char fifoPath[50] = {0};
    sprintf(fifoPath, "%s", fifoBasePath);

    //IN QUESTA ISTRUZIONE SUCCESSIVA E NEI SUCCESSIVI do-while FACCIO INSERIRE I DATI ALL'UTENTE E QUESTI DATI VERRANO SALVATI NEI CORRISPONDENTI CAMPI DELLA struct Message msg
    coloredPrintf("cyan", 0, "<client %d> Insert message: \n", getpid());
    scanf("%[^\n]%*c", msg.message);//ACCETTO ANCHE STRINGHE CON GLI SPAZI

    do{//QUI FACCIO INSERIRE IL PID DI DESTINAZIONE ALL'UTENTE
        char pidString[7] = {0};
        coloredPrintf("cyan", 0, "<client> Insert destination PID:\n");
        scanf("%s", pidString);
        sprintf(fifoPath, "%s%s", fifoPath, pidString);//QUI CONCATENO LA VERSIONE "STRINGA" DEL NOSTRO PID DATO CHE NEL PATH DELLA FIFO MI SERVIRA' COME STRINGA
        msg.pid_receiver = atoi(pidString);//MENTRE QUI LO CONVERTO IN NUMERO PERCHE' GIUSTAMENTE NEL CAMPO pid_receiver DELLA NOSTRA STRUTTURA MI SERVIRA' COME INTERO
    } while(msg.pid_receiver < 1);

    do{
        coloredPrintf("cyan", 0, "<client %d> Insert message id: \n", getpid());
        scanf("%d", &msg.message_id);
    } while(msg.message_id < 0);

    do{
        coloredPrintf("cyan", 0, "<client %d> Insert max distance: \n", getpid());
        scanf("%d", &msg.max_distance);
    } while(msg.max_distance < 1);
    

    // trick per rendere message_id univoco
    // il bit meno significativo è un 1 se il sender è un client
    // il bit meno significativo è uno 0 se il sender è un device
    msg.message_id <<= 1;
    msg.message_id |= 1;


    coloredPrintf("default", 0,"<client> Sending to fifo: %s\n", fifoPath);

    int fd = get_fifo(fifoPath, O_WRONLY);//QUANDO QUI FACCIO LA get_fifo MI ACCORGO SUBITO SE IL PID CHE MI HA INSERITO L'UTENTE E' SBAGLIATO OPPURE NO PERCHE' LA get_fifo MI RITORNA UN ERRORE PERCHE' NON RIESCE AD APRIRE UNA FIFO CHE NON ESISTE. CON LA get_fifo CI RESTITUISCE UN IDENTIFICATORE DELLA FIFO CON CUI POSSIAMO INTERAGIRE E SE GLI PASSIAMO UN NUMERO CHE NON ESISTE NON RIESCE A RITORNARCI UN PUNTATORE A fifo.numerosbagliato
    write_fifo(fd, msg);

    /* Waiting for ack list from ackManager */

    //QUI CI METTIAMO IN ATTESA SULLA MESSAGE QUEUE IN ATTESA CHE ACK MANAGER GLI MANDI IL MESSAGGIO FINALE CON TUTTI GLI ACK DEI MESSAGGI 
    ClientMessage cm;//PREPARO LA STRUTTURA DATI PER RICEVERE 5 Acknowledgment, FACCIO LA read E FILTRO PER msg.message_id. QUINDI QUANDO TUTTI I DEVICE HANNO RICEVUTO IL MESSAGGIO CHE ERA PARTITO DAL CLIENT ALL'INIZIO STAMPERO'
    size_t mSize = sizeof(ClientMessage) - sizeof(long);

    int msqid = getMsgQueue(msgQueueKey, IPC_EXCL | S_IRUSR | S_IWUSR);//QUI FACCIAMO UNA get DELLA MESSAGE QUEUE
    
    coloredPrintf("yellow", 1,"<client %d> Waiting for ack of type %d\n", getpid(), msg.message_id & 0xfffffffe);
    
    readMsgQueue(msqid, &cm, mSize, msg.message_id & 0xfffffffe, 0);//FACCIO UNA read,IL PARAMETRO msg.message_id STA DICENDO DI FILTRARE I MESSAGGI CHE ARRIVANO SULLA MESSAGE QUEUE PER msg.message_id PERCHE' IL CLIENT VUOLE LEGGERE SOLAMENTE GLI ACK DEL MESSAGGIO INIZIALE CHE HA MANDATO. SE LUI HA MANDATO UN MESSAGGIO CON ID = 5 LUI SI ASPETTA CHE GLI ARRIVINO TUTTI GLI ACK DI QUELLO SPECIFICO MESSAGGIO. INFATTI NELLA MESSAGE QUEUE FILTRO PER msg.message_id E CI METTO COME MASCHERA TUTTE f E POI e, CHE E' ESATTAMENTE QUELLO CHE FANNO I DEVICE. msg.message_id E' QUELLO DEL CLIENT.
        
    coloredPrintf("green", 0, "\n<client %d> ClientMessage received\n", getpid());

    writeClientMessage(cm, msg.message_id >> 1, msg.message);
    
    exit(0);
}

void writeClientMessage(ClientMessage cm, int message_id, char *message){//SCRIVE SU FILE LA LISTA DI ACK (CONTENUTA IN ClientMessage) CHE L'ackManager HA INVIATO AL Client

    char filename[50];//CREO IL filename DEL FORMATO VOLUTO DAL PROF
    sprintf(filename, "out_%d.txt", message_id);

    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);//APRO IL FILE
    if(fd == -1)
        coloredPrintf("red", 1, "open failed: %s\n", filename);

    char header[150];//CREO header --> COMINCIO A COMPORRE IL MESSAGGIO CHE DEVO SCRIVERE SUL FILE. IN header CI SCRIVO "Messaggio %d: %s\nLista acks:\n"
    sprintf(header, "Messaggio %d: %s\nLista acks:\n", message_id, message);

    if(write(fd, header, strlen(header)) == -1)
        ErrExit("failed write 1");

    char row[100];//QUESTA SARA' LA MIA RIGA EFFETTIVA PER ESEMPIO "134, 156, 2020-05-02 18:10:23". OVVIAMENTECI SARANNO 5 ROW DIVERSE PERCHE' QUESTA ROBA LA CICLO CON IL for SOTTO E OGNI VOLTA PULISCO CON UNA memset. OGNI VOLTA RIEMPIAMO CON LA NUOVA RIGA.

    Acknowledgment a;
    int counter; 
    for(counter = NDEVICES; counter > 0; counter--){//QUI VADO A SCRIVERE GLI ACK. SCORRO ALL'INDIETRO PERCHE' IL sortAck MI ORDINA GLI ACK AL CONTRARIO 
        a = cm.acks[counter - 1];
        a.message_id >>= 1; //ANCHE QUI FACCIO UNO shift PERCHE' A RIGA 84 (msg.message_id >> 1) NON HO MESSO msg.message_id >>= 1 E QUINDI DEVO CAMBIARLO IL BIT PER METTERLO A POSTO

        char date[50];
        timestampToDate(a.timestamp, date, 50);

        memset(row, 0, sizeof(row));
        sprintf(row, "%d, %d, %s\n", a.pid_sender, a.pid_receiver, date);//MI CREO LA RIGA CHE DEVO SCRIVERE 

        int bW = write(fd, row, strlen(row));//QUESTA write SCRIVE SU FILE LA RIGA DELL'ACK LIST. SCRIVO UNA RIGA E CONTINUO A CICLARE (QUINDI SCRIVERE) FINO A CHE TROVO ACK NEL ClientMessage (CE NE SARANNO OVVIAMENTE 5 PERCHE' I DEVICES SONO 5)
        if(bW == -1)
            ErrExit("failed write\n");
    }

    close(fd);
    coloredPrintf("green", 0, "<client %d> Finished writing on file\n", getpid());
}

void timestampToDate(time_t timestamp, char date[], int size){
    struct tm ts;//SERVER PER CONVERTIRE IL timestamp IN UNA RIGA HUMAN READABLE
    ts = *localtime(&timestamp);
    strftime(date, size, "%Y-%m-%d %H:%M:%S", &ts);
}