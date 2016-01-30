#ifndef SIGNETDEV_PRIV_H
#include "common.h"
void signetdev_priv_platform_init();
void signetdev_priv_platform_deinit();
int signetdev_priv_send_message_async(void *user, int token,  int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp);
void signetdev_priv_handle_error();
void signetdev_priv_handle_command_resp(void *user, int token, int dev_cmd, int api_cmd, int resp_code, const u8 *resp, int resp_len);
int signetdev_priv_cancel_message_async(int dev_cmd, const u8 *payload, unsigned int payload_size);
void signetdev_priv_handle_device_event(int event_type, const u8 *resp, int resp_len);

extern void (*g_device_opened_cb)(void *);
extern void *g_device_opened_cb_param;

extern void (*g_device_closed_cb)(void *);
extern void *g_device_closed_cb_param;

#define SIGNETDEV_PRIV_GET_RESP 1
#define SIGNETDEV_PRIV_NO_RESP 0

#endif
