#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// macros for handling errors
#define handle_error_en(en, msg)    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)

// parametri di configurazione
#define SEMAPHORE_NAME  "/sc2_sem"
#define CHILDREN_COUNT  3
#define MSG_COUNT 4
#define MSG_SIZE (64 * PIPE_BUF)

#endif
