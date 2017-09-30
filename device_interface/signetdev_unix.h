#ifndef SIGNETDEV_UNIX_H
#define SIGNETDEV_UNIX_H

#include "common.h"

#include <pthread.h>

//Platform specific
void *transaction_thread(void *arg);

extern pthread_t worker_thread;
extern int g_command_pipe[];
extern int g_command_resp_pipe[];

#endif // SIGNETDEV_UNIX_H
