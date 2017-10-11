#ifndef SIGNETDEV_H
#define SIGNETDEV_H

#include "common.h"
#include <stdint.h>

void signetdev_initialize_api();

void signetdev_deinitialize_api();
int signetdev_open_connection();
void signetdev_close_connection();
typedef void (*signetdev_conn_err_t)(void *);

void signetdev_set_error_handler(signetdev_conn_err_t handler, void *param);

typedef enum signetdev_cmd_id {
	SIGNETDEV_CMD_STARTUP,
	SIGNETDEV_CMD_LOGOUT,
	SIGNETDEV_CMD_LOGIN,
	SIGNETDEV_CMD_WIPE,
	SIGNETDEV_CMD_BUTTON_WAIT,
	SIGNETDEV_CMD_DISCONNECT,
	SIGNETDEV_CMD_CONNECT,
	SIGNETDEV_CMD_GET_PROGRESS,
	SIGNETDEV_CMD_BEGIN_DEVICE_BACKUP,
	SIGNETDEV_CMD_END_DEVICE_BACKUP,
	SIGNETDEV_CMD_BEGIN_DEVICE_RESTORE,
	SIGNETDEV_CMD_END_DEVICE_RESTORE,
	SIGNETDEV_CMD_BEGIN_UPDATE_FIRMWARE,
	SIGNETDEV_CMD_RESET_DEVICE,
	SIGNETDEV_CMD_OPEN_ID,
	SIGNETDEV_CMD_READ_ID,
	SIGNETDEV_CMD_WRITE_ID,
	SIGNETDEV_CMD_DELETE_ID,
	SIGNETDEV_CMD_CLOSE_ID,
	SIGNETDEV_CMD_TYPE,
	SIGNETDEV_CMD_CHANGE_MASTER_PASSWORD,
	SIGNETDEV_CMD_BEGIN_INITIALIZE_DEVICE,
	SIGNETDEV_CMD_READ_BLOCK,
	SIGNETDEV_CMD_WRITE_BLOCK,
	SIGNETDEV_CMD_WRITE_FLASH,
	SIGNETDEV_CMD_ERASE_PAGES,
	SIGNETDEV_CMD_READ_ALL_ID,
	SIGNETDEV_NUM_COMMANDS
} signetdev_cmd_id_t;

int signetdev_logout_async(void *user, int *token);
int signetdev_login_async(void *user, int *token, u8 *key, unsigned int key_len);
int signetdev_begin_update_firmware_async(void *user, int *token);
int signetdev_reset_device_async(void *user, int *token);
int signetdev_get_progress_async(void *user, int *token, int progress, int state);
int signetdev_wipe_async(void *user, int *token);
int signetdev_begin_device_backup_async(void *user, int *token);
int signetdev_end_device_backup_async(void *user, int *token);
int signetdev_begin_device_restore_async(void *user, int *token);
int signetdev_end_device_restore_async(void *user, int *token);
int signetdev_startup_async(void *param, int *token);
int signetdev_type_async(void *param, int *token, const u8 *keys, int n_keys);
int signetdev_open_id_async(void *param, int *token, int id);
int signetdev_delete_id_async(void *param, int *token, int id);
int signetdev_close_id_async(void *param, int *token, int id);
int signetdev_button_wait_async(void *user, int *token);
int signetdev_read_id_async(void *user, int *token, int id);
int signetdev_write_id_async(void *user, int *token, int id, int size, const u8 *data, const u8 *mask);
int signetdev_change_master_password_async(void *param, int *token,
						u8 *old_key, u32 old_key_len,
						u8 *new_key, u32 new_key_len,
						u8 *hashfn, u32 hashfn_len,
						u8 *salt, u32 salt_len);
int signetdev_begin_initialize_device_async(void *param, int *token,
					const u8 *key, int key_len,
					const u8 *hashfn, int hashfn_len,
					const u8 *salt, int salt_len,
					const u8 *rand_data, int rand_data_len);
int signetdev_connect_async(void *user, int *token);
int signetdev_disconnect_async(void *user, int *token);
int signetdev_read_block_async(void *param, int *token, int idx);
int signetdev_write_block_async(void *param, int *token, int idx, const void *buffer);
int signetdev_write_flash_async(void *param, int *token, u32 addr, const void *data, int data_len);
int signetdev_erase_pages_async(void *param, int *token, int n_pages, const u8 *page_numbers);
int signetdev_read_all_id_async(void *user, int *token, int unmask);

struct signetdev_read_all_id_resp_data {
	int id;
	int size;
	u8 data[CMD_PACKET_PAYLOAD_SIZE];
	u8 mask[CMD_PACKET_PAYLOAD_SIZE];
};

struct signetdev_read_id_resp_data {
	int size;
	u8 data[CMD_PACKET_PAYLOAD_SIZE];
	u8 mask[CMD_PACKET_PAYLOAD_SIZE];
};

struct signetdev_startup_resp_data {
	int device_state;
	int root_block_format;
	u8 hashfn[AES_BLK_SIZE];
	u8 salt[AES_BLK_SIZE];
};

struct signetdev_get_progress_resp_data {
	int n_components;
	int total_progress;
	int total_progress_maximum;
	int progress[8];
	int progress_maximum[8];
};

int signetdev_cancel_button_wait();


#ifdef _WIN32
#include <windows.h>
void signetdev_win32_set_window_handle(HANDLE recp);
int signetdev_filter_window_messasage(UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

typedef void (*signetdev_cmd_resp_t)(void *cb_param, void *cmd_user_param, int cmd_token, int messages_remaining, int cmd, int resp_code, void *resp_data);
typedef void (*signetdev_device_event_t)(void *cb_param, int event_type, void *resp_data, int resp_len);


void signetdev_set_device_opened_cb(void (*device_opened)(void *), void *param);
void signetdev_set_device_closed_cb(void (*device_closed)(void *), void *param);
void signetdev_set_command_resp_cb(signetdev_cmd_resp_t cmd_resp_cb, void *cb_param);
void signetdev_set_device_event_cb(signetdev_device_event_t device_event_cb, void *cb_param);

#endif


