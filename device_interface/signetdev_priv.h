#ifndef SIGNETDEV_PRIV_H
#include "common.h"
void signetdev_priv_platform_init();
void signetdev_priv_platform_deinit();
int signetdev_priv_send_message_async(void *user, int token,  int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp);
void signetdev_priv_handle_error();
void signetdev_priv_handle_command_resp(void *user, int token, int dev_cmd, int api_cmd, int resp_code, const u8 *resp, int resp_len, int expected_messages_remaining);
int signetdev_priv_cancel_message_async(int dev_cmd, const u8 *payload, unsigned int payload_size);
void signetdev_priv_handle_device_event(int event_type, const u8 *resp, int resp_len);

struct tx_message_state {
	u8 msg_buf[CMD_PACKET_BUF_SIZE];
	u8 packet_buf[RAW_HID_PACKET_SIZE + 1];
	int msg_size;
	int msg_packet_seq;
	int msg_packet_count;
};

void signetdev_priv_prepare_message_state(struct tx_message_state *msg, int dev_cmd, u8 *payload, int payload_size);
void signetdev_priv_advance_message_state(struct tx_message_state *msg);

extern void (*g_device_opened_cb)(void *);
extern void *g_device_opened_cb_param;

extern void (*g_device_closed_cb)(void *);
extern void *g_device_closed_cb_param;

#define SIGNETDEV_PRIV_GET_RESP 1
#define SIGNETDEV_PRIV_NO_RESP 0

#endif
