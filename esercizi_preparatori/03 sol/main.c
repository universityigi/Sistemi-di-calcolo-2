#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "common.h"

int pipefd[2];

int write_to_pipe(int fd, void* data, size_t data_len) {
    
    /**
     * SOLUZIONE
     * 
     * Obiettivi:
     * - scrivere sul descrittore i primi 'data_len' bytes contenuti in data
     * - gestire eventuali interruzioni ed errori
     * - assicurarsi che tutti i 'data_len' byte siano stati scritti
     * - restituire il numero di bytes scritti
     **/
     
    int ret;
    int written_bytes = 0;
    while (written_bytes < data_len) {
        ret = write(fd, data + written_bytes, data_len - written_bytes);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Errore nella scrittura su pipe");
        written_bytes += ret;
    }
    return written_bytes;
}

int read_from_pipe(int fd, void* data, size_t data_len) {
    
    /**
     * SOLUZIONE
     * 
     * Obiettivi:
     * - leggere dal descrittore 'data_len' bytes e memorizzarli in data
     * - gestire eventuali interruzioni ed errori
     * - in caso di chiusura inaspettata della pipe, ritornare immediatamente -1
     * - assicurarsi che tutti i 'data_len' bytes siano stati letti
     * - restituire il numero di bytes letti
     **/
     
    int ret;
    int read_bytes = 0;
    while (read_bytes < data_len) {
        ret = read(fd, data + read_bytes, data_len - read_bytes);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Errore nella lettura da pipe");
        if (ret ==  0) handle_error("Chiusura inattesa della pipe");
        read_bytes += ret;
    }
    return read_bytes;
}

/**
 * Questa funzione scrive nell'array di interi 'data' un numero pari a 
 * 'elem_count' di interi, tutti aventi lo stesso valore 'value'.
 **/
void create_msg(int *data, int elem_count, int value) {
    int i;
    for (i = 0; i < elem_count; i++)
        data[i] = value;
}

/**
 * Questa funzione verifica se un messaggio sia integro controllando se tutti 
 * gli 'elem_count' valori interi contenuti nell'array 'data' abbiano lo 
 * stesso valore.
 * In caso positivo, ritorna 1 (true), altrimenti 0 (false).
 **/
int is_msg_ok(const int *data, int elem_count) {
    int i;
    for (i = 0; i < elem_count; i++)
        if (data[0] != data[i])
            return 0;
    return 1;
}

void child(int child_id, sem_t* named_semaphore) {
    
    int data[MSG_SIZE];
    
    /**
     * SOLUZIONE
     * 
     * Obiettivi:
     * - chiudere i descrittori non necessari
     * - inviare MSG_COUNT messaggi usando la funzione write_to_pipe(); ogni 
     *   messaggio va prima creato usando la funzione create_msg(), specificando
     *   'child_id' come valore dell'argomento 'elem_count'; per memorizzare i 
     *   messaggi, si consiglia di usare l'array 'data' già dichiarato; per 
     *   gestire la mutua esclusione nella scrittura su pipe, usare il named 
     *   semaphore creato prima
     * - NB: il numero di byte da scrivere è diverso dal numero di interi 
     *   contenuti nel messaggio!!!
     * - infine, chiudere i descrittori rimanenti ed il named semaphore
     * - gestire eventuali errori
     **/
    
    printf("Sono il figlio: %d \n",child_id);
    
    int ret = close(pipefd[0]);
    if(ret) handle_error("Errore nella chiusura del descrittore di lettura della pipe");
    
    int i;
    for (i = 0; i < MSG_COUNT; i++) {
        create_msg(data, MSG_SIZE, child_id);
        
        ret = sem_wait(named_semaphore);
        if(ret) handle_error("Errore nella wait");
        
        write_to_pipe(pipefd[1], data, MSG_SIZE * sizeof(int));
        
        ret = sem_post(named_semaphore);
        if(ret) handle_error("Errore nella post");
        
        printf("Process %d sent msg #%d\n", child_id, i);
    }
    
    ret = close(pipefd[1]);
    if(ret) handle_error("Errore nella chiusura del descrittore di scrittura della pipe");
    
    ret = sem_close(named_semaphore);
    if(ret) handle_error("Errore nella chiusura del named semaphore");
}

