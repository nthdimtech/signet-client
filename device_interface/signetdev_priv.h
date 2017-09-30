#ifndef SIGNETDEV_PRIV_H
#include "common.h"
void signetdev_priv_platform_init();
void signetdev_priv_platform_deinit();
void signetdev_priv_handle_error();
void signetdev_priv_handle_command_resp(void *user, int token, int dev_cmd, int api_cmd, int resp_code, const u8 *resp, int resp_len, int expected_messages_remaining);
void signetdev_priv_handle_device_event(int event_type, const u8 *resp, int resp_len);

enum signetdev_commands {
    SIGNETDEV_CMD_OPEN,
    SIGNETDEV_CMD_CANCEL_OPEN,
    SIGNETDEV_CMD_CLOSE,
    SIGNETDEV_CMD_QUIT,
    SIGNETDEV_CMD_MESSAGE,
    SIGNETDEV_CMD_CANCEL_MESSAGE
};

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

struct tx_message_state {
	u8 msg_buf[CMD_PACKET_BUF_SIZE];
	u8 packet_buf[RAW_HID_PACKET_SIZE + 1];
	int msg_size;
	int msg_packet_seq;
	int msg_packet_count;
	struct send_message_req *message;
};

struct rx_message_state {
	int expected_resp_size;
	int expected_messages_remaining;
	int resp_code;
	int resp_buffer[CMD_PACKET_BUF_SIZE];
	struct send_message_req *message;
};

void signetdev_priv_prepare_message_state(struct tx_message_state *msg, int dev_cmd, u8 *payload, int payload_size);
void signetdev_priv_advance_message_state(struct tx_message_state *msg);

extern void (*g_device_opened_cb)(void *);
extern void *g_device_opened_cb_param;

extern void (*g_device_closed_cb)(void *);
extern void *g_device_closed_cb_param;

#define SIGNETDEV_PRIV_GET_RESP 1
#define SIGNETDEV_PRIV_NO_RESP 0

int signetdev_priv_send_message_async(void *user, int token,  int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp);
void signetdev_priv_message_send_resp(struct send_message_req *msg, int rc, int expected_messages_remaining);
void signetdev_priv_free_message(struct send_message_req **req);
void signetdev_priv_finalize_message(struct send_message_req **msg ,int rc);
void signetdev_priv_process_rx_packet(struct rx_message_state *state, u8 *rx_packet_buf);
int signetdev_priv_cancel_message_async(int dev_cmd, const u8 *payload, unsigned int payload_size);

void issue_command_no_resp(int command, void *p);
int issue_command(int command, void *p);

#endif
