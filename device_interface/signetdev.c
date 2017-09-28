#include "signetdev.h"
#include "signetdev_priv.h"
#include "common.h"
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>

void (*g_device_opened_cb)(void *) = NULL;
void *g_device_opened_cb_param = NULL;

void (*g_device_closed_cb)(void *) = NULL;
void *g_device_closed_cb_param = NULL;

signetdev_cmd_resp_t g_command_resp_cb = NULL;
static void *g_command_resp_cb_param = NULL;

signetdev_device_event_t g_device_event_cb = NULL;
static void *g_device_event_cb_param = NULL;

signetdev_conn_err_t g_error_handler = NULL;
void *g_error_handler_param = NULL;

void signetdev_set_device_opened_cb(void (*device_opened)(void *), void *param)
{
	g_device_opened_cb = device_opened;
	g_device_opened_cb_param = param;
}

void signetdev_set_device_closed_cb(void (*device_closed)(void *), void *param)
{
	g_device_closed_cb = device_closed;
	g_device_closed_cb_param = param;
}

void signetdev_set_command_resp_cb(signetdev_cmd_resp_t cb, void *cb_param)
{
	g_command_resp_cb = cb;
	g_command_resp_cb_param = cb_param;
}

void signetdev_set_device_event_cb(signetdev_device_event_t cb, void *cb_param)
{
	g_device_event_cb = cb;
	g_device_event_cb_param = cb_param;
}

void signetdev_set_error_handler(signetdev_conn_err_t handler, void *param)
{
	g_error_handler = handler;
	g_error_handler_param = param;
}

static uint8_t shift_usages[232][2] = {
	[4] = {'a','A'},
	[5] = {'b','B'},
	[6] = {'c','C'},
	[7] = {'d','D'},
	[8] = {'e','E'},
	[9] = {'f','F'},
	[10] = {'g','G'},
	[11] = {'h','H'},
	[12] = {'i','I'},
	[13] = {'j','J'},
	[14] = {'k','K'},
	[15] = {'l','L'},
	[16] = {'m','M'},
	[17] = {'n','N'},
	[18] = {'o','O'},
	[19] = {'p','P'},
	[20] = {'q','Q'},
	[21] = {'r','R'},
	[22] = {'s','S'},
	[23] = {'t','T'},
	[24] = {'u','U'},
	[25] = {'v','V'},
	[26] = {'w','W'},
	[27] = {'x','X'},
	[28] = {'y','Y'},
	[29] = {'z','Z'},
	[30] = {'1','!'},
	[31] = {'2','@'},
	[32] = {'3','#'},
	[33] = {'4','$'},
	[34] = {'5','%'},
	[35] = {'6','^'},
	[36] = {'7','&'},
	[37] = {'8','*'},
	[38] = {'9','('},
	[39] = {'0',')'},
	[40] = {' ',' '},
	[46] = {'=','+'},
	[47] = {'[','{'},
	[48] = {']','}'},
	[49] = {'\\','|'},
	[51] = {';',':'},
	[52] = {'\'','"'},
	[53] = {'`','~'},
	[54] = {',','<'},
	[55] = {'.','>'},
	[56] = {'/','?'}
};

static uint8_t single_usages[232] = {
	[40] = '\n',
	[43] = '\t',
	[44] = ' ',
};

static uint8_t usage_inv[256][2];

void signetdev_initialize_api()
{
	int i = 0;
	for (i = 0; i < 232; i++) {
		if (!shift_usages[i][0])
			continue;
		usage_inv[shift_usages[i][1]][0] = 1;
		usage_inv[shift_usages[i][1]][1] = i;
		usage_inv[shift_usages[i][0]][0] = 0;
		usage_inv[shift_usages[i][0]][1] = i;
	}
	for (i = 0; i < 232; i++) {
		if (!single_usages[i])
			continue;
		usage_inv[single_usages[i]][0] = 0;
		usage_inv[single_usages[i]][1] = i;
	}
	signetdev_priv_platform_init();
}

void signetdev_deinitialize_api()
{
	signetdev_priv_platform_deinit();
}

