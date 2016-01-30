#include "../device_interface/rawhid/hid.h"
#include "common.h"
#include <windows.h>
#include <dbt.h>
#include <Usbiodef.h>
#include <hidsdi.h>
#include <hidclass.h>

#include "signetdev.h"
#include "signetdev_priv.h"

static int g_open_request_pending = 0;
static int g_read_requested = 0;

static HWND g_window = INVALID_HANDLE_VALUE;
static HANDLE g_device_handle = INVALID_HANDLE_VALUE;

struct send_message_req {
	int dev_cmd;
	int api_cmd;
	uint8_t *payload;
	unsigned int payload_size;
	int get_resp;
	void *user;
	int token;
} g_send_message_req;

struct cancel_message_req {
	int dev_cmd;
	uint8_t *payload;
	unsigned int payload_size;
} g_cancel_message_req;

int g_cancel_requested = 0;
int g_cancel_posted = 0;

static HANDLE g_msg_req_event = INVALID_HANDLE_VALUE;
static HANDLE g_msg_cancel_event = INVALID_HANDLE_VALUE;
static HANDLE g_msg_quit_event = INVALID_HANDLE_VALUE;
static HANDLE g_msg_thread = INVALID_HANDLE_VALUE;
static HANDLE g_msg_read_event = INVALID_HANDLE_VALUE;
static HANDLE g_msg_write_event = INVALID_HANDLE_VALUE;
static OVERLAPPED g_read_overlapped;
static OVERLAPPED g_write_overlapped;

extern signetdev_conn_err_t g_error_handler;
extern void *g_error_handler_param;

static int g_tx_packet_count = -1;
static int g_tx_packet_index = 0;
static u8 g_tx_msg[CMD_PACKET_BUF_SIZE];
static u8 g_tx_packet[RAW_HID_PACKET_SIZE + 1];
static u8 g_rx_packet[RAW_HID_PACKET_SIZE + 1];

static u8 g_resp[CMD_PACKET_PAYLOAD_SIZE];
static int g_rx_packet_index = 0;
static int g_resp_code;
static int g_resp_len;
static int g_resp_len_max;
static int g_get_resp;
static int g_token;
static void *g_user;

void signetdev_priv_handle_error()
{
	rawhid_close(0);
	g_device_handle = INVALID_HANDLE_VALUE;
	if (g_error_handler) {
		g_error_handler(g_error_handler_param);
	}
}

static int send_next_packet()
{
	int seq_no = g_tx_packet_index;
	int last = (g_tx_packet_index + 1) == g_tx_packet_count;
	int pidx = 0;
	g_tx_packet[pidx] = 0; pidx++;
	g_tx_packet[pidx] = seq_no;
	if (last)
		g_tx_packet[pidx] |= 0x80;
	pidx++;
	memcpy(g_tx_packet + pidx, g_tx_msg + RAW_HID_PAYLOAD_SIZE * seq_no, RAW_HID_PAYLOAD_SIZE);
	pidx += RAW_HID_PAYLOAD_SIZE;
	memset(&g_write_overlapped, 0, sizeof(OVERLAPPED));
	g_write_overlapped.hEvent = g_msg_write_event;
	if (!WriteFile(g_device_handle, g_tx_packet, pidx, NULL, &g_write_overlapped)) {
		if (GetLastError() != ERROR_IO_PENDING)	{
			return -1;
		}
	}
	return 0;
}

static int request_next_packet()
{
	memset(&g_read_overlapped, 0, sizeof(OVERLAPPED));
	g_read_overlapped.hEvent = g_msg_read_event;
	if (!ReadFile(g_device_handle, g_rx_packet, RAW_HID_PACKET_SIZE + 1, NULL, &g_read_overlapped)) {
		if (GetLastError() != ERROR_IO_PENDING)	{
			return -1;
		}
	}
	g_read_requested = 1;
	return 0;
}