void parent() {
    
    int data[MSG_SIZE];
    
    /**
     * SOLUZIONE
     * 
     * Obiettivi:
     * - chiudere i descrittori non necessari
     * - ricevere CHILDREN_COUNT*MSG_COUNT messaggi usando la funzione 
     *   read_from_pipe(); per memorizzare i messaggi, si consiglia di usare 
     *   l'array 'data' già dichiarato; verificare se ogni messaggio sia 
     *   integro usando la funzione is_msg_ok(), ed in caso negativo stampare 
     *   un messaggio su terminale
     * - NB: il numero di byte da leggere è diverso dal numero di interi 
     *   contenuti nel messaggio!!!
     * - infine, chiudere i descrittori rimanenti, attendere il termine di tutti
     *   i processi figlio, e rimuovere il named semaphore
     * - gestire eventuali errori
     **/
     
    int ret = close(pipefd[1]);
    if(ret) handle_error("Errore nella chiusura del descrittore di scrittura della pipe");
    
    int i;
    for (i = 0; i < CHILDREN_COUNT * MSG_COUNT; i++) {
        ret = 0;
        ret = read_from_pipe(pipefd[0], data, MSG_SIZE * sizeof(int));
        //if(ret) handle_error("Errore nella lettura da pipe");
        
        printf("Main process received msg #%d from process %d\n", i, data[0]);
        if (!is_msg_ok(data, MSG_SIZE))
            printf("WARN: msg #%d is malformed!\n", i);
    }
    
    ret = close(pipefd[0]);
    if(ret) handle_error("Errore nella chiusura del descrittore di lettura della pipe");
    
    int status;
    for (i = 0; i < CHILDREN_COUNT; i++) {
        ret = wait(&status);
        if(ret == -1) handle_error("Errore nella wait");
        if (WEXITSTATUS(status)) {
            handle_error("ERROR: child process died unexpectedly");
        }
    }
    
    ret = sem_unlink(SEMAPHORE_NAME);
    if(ret) handle_error("Errore nella rimozione del named semaphore");
}

int main(int argc, char* argv[]) {
    
    /**
     * SOLUZIONE
     * 
     * Obiettivi:
     * - assicurarsi che non esista un named semaphore di nome SEMAPHORE_NAME
     * - creare una pipe e salvare i descrittori nell'array 'pipefd' già 
     *   dichiarato
     * - creare un named semaphore con nome 'SEMAPHORE_NAME' per gestire la 
     *   mutua esclusione nella scrittura su pipe;
     * - creare CHILDREN_COUNT processi figlio paralleli; il processo figlio 
     *   i-esimo deve invocare la funzione child(i,named_semaphore)
     * - dopo aver lanciato i processi figlio, il processo padre deve chiudere il 
     *   semaforo named ed eseguire la funzione parent()
     * - NB: nessun processo figlio deve creare altri processi o eseguire la 
     *   funzione parent()
     * - gestire eventuali errori
     **/
    
    
    int ret = pipe(pipefd);
    if(ret) handle_error("Impossibile creare la pipe");
    
    sem_unlink(SEMAPHORE_NAME);
    sem_t* named_semaphore = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (named_semaphore == SEM_FAILED) handle_error("Impossibile creare il named semaphore");
    
    int i;
    pid_t pid;
    for (i = 0; i < CHILDREN_COUNT; i++) {
        pid = fork();
        if (pid == -1) handle_error("Errore nella creazione di un figlio");
        if (pid == 0) {
            child(i,named_semaphore);
            _exit(0);
        }
    }
    
    ret = sem_close(named_semaphore);
    if(ret) handle_error("Errore nella chiusura del semaforo");
    parent();
    
    return EXIT_SUCCESS;
}