int signetdev_cancel_button_wait()
{
    signetdev_priv_cancel_message_async(CANCEL_BUTTON_PRESS, NULL, 0);
    return OKAY;
}

static int get_cmd_token()
{
	static int token_ctr = 0;
	return token_ctr++;
}

static int execute_command_async(void *user, int token, int dev_cmd, int api_cmd)
{
	return signetdev_priv_send_message_async(user, token, dev_cmd,
			api_cmd, NULL, 0 /* payload size*/, SIGNETDEV_PRIV_GET_RESP);
}

static int execute_command_no_resp_async(void *user, int token,  int dev_cmd, int api_cmd)
{
	return signetdev_priv_send_message_async(user, token, dev_cmd,
			api_cmd, NULL, 0, SIGNETDEV_PRIV_NO_RESP);
}

int signetdev_logout_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token, LOGOUT, SIGNETDEV_CMD_LOGOUT);
}

int signetdev_wipe_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		WIPE, SIGNETDEV_CMD_WIPE);
}

int signetdev_button_wait_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		BUTTON_WAIT, SIGNETDEV_CMD_BUTTON_WAIT);
}

int signetdev_disconnect_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_no_resp_async(user, *token,
		DISCONNECT, SIGNETDEV_CMD_DISCONNECT);
}

int signetdev_connect_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		CONNECT, SIGNETDEV_CMD_CONNECT);
}

int signetdev_login_async(void *user, int *token, u8 *key, unsigned int key_len)
{
	*token = get_cmd_token();
	uint8_t msg[AES_BLK_SIZE];
	memset(msg, 0, sizeof(msg));
	memcpy(msg, key, key_len > sizeof(msg) ? sizeof(msg) : key_len);
	return signetdev_priv_send_message_async(user, *token,
		LOGIN, SIGNETDEV_CMD_LOGIN,
		msg, sizeof(msg), SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_get_progress_async(void *user, int *token, int progress, int state)
{
	uint8_t msg[4];
	*token = get_cmd_token();
	msg[0] = progress & 0xff;
	msg[1] = progress >> 8;
	msg[2] = state & 0xff;
	msg[3] = state >> 8;
	return signetdev_priv_send_message_async(user, *token,
			GET_PROGRESS, SIGNETDEV_CMD_GET_PROGRESS,
			msg, sizeof(msg), SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_begin_device_backup_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		BACKUP_DEVICE, SIGNETDEV_CMD_BEGIN_DEVICE_BACKUP);
}

int signetdev_end_device_backup_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		BACKUP_DEVICE_DONE, SIGNETDEV_CMD_END_DEVICE_BACKUP);
}

int signetdev_begin_device_restore_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		RESTORE_DEVICE, SIGNETDEV_CMD_BEGIN_DEVICE_RESTORE);
}

int signetdev_end_device_restore_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		RESTORE_DEVICE_DONE, SIGNETDEV_CMD_END_DEVICE_RESTORE);
}

int signetdev_begin_update_firmware_async(void *user, int *token)
{
	*token = get_cmd_token();
	return execute_command_async(user, *token,
		UPDATE_FIRMWARE, SIGNETDEV_CMD_BEGIN_UPDATE_FIRMWARE);
}

int signetdev_reset_device_async(void *user, int *token)
{
	*token = get_cmd_token();
	int rc = execute_command_no_resp_async(user, *token,
		RESET_DEVICE, SIGNETDEV_CMD_RESET_DEVICE);
	return rc;
}