static void process_send_message_req(struct send_message_req *req)
{
	unsigned int cmd_size = req->payload_size + CMD_PACKET_HEADER_SIZE;
	g_tx_msg[0] = cmd_size & 0xff;
	g_tx_msg[1] = cmd_size >> 8;
	g_tx_msg[2] = req->dev_cmd;
	if (req->payload)
		memcpy(g_tx_msg + CMD_PACKET_HEADER_SIZE, req->payload, req->payload_size);
	g_tx_packet_count = (cmd_size + RAW_HID_PAYLOAD_SIZE - 1) / RAW_HID_PAYLOAD_SIZE;
	g_tx_packet_index = 0;
	g_resp_len_max = CMD_PACKET_PAYLOAD_SIZE;
	g_get_resp = req->get_resp;
	g_token = req->token;
	g_user = req->user;
	if (send_next_packet()) {
		signetdev_priv_handle_error();
	}
	if (!g_read_requested) {
		if (request_next_packet()) {
			signetdev_priv_handle_error();
		}
	}
}

static void process_cancel_message_req(struct cancel_message_req *req)
{
	unsigned int cmd_size = req->payload_size + CMD_PACKET_HEADER_SIZE;
	g_tx_msg[0] = cmd_size & 0xff;
	g_tx_msg[1] = cmd_size >> 8;
	g_tx_msg[2] = req->dev_cmd;
	if (req->payload)
		memcpy(g_tx_msg + CMD_PACKET_HEADER_SIZE, req->payload, req->payload_size);
	g_tx_packet_count = (cmd_size + RAW_HID_PAYLOAD_SIZE - 1) / RAW_HID_PAYLOAD_SIZE;
	g_tx_packet_index = 0;
	g_resp_len_max = CMD_PACKET_PAYLOAD_SIZE;
	g_get_resp = 0;
	g_cancel_posted = 1;
	if (send_next_packet()) {
		signetdev_priv_handle_error();
	}
	if (!g_read_requested) {
		if (request_next_packet()) {
			signetdev_priv_handle_error();
		}
	}
}

