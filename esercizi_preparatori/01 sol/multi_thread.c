#include <pthread.h>
#include <semaphore.h>
#include "common.h"

#define BUFFER_LENGTH 5
#define THREAD_COUNT 4
#define ITERATION_COUNT 10
#define PROD_ROLE 0
#define CONS_ROLE 1

typedef struct thread_args_s {
    int idx;
    int role;
} thread_args_t;

int buffer[BUFFER_LENGTH];
int write_index, read_index;

/**
 * COMPLETARE QUI
 *
 * Obiettivi:
 * - dichiarare i semafori necessari
 *
 */

sem_t filled_sem;
sem_t empty_sem;
sem_t cons_sem;
sem_t prod_sem;
/***/


void producer_routine(int idx) {

    /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - completare l'implementazione secondo il paradigma
     *   multi-prod/multi-cons inserendo i semafori necessari al punto giusto.
     * - gestire gli errori
     *
     */

     int i, ret;
     for (i = 0; i < ITERATION_COUNT; i++) {
        int value = idx*ITERATION_COUNT + i;

        ret = sem_wait(&empty_sem);
        if(ret) handle_error("Error waiting on empty sem");
        
        ret = sem_wait(&prod_sem);
        if(ret) handle_error("Error waiting on prod sem");

        buffer[write_index] = value;
        write_index = (write_index + 1) % BUFFER_LENGTH;
        
        ret = sem_post(&prod_sem);
        if(ret) handle_error("Error waiting on prod sem");

        ret = sem_post(&filled_sem);
        if(ret) handle_error("Error posting on filled sem");

        printf("[Thread #%d] inserito %d nel buffer\n", idx, value);
     }
}

void consumer_routine(int idx) {

    /**
     * COMPLETARE QUI
     *
     * Obiettivi:
     * - completare l'implementazione secondo il paradigma
     *   multi-prod/multi-cons inserendo i semafori necessari al punto giusto.
     * - gestire gli errori
     *
     */

     int i, ret;
     for (i = 0; i < ITERATION_COUNT; i++) {
     
        ret = sem_wait(&filled_sem);
        if(ret) handle_error("Error on waiting on filled sem");
        
        ret = sem_wait(&cons_sem);
        if(ret) handle_error("Error on waiting on cons sem");

        int value = buffer[read_index];
        read_index = (read_index + 1) % BUFFER_LENGTH;

        ret = sem_post(&cons_sem);
        if(ret) handle_error("Error posting on cons sem");
        
        ret = sem_post(&empty_sem);
        if(ret) handle_error("Error posting on empty sem");

        printf("[Thread #%d] letto %d dal buffer\n", idx, value);
     }
}


void *thread_routine(void *args) {

    thread_args_t *thread_args = (thread_args_t*)args;
    if (thread_args->role == PROD_ROLE) {
        producer_routine(thread_args->idx);
    } else if (thread_args->role == CONS_ROLE) {
        consumer_routine(thread_args->idx);
    } else {
        printf("[Thread #%d] ruolo sconosciuto: %d\n", thread_args->idx, thread_args->role);
    }

    free(thread_args);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

    int ret;

    // inizializzazioni
    read_index = write_index = 0;

    /**
     * COMPLETARE QUI
     *
     * Obiettivi
     * - inizializzare i semafori dichiarati
     * - gestire gli errori
     */

     ret = sem_init(&cons_sem, 0, 1);
     if(ret) handle_error("Error on sem_init of consumer sem");
     
    ret = sem_init(&prod_sem, 0, 1);
     if(ret) handle_error("Error on sem_init of producer sem");

     ret = sem_init(&filled_sem, 0, 0);
     if(ret) handle_error("Error on sem_init of filled sem");

     ret = sem_init(&empty_sem, 0, BUFFER_LENGTH);
     if(ret) handle_error("Error on sem_init of empty sem");
    /***/

    /**
     * COMPLETARE QUI
     *
     * Obiettivi: lanciare THREAD_COUNT thread
     * - nota bene: successivamente bisogna attendere il termine di ogni
     *   thread
     * - per ogni thread, preparare prima i parametri da passargli
     *  (struttura thread_args_t)
     * - l'idx deve essere diverso per ogni thread
     * - il role può assumere come valori PROD_ROLE o CONS_ROLE, ci
     *   devono essere THREAD_COUNT / 2 thread produttore e THREAD_COUNT / 2  thread consumatori
     * - ogni thread lanciato deve eseguire la funzione thread_routine()
     * - una volta lanciato un thread, gestire gli errori
     */

    // lancio thread
    pthread_t threads[THREAD_COUNT];
    int i;
    for(i = 0; i < THREAD_COUNT; i++) {
        thread_args_t* arg = calloc(1, sizeof(thread_args_t));
        arg->idx = i;
        if(i < THREAD_COUNT / 2) {
            arg->role=CONS_ROLE;
        }else{
            arg->role=PROD_ROLE;
        }
        ret = pthread_create(&threads[i], NULL, thread_routine, arg);
        if(ret) handle_error_en(ret,"error on creating thread");
    }
    /***/


    /**
     * COMPLETARE QUI
     *
     * Obiettivi: attendere il termine dei thread
     * - attendere il termine di ognuno dei THREAD_COUNT thread lanciati
     *   in precedenza
     * - gestire gli errori
     */
     for(i = 0 ; i < THREAD_COUNT; i++) {
        ret = pthread_join(threads[i], NULL);
        if(ret) handle_error_en(ret,"error joining thread");
     }
    /***/

    /**
     * COMPLETARE QUI
     *
     * Obiettivi: rilasciare i semafori inizializzati in precedenza
     * - per ogni semaforo rilasciato, gestire gli errori
     */

     ret = sem_destroy(&cons_sem);
     if(ret) handle_error("error closing consumer sem");
     
    ret = sem_destroy(&prod_sem);
     if(ret) handle_error("error closing consumer sem");

     ret = sem_destroy(&filled_sem);
     if(ret) handle_error("error closing filled sem");

     ret = sem_destroy(&empty_sem);
     if(ret) handle_error("error closing empty sem");
    /***/

    printf("[Main Thread] operazioni completate\n");

    return EXIT_SUCCESS;
}