int signetdev_read_id_async(void *user, int *token, int id)
{
	*token = get_cmd_token();
	uint8_t msg[] = {id};
	return signetdev_priv_send_message_async(user, *token,
		GET_DATA, SIGNETDEV_CMD_READ_ID,
		msg, sizeof(msg), SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_write_id_async(void *user, int *token, int id, int size, const u8 *data, const u8 *mask)
{
	uint8_t msg[CMD_PACKET_PAYLOAD_SIZE];
	*token = get_cmd_token();
	int k = 0;
	msg[k] = id; k++;
	msg[k] = size & 0xff; k++;
	msg[k] = size >> 8; k++;
	int i;
	int blk_count = SIZE_TO_SUB_BLK_COUNT(size);
	unsigned int message_size = k + (blk_count * SUB_BLK_SIZE);
	if (message_size >= sizeof(msg))
		return SIGNET_ERROR_OVERFLOW;

	for (i = 0; i < size; i++) {
		int r = i % SUB_BLK_DATA_SIZE;
		int blk = i / SUB_BLK_DATA_SIZE;
		int idx = blk * SUB_BLK_SIZE + r + SUB_BLK_MASK_SIZE;
		int bit = (mask[i/8] >> (i % 8)) & 0x1;
		int m_idx = blk * SUB_BLK_SIZE + (r/8);
		msg[k + idx] = data[i];
		msg[k + m_idx] = (msg[k + m_idx] & ~(1<<(r%8))) | (bit << (r%8));
	}
	k += blk_count * SUB_BLK_SIZE;
	return signetdev_priv_send_message_async(user, *token,
		SET_DATA, SIGNETDEV_CMD_WRITE_ID,
		msg, message_size, SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_startup_async(void *param, int *token)
{
	*token = get_cmd_token();
	return signetdev_priv_send_message_async(param, *token,
			STARTUP, SIGNETDEV_CMD_STARTUP,
			NULL, 0,
			SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_type_async(void *param, int *token, const u8 *keys, int n_keys)
{
	*token = get_cmd_token();
	u8 msg[CMD_PACKET_PAYLOAD_SIZE];
	unsigned int message_size = n_keys * 4;
	if (message_size >= sizeof(msg))
		return SIGNET_ERROR_OVERFLOW;
	int i;
	for (i = 0; i < n_keys; i++) {
		int j = 4 * i;
		u8 c = keys[i];
		msg[j + 0] = usage_inv[c][0] << 1;
		msg[j + 1] = usage_inv[c][1];
		msg[j + 2] = 0;
		msg[j + 3] = 0;
	}
	return signetdev_priv_send_message_async(param, *token,
			TYPE, SIGNETDEV_CMD_TYPE,
			msg, message_size,
			SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_open_id_async(void *param, int *token, int id)
{
	*token = get_cmd_token();
	u8 msg[] = {id};
	return signetdev_priv_send_message_async(param, *token,
			OPEN_ID, SIGNETDEV_CMD_OPEN_ID,
			msg, sizeof(msg),
			SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_delete_id_async(void *param, int *token, int id)
{
	*token = get_cmd_token();
	u8 msg[] = {id};
	return signetdev_priv_send_message_async(param, *token,
			DELETE_ID, SIGNETDEV_CMD_DELETE_ID,
			msg, sizeof(msg),
			SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_close_id_async(void *param, int *token, int id)
{
	*token = get_cmd_token();
	u8 msg[] = {id};
	return signetdev_priv_send_message_async(param, *token,
			CLOSE_ID, SIGNETDEV_CMD_CLOSE_ID,
			msg, sizeof(msg),
			SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_begin_initialize_device_async(void *param, int *token,
					const u8 *key, int key_len,
					const u8 *hashfn, int hashfn_len,
					const u8 *salt, int salt_len,
					const u8 *rand_data, int rand_data_len)
{
	*token = get_cmd_token();
	u8 msg[CMD_PACKET_PAYLOAD_SIZE];
	memset(msg, 0, INITIALIZE_CMD_SIZE); //Hash function

	memcpy(msg, key, key_len > AES_BLK_SIZE ? AES_BLK_SIZE : key_len);
	memcpy(msg + AES_BLK_SIZE, hashfn, hashfn_len > AES_BLK_SIZE ? AES_BLK_SIZE : hashfn_len);
	memcpy(msg + AES_BLK_SIZE*2, salt, salt_len > AES_BLK_SIZE ? AES_BLK_SIZE : salt_len);
	memcpy(msg + AES_BLK_SIZE*3, rand_data, rand_data_len > INIT_RAND_DATA_SZ ? INIT_RAND_DATA_SZ : rand_data_len);

	return signetdev_priv_send_message_async(param, *token,
			INITIALIZE, SIGNETDEV_CMD_BEGIN_INITIALIZE_DEVICE,
			msg, INITIALIZE_CMD_SIZE, SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_read_block_async(void *param, int *token, int idx)
{
	*token = get_cmd_token();
	u8 msg[] = {idx};
	return signetdev_priv_send_message_async(param, *token,
			READ_BLOCK, SIGNETDEV_CMD_READ_BLOCK,
			msg, sizeof(msg), SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_write_block_async(void *param, int *token, int idx, const void *buffer)
{
	*token = get_cmd_token();
	u8 msg[BLK_SIZE + 1] = {idx};
	memcpy(msg + 1, buffer, BLK_SIZE);
	return signetdev_priv_send_message_async(param, *token,
				WRITE_BLOCK, SIGNETDEV_CMD_WRITE_BLOCK,
				msg, sizeof(msg), SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_write_flash_async(void *param, int *token, u32 addr, const void *data, int data_len)
{
	*token = get_cmd_token();
	uint8_t msg[CMD_PACKET_PAYLOAD_SIZE];
	unsigned int message_size = 4 + data_len;
	if (message_size >= sizeof(msg))
		return SIGNET_ERROR_OVERFLOW;
	msg[0] = (addr >> 0) & 0xff;
	msg[1] = (addr >> 8) & 0xff;
	msg[2] = (addr >> 16) & 0xff;
	msg[3] = (addr >> 24) & 0xff;
	memcpy(msg + 4, data, data_len);
	return signetdev_priv_send_message_async(param, *token,
				WRITE_FLASH, SIGNETDEV_CMD_WRITE_FLASH,
				msg, 4 + data_len, SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_change_master_password_async(void *param, int *token,
		u8 *old_key, u32 old_key_len,
		u8 *new_key, u32 new_key_len,
		u8 *hashfn, u32 hashfn_len,
		u8 *salt, u32 salt_len)
{
	*token = get_cmd_token();
	uint8_t msg[AES_BLK_SIZE * 4];
	memset(msg, 0, AES_BLK_SIZE * 2);
	memcpy(msg, old_key, old_key_len > AES_BLK_SIZE ? AES_BLK_SIZE : old_key_len);
	memcpy(msg + AES_BLK_SIZE, new_key, new_key_len > AES_BLK_SIZE ? AES_BLK_SIZE : new_key_len);
	memcpy(msg + AES_BLK_SIZE * 2, hashfn, hashfn_len > AES_BLK_SIZE ? AES_BLK_SIZE : hashfn_len);
	memcpy(msg + AES_BLK_SIZE * 3, salt, salt_len > AES_BLK_SIZE ? AES_BLK_SIZE : salt_len);
	return signetdev_priv_send_message_async(param, *token, CHANGE_MASTER_PASSWORD,
			SIGNETDEV_CMD_CHANGE_MASTER_PASSWORD, msg, sizeof(msg),
			SIGNETDEV_PRIV_GET_RESP);
}

int signetdev_erase_pages_async(void *param, int *token, int n_pages, const u8 *page_numbers)
{
	*token = get_cmd_token();
	return signetdev_priv_send_message_async(param, *token,
			ERASE_FLASH_PAGES, SIGNETDEV_CMD_ERASE_PAGES,
			page_numbers, n_pages,
			SIGNETDEV_PRIV_GET_RESP);
}

void signetdev_priv_handle_device_event(int event_type, const u8 *resp, int resp_len)
{
	if (g_device_event_cb) {
		g_device_event_cb(g_device_event_cb_param, event_type, (void *)resp, resp_len);
	}
}

int signetdev_priv_prepare_message(u8 *msg, int dev_cmd, u8 *payload, int payload_size)
{
	int cmd_size = payload_size + CMD_PACKET_HEADER_SIZE;
	msg[0] = cmd_size & 0xff;
	msg[1] = cmd_size >> 8;
	msg[2] = dev_cmd;
	if (payload)
		memcpy(msg + CMD_PACKET_HEADER_SIZE, payload, payload_size);
	return cmd_size;
}

int signetdev_priv_message_packet_count(int msg_sz)
{
	return (msg_sz + RAW_HID_PAYLOAD_SIZE - 1)/ RAW_HID_PAYLOAD_SIZE;
}

void signetdev_priv_handle_command_resp(void *user, int token, int dev_cmd, int api_cmd, int resp_code, const u8 *resp, int resp_len, int expected_messages_remaining)
{
	(void)(expected_messages_remaining);
	switch (dev_cmd)
	{
	case READ_BLOCK: {
		if (resp_len != BLK_SIZE) {
			signetdev_priv_handle_error();
			break;
		} else if (g_command_resp_cb) {
			g_command_resp_cb(g_command_resp_cb_param,
				user, token, api_cmd,
				resp_code, (void *)resp);
		}
	} break;
	case GET_PROGRESS: {
		struct signetdev_get_progress_resp_data cb_resp;
		if (resp_code == OKAY && resp_len >= 4 && (resp_len % 4) == 0) {
			cb_resp.total_progress = resp[0] + (resp[1] << 8);
			cb_resp.total_progress_maximum = resp[2] + (resp[3] << 8);
			int k = (resp_len/4) - 1;
			int i;
			cb_resp.n_components = k;
			for (i = 0; i < k; i++) {
				int j = i + 1;
				cb_resp.progress[i] = resp[j*4] + (resp[j*4 + 1] << 8);
				cb_resp.progress_maximum[i] = resp[j*4 + 2] + (resp[j*4 + 3] << 8);
			}
		} else if (resp_code == INVALID_STATE) {
			cb_resp.n_components = 0;
			cb_resp.total_progress = 0;
			cb_resp.total_progress_maximum = 0;
		} else {
			signetdev_priv_handle_error();
			break;
		}
		if (g_command_resp_cb)
			g_command_resp_cb(g_command_resp_cb_param,
				user, token, api_cmd,
				resp_code, &cb_resp);

		} break;
	case STARTUP: {
		struct signetdev_startup_resp_data cb_resp;
		cb_resp.device_state = resp[0];
		memcpy(cb_resp.hashfn, resp + 1, AES_BLK_SIZE);
		memcpy(cb_resp.salt, resp + 1 + AES_BLK_SIZE, AES_BLK_SIZE);
		if (g_command_resp_cb)
			g_command_resp_cb(g_command_resp_cb_param,
				user, token, api_cmd,
				resp_code, &cb_resp);
		} break;
	case GET_DATA: {
		struct signetdev_read_id_resp_data cb_resp;
		u8 *data = cb_resp.data;
		u8 *mask = cb_resp.mask;
		cb_resp.size = 0;
		const u8 *ptr = resp;
		if (resp_code == OKAY) {
			if (resp_len < 2) {
				signetdev_priv_handle_error();
				break;
			}
			cb_resp.size = resp[0] + (resp[1] << 8);
			resp_len -= 2;
			ptr += 2;
			int i;
			unsigned int blk_count = SIZE_TO_SUB_BLK_COUNT(resp_len);
			if ((SUB_BLK_SIZE * blk_count) >= sizeof(cb_resp.data)) {
				signetdev_priv_handle_error();
				break;
			}
			for (i = 0; i < cb_resp.size; i++) {
				int blk = i / SUB_BLK_DATA_SIZE;
				int blk_field = (i % SUB_BLK_DATA_SIZE);
				int bit = (ptr[(blk * SUB_BLK_SIZE) + (blk_field/8)] >> (blk_field%8)) & 1;
				data[i] = ptr[(blk * SUB_BLK_SIZE) + blk_field + SUB_BLK_MASK_SIZE];
				mask[i/8] = (mask[i/8] & ~(1<<(i%8))) | (bit << (i%8));
			}
		}
		if (g_command_resp_cb)
			g_command_resp_cb(g_command_resp_cb_param,
				user, token, api_cmd,
				resp_code, &cb_resp);

		} break;
	default:
		if (g_command_resp_cb)
			g_command_resp_cb(g_command_resp_cb_param,
					  user, token, api_cmd,
					  resp_code, NULL);
		break;
	}
}
