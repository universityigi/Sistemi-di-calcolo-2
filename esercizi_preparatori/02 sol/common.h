#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// macros for handling errors
#define handle_error_en(en, msg)    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)

// parametri di configurazione
#define FIRST_FIFO_NAME     "/tmp/ds_fifo_1"
#define SECOND_FIFO_NAME    "/tmp/ds_fifo_2"
#define SEED        0

// typedef e metodi per la struttura dati da trasferire
typedef struct myStruct {
    int*    vec;
    int     elems;
} myStruct;

myStruct*   initialize_struct();
void*       get_struct_for_fifo_write(myStruct* S);
size_t      get_struct_len_for_fifo_write(myStruct* S);
uint32_t    get_struct_checksum(myStruct* S);
myStruct*   reconstruct_from_fifo_read(void* data, size_t len);
void        free_struct(myStruct* S);

#endif