static DWORD WINAPI message_thread(LPVOID lpParameter)
{
	(void)lpParameter;
	HANDLE wait_handles[8];
	wait_handles[0] = g_msg_req_event;
	wait_handles[1] = g_msg_quit_event;
	wait_handles[2] = g_msg_read_event;
	wait_handles[3] = g_msg_write_event;
	wait_handles[4] = g_msg_cancel_event;
	while (1) {
		DWORD rc = WaitForMultipleObjects(5, wait_handles, FALSE, INFINITE);
		int index = rc - WAIT_OBJECT_0;
		ResetEvent(wait_handles[index]);
		switch(index) {
		case 0: {
			process_send_message_req(&g_send_message_req);
		} break;
		case 1:
			ExitThread(0);
		case 2: {
			DWORD n;
			if (!GetOverlappedResult(g_device_handle, &g_read_overlapped, &n, FALSE)) {
				signetdev_priv_handle_error();
				break;
			}
			if (n != (RAW_HID_PACKET_SIZE + 1)) {
				signetdev_priv_handle_error();
				break;
			}
			int seq = g_rx_packet[1] & 0x7f;
			int last = g_rx_packet[1] >> 7;
			int src_offset;
			int dst_offset;
			int to_copy;

			if (seq == 0x7f) {
				int event_type = g_rx_packet[RAW_HID_HEADER_SIZE + 2];
				int resp_len =  g_rx_packet[RAW_HID_HEADER_SIZE + 3];
				void *data = (void *)(g_rx_packet + RAW_HID_HEADER_SIZE + 4);
				signetdev_priv_handle_device_event(event_type, data, resp_len);
				if (request_next_packet()) {
					signetdev_priv_handle_error();
					break;
				}
				break;
			}  else if (seq != g_rx_packet_index) {
				signetdev_priv_handle_error();
				break;
			} else if (seq == 0) {
				g_resp_len = g_rx_packet[1 + RAW_HID_HEADER_SIZE] + (((int)g_rx_packet[1 +RAW_HID_HEADER_SIZE + 1]) << 8);
				g_resp_len -= CMD_PACKET_HEADER_SIZE;
				g_resp_code = g_rx_packet[1 + RAW_HID_HEADER_SIZE + 2];

				src_offset = 1 + RAW_HID_HEADER_SIZE + CMD_PACKET_HEADER_SIZE;
				dst_offset = 0;
				to_copy = RAW_HID_PAYLOAD_SIZE - CMD_PACKET_HEADER_SIZE;
			} else {
				src_offset = 1 + RAW_HID_HEADER_SIZE;
				dst_offset = RAW_HID_PAYLOAD_SIZE * seq - CMD_PACKET_HEADER_SIZE;
				to_copy = RAW_HID_PAYLOAD_SIZE;
			}
			if ((dst_offset + to_copy) > g_resp_len) {
				to_copy = g_resp_len - dst_offset;
			}
			if ((dst_offset + to_copy) > g_resp_len_max) {
				to_copy = g_resp_len_max - dst_offset;
			}
			if (to_copy < 0) {
				signetdev_priv_handle_error();
				break;
			} else {
				memcpy(g_resp + dst_offset, g_rx_packet + src_offset, to_copy);
			}
			//TODO: validate that we got enough data
			if (last) {
				signetdev_priv_handle_command_resp(g_user, g_token,
					g_send_message_req.dev_cmd, g_send_message_req.api_cmd,
					g_resp_code, g_resp, g_resp_len);
				if (g_cancel_requested) {
					g_cancel_requested = 0;
					process_cancel_message_req(&g_cancel_message_req);
				}
			}
			if (request_next_packet()) {
				signetdev_priv_handle_error();
			}
			g_rx_packet_index++;
		} break;
		case 3:
			g_tx_packet_index++;
			if (g_tx_packet_index >= g_tx_packet_count) {
				g_tx_packet_count = -1;
				//Move to reading phase
				if (g_get_resp) {
					g_rx_packet_index = 0;
				} else {
					if (!g_cancel_posted) {
						signetdev_priv_handle_command_resp(g_user, g_token,
							g_send_message_req.dev_cmd, g_send_message_req.api_cmd,
							g_resp_code, NULL, 0);
					} else {
						g_cancel_posted = 0;
					}
					if (g_cancel_requested) {
						g_cancel_requested = 0;
						process_cancel_message_req(&g_cancel_message_req);
					}
				}
			} else {
				DWORD n;
				if (!GetOverlappedResult(g_device_handle, &g_write_overlapped, &n, FALSE)) {
					signetdev_priv_handle_error();
					break;
				}
				if (n != (RAW_HID_PACKET_SIZE + 1)) {
					signetdev_priv_handle_error();
					break;
				}
				if (send_next_packet()) {
					signetdev_priv_handle_error();
				}
				if (!g_read_requested) {
					if (request_next_packet()) {
						signetdev_priv_handle_error();
					}
				}
			}
			break;
		case 4:
			if (g_tx_packet_count < 0) {
				process_cancel_message_req(&g_cancel_message_req);
			} else {
				g_cancel_requested = 1;
			}
			break;
		}
	}
}

void signetdev_priv_platform_init()
{
	g_msg_req_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_msg_quit_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_msg_read_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_msg_write_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_msg_cancel_event =  CreateEvent(NULL, TRUE, FALSE, NULL);
	g_msg_thread = CreateThread(NULL, 4096*16, message_thread, NULL, 0, NULL);
}

void signetdev_priv_platform_deinit()
{
	SetEvent(g_msg_quit_event);
	WaitForSingleObject(g_msg_thread, INFINITE);
	CloseHandle(g_msg_thread);
	g_msg_thread = INVALID_HANDLE_VALUE;
	CloseHandle(g_msg_req_event);
	g_msg_req_event = INVALID_HANDLE_VALUE;
	CloseHandle(g_msg_quit_event);
	g_msg_quit_event = INVALID_HANDLE_VALUE;
	CloseHandle(g_msg_read_event);
	g_msg_read_event = INVALID_HANDLE_VALUE;
	CloseHandle(g_msg_write_event);
	g_msg_write_event = INVALID_HANDLE_VALUE;
	CloseHandle(g_msg_cancel_event);
	g_msg_cancel_event = INVALID_HANDLE_VALUE;
}

