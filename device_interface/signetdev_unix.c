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

int signetdev_priv_send_message_async(void *user, int token, int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp)
{
	static u8 s_async_resp[CMD_PACKET_PAYLOAD_SIZE];
	static int s_async_resp_code;

	struct send_message_req *r = (struct send_message_req *)malloc(sizeof(struct send_message_req));
	r->dev_cmd = dev_cmd;
	r->api_cmd = api_cmd;
	if (payload) {
		r->payload = malloc(payload_size);
		memcpy(r->payload, payload, payload_size);
	} else {
		r->payload = NULL;
	}
	r->payload_size = payload_size;
	r->user = user;
	r->token = token;
	if (get_resp) {
		r->resp = s_async_resp;
		r->resp_code = &s_async_resp_code;
	} else {
		r->resp = NULL;
		r->resp_code = NULL;
	}
	r->interrupt = 0;
	issue_command_no_resp(SIGNETDEV_CMD_MESSAGE, r);
	return 0;
}

int signetdev_priv_cancel_message_async(int dev_cmd, const u8 *payload, unsigned int payload_size)
{
	struct send_message_req *r = malloc(sizeof(struct send_message_req));
	r->dev_cmd = dev_cmd;
	if (payload) {
		r->payload = malloc(payload_size);
		memcpy(r->payload, payload, payload_size);
	} else {
		r->payload = NULL;
	}
	r->payload_size = payload_size;
	r->resp = NULL;
	r->resp_code = NULL;
	r->interrupt = 1;
	issue_command_no_resp(SIGNETDEV_CMD_CANCEL_MESSAGE, r);
	return 0;
}

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

void signetdev_priv_message_send_resp(struct send_message_req *msg, int rc, int expected_messages_remaining)
{
	if (!msg->interrupt) {
		int resp_code = OKAY;
		int resp_len = 0;
		if (msg->resp_code)
			resp_code = *msg->resp_code;
		if (rc >= 0)
			resp_len = rc;
		else
			resp_code = rc;
		signetdev_priv_handle_command_resp(msg->user,
			       msg->token,
			       msg->dev_cmd,
			       msg->api_cmd,
			       resp_code,
			       msg->resp,
			       resp_len,
			       expected_messages_remaining);
	}
}

void signetdev_priv_free_message(struct send_message_req **req)
{
	if ((*req)->payload)
		free((*req)->payload);
	free(*req);
	*req = NULL;
}

void signetdev_priv_finalize_message(struct send_message_req **msg ,int rc)
{
	signetdev_priv_message_send_resp(*msg, rc, 0);
	signetdev_priv_free_message(msg);
}
