#include "signetdev_unix.h"
#include "signetdev_priv.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

pthread_t worker_thread;
int g_command_pipe[2];
int g_command_resp_pipe[2];

int signetdev_open_connection()
{
	return issue_command(SIGNETDEV_CMD_OPEN, NULL);
}

void signetdev_close_connection()
{
	issue_command_no_resp(SIGNETDEV_CMD_CLOSE, NULL);
}

void signetdev_cancel_close_connection()
{
	issue_command_no_resp(SIGNETDEV_CMD_CANCEL_OPEN, NULL);
}

void signetdev_priv_platform_init()
{
	pipe(g_command_pipe);
	pipe(g_command_resp_pipe);
	fcntl(g_command_pipe[0], F_SETFL, O_NONBLOCK);
	pthread_create(&worker_thread, NULL, transaction_thread, NULL);
}

void signetdev_priv_platform_deinit()
{
	void *ret = NULL;
	issue_command_no_resp(SIGNETDEV_CMD_QUIT, NULL);
	pthread_join(worker_thread, &ret);
}
