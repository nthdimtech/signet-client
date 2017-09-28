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


void signetdev_priv_process_rx_packet(struct rx_message_state *state, u8 *rx_packet_buf)
{
	int seq = rx_packet_buf[0] & 0x7f;
	int last = rx_packet_buf[0] >> 7;
	const u8 *rx_packet_header = rx_packet_buf + RAW_HID_HEADER_SIZE;
	if (seq == 0x7f) {
		int event_type = rx_packet_header[0];
		int resp_len =  rx_packet_header[1];
		void *data = (void *)(rx_packet_header + 2);
		signetdev_priv_handle_device_event(event_type, data, resp_len);
	} else if (state->message) {
		if (seq == 0) {
			state->expected_resp_size = rx_packet_header[0] + (rx_packet_header[1] << 8) - CMD_PACKET_HEADER_SIZE;
			state->expected_messages_remaining = rx_packet_header[3] + (rx_packet_header[4] << 8);
			if (state->message->resp_code) {
				*state->message->resp_code = rx_packet_header[2];
			}
			memcpy(state->message->resp,
				rx_packet_buf + RAW_HID_HEADER_SIZE + CMD_PACKET_HEADER_SIZE,
				RAW_HID_PAYLOAD_SIZE - CMD_PACKET_HEADER_SIZE);
		} else {
			int to_read = RAW_HID_PAYLOAD_SIZE;
			int offset = (RAW_HID_PAYLOAD_SIZE * seq) - CMD_PACKET_HEADER_SIZE;
			if ((offset + to_read) > state->expected_resp_size) {
				to_read = (state->expected_resp_size - offset);
			}
			memcpy(state->message->resp + offset, rx_packet_header, to_read);
		}
		if (last) {
			if (state->expected_messages_remaining == 0) {
				signetdev_priv_finalize_message(&state->message, state->expected_resp_size);
			} else {
				signetdev_priv_message_send_resp(state->message, state->expected_resp_size, state->expected_messages_remaining);
			}
		}
	}
}