int signetdev_priv_send_message_async(void *user, int token, int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp)
{
	struct send_message_req *r = &g_send_message_req;
	r->dev_cmd = dev_cmd;
	if (payload) {
		r->payload = malloc(payload_size);
		memcpy(r->payload, payload, payload_size);
	} else {
		r->payload = NULL;
	}
	r->payload_size = payload_size;
	r->get_resp = get_resp;
	r->user = user;
	r->token = token;
	r->api_cmd = api_cmd;
	SetEvent(g_msg_req_event);
	return 0;
}

int signetdev_priv_cancel_message_async(int dev_cmd, const u8 *payload, unsigned int payload_size)
{
	struct cancel_message_req *r = &g_cancel_message_req;
	r->dev_cmd = dev_cmd;
	if (payload) {
		r->payload = malloc(payload_size);
		memcpy(r->payload, payload, payload_size);
	} else {
		r->payload = NULL;
	}
	r->payload_size = payload_size;
	SetEvent(g_msg_cancel_event);
	return 0;
}

int signetdev_open_connection()
{
	int ct = rawhid_open(1, USB_VENDOR_ID, USB_PRODUCT_ID, USB_RAW_HID_USAGE_PAGE, USB_RAW_HID_USAGE);
	if (ct != 1) {
		g_open_request_pending = 1;
		return -1;
	}
	g_read_requested = 0;
	g_device_handle = rawhid_win32_get_handle(0);
	g_open_request_pending = 0;

	DEV_BROADCAST_HANDLE dbh;
	dbh.dbch_size = sizeof(dbh);
	dbh.dbch_handle = g_device_handle;
	dbh.dbch_devicetype = DBT_DEVTYP_HANDLE;
	dbh.dbch_hdevnotify = NULL;
	RegisterDeviceNotification((HANDLE)g_window, &dbh ,DEVICE_NOTIFY_WINDOW_HANDLE);
	return 0;
}

void signetdev_win32_set_window_handle(HANDLE recp)
{
	GUID guid;
	HidD_GetHidGuid(&guid);
	DEV_BROADCAST_DEVICEINTERFACE bdi;
	bdi.dbcc_size = sizeof(&bdi);
	bdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	bdi.dbcc_classguid = guid;
	bdi.dbcc_reserved = 0;
	bdi.dbcc_name[0] = 0;
	g_window = (HWND)recp;
	RegisterDeviceNotification(recp, &bdi, DEVICE_NOTIFY_WINDOW_HANDLE);
}

int signetdev_filter_window_messasage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PDEV_BROADCAST_HDR	pDBHdr;
	PDEV_BROADCAST_HANDLE pDBHandle;
	int rc;
	switch (uMsg) {
	case WM_DEVICECHANGE:
		switch (wParam) {
		case DBT_DEVTYP_HANDLE:
			break;
		case DBT_DEVNODES_CHANGED:
			if (g_open_request_pending) {
				rc = signetdev_open_connection();
				if (!rc) {
					g_open_request_pending = 0;
				}
				if (!rc && g_device_opened_cb) {
					g_device_opened_cb(g_device_opened_cb_param);
				}
			}
			break;
		case DBT_DEVICEREMOVECOMPLETE:
			pDBHdr = (PDEV_BROADCAST_HDR) lParam;
			switch (pDBHdr->dbch_devicetype)
			{
				case DBT_DEVTYP_HANDLE:
					pDBHandle = (PDEV_BROADCAST_HANDLE) pDBHdr;
					UnregisterDeviceNotification(pDBHandle->dbch_hdevnotify);
					rawhid_close(0);
					g_device_handle = INVALID_HANDLE_VALUE;
					if (g_device_closed_cb)
						g_device_closed_cb(g_device_closed_cb_param);
					break;
				default:
					break;

			}
			break;
		}
		return 1;
		break;
	default:
		return 0;
	}
}

void signetdev_close_connection()
{
	rawhid_close(0);
	g_device_handle = INVALID_HANDLE_VALUE;
}

int signetdev_priv_conn_read(void *data, int count)
{
	DWORD bytesWritten = rawhid_recv(0, data, count, -1);
	return bytesWritten;
}

int signetdev_priv_conn_write(void *data, int count)
{
	DWORD bytesWritten = rawhid_send(0, data, count, -1);
	return bytesWritten;
}
