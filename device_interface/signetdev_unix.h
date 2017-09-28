#ifndef SIGNETDEV_UNIX_H
#define SIGNETDEV_UNIX_H

#include "common.h"

#include <pthread.h>

struct send_message_req {
	int dev_cmd;
	int api_cmd;
	u8 *payload;
	unsigned int payload_size;
	u8 *resp;
	int *resp_code;
	void *user;
	int token;
	int interrupt;
	struct send_message_req *next;
};

enum signetdev_commands {
    SIGNETDEV_CMD_OPEN,
    SIGNETDEV_CMD_CANCEL_OPEN,
    SIGNETDEV_CMD_CLOSE,
    SIGNETDEV_CMD_QUIT,
    SIGNETDEV_CMD_MESSAGE,
    SIGNETDEV_CMD_CANCEL_MESSAGE
};

void signetdev_priv_message_send_resp(struct send_message_req *msg, int rc, int expected_messages_remaining);
void signetdev_priv_free_message(struct send_message_req **req);
void signetdev_priv_finalize_message(struct send_message_req **msg ,int rc);

//Platform specific
void issue_command_no_resp(int command, void *p);
int issue_command(int command, void *p);
void *transaction_thread(void *arg);

extern pthread_t worker_thread;
extern int g_command_pipe[];
extern int g_command_resp_pipe[];

#endif // SIGNETDEV_UNIX_H
