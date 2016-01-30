#include "signetdev_priv.h"
#include "signetdev.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

static pthread_t g_worker_thread;
static int g_command_pipe[2];
static int g_command_resp_pipe[2];

extern signetdev_conn_err_t g_error_handler;
extern void *g_error_handler_param;
static int g_opening_connection = 0;
static IOHIDManagerRef hid_manager=NULL;
static IOHIDDeviceRef hid_dev=NULL;

static struct send_message_req *g_tail_message = NULL;
static struct send_message_req *g_head_message = NULL;

static struct send_message_req *g_tail_cancel_message = NULL;
static struct send_message_req *g_head_cancel_message = NULL;

static struct send_message_req *g_current_write_message = NULL;
static struct send_message_req *g_current_read_message = NULL;

u8 g_async_resp[CMD_PACKET_PAYLOAD_SIZE];
int g_async_resp_code;

enum signetdev_commands {
    SIGNETDEV_CMD_OPEN,
    SIGNETDEV_CMD_CANCEL_OPEN,
    SIGNETDEV_CMD_CLOSE,
    SIGNETDEV_CMD_QUIT,
    SIGNETDEV_CMD_MESSAGE,
    SIGNETDEV_CMD_CANCEL_MESSAGE
};

void message_response(struct send_message_req *msg, int rc);

static void handle_error()
{
    if (g_error_handler) {
        g_error_handler(g_error_handler_param);
    }
}

static int issue_command(int command, void *p)
{
    intptr_t v[2] = {command, (intptr_t)p};
    write(g_command_pipe[1], v, sizeof(intptr_t) * 2);
    char cmd_resp;
    read(g_command_resp_pipe[0], &cmd_resp, 1);
    return cmd_resp;
}

static void issue_command_no_resp(int command, void *p)
{
    intptr_t v[2] = {command, (intptr_t)p};
    write(g_command_pipe[1], v, sizeof(intptr_t) * 2);
}

void signetdev_priv_handle_error()
{
    handle_error();
}

struct send_message_req {
	int dev_cmd;
	int api_cmd;
	u8 *payload;
	unsigned int payload_size;
	u8 *resp;
	int *resp_code;
	int async;
	void *user;
	int token;
	int interrupt;
	struct send_message_req *next;
};

void free_message(struct send_message_req **req);

int signetdev_priv_send_message_async(void *user, int token, int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp)
{
	struct send_message_req *r = malloc(sizeof(struct send_message_req));
	r->dev_cmd = dev_cmd;
	r->api_cmd = api_cmd;
	if (payload) {
		r->payload = malloc(payload_size);
		memcpy(r->payload, payload, payload_size);
	} else {
		r->payload = NULL;
	}
	r->payload_size = payload_size;
	r->async = 1;
	r->interrupt = 0;
	r->user = user;
	r->token = token;
	if (get_resp) {
		r->resp = g_async_resp;
		r->resp_code = &g_async_resp_code;
	} else {
		r->resp = NULL;
		r->resp_code = NULL;
	}
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
	r->interrupt = 1;
	r->resp = NULL;
	r->resp_code = NULL;
	r->payload_size = payload_size;
	issue_command_no_resp(SIGNETDEV_CMD_CANCEL_MESSAGE, r);
	return 0;
}

void signetdev_priv_platform_deinit()
{
    void *ret;
    issue_command_no_resp(SIGNETDEV_CMD_QUIT, NULL);
    pthread_join(g_worker_thread, &ret);
}

static void command_response(int rc)
{
    char resp = rc;
    write(g_command_resp_pipe[1], &resp, 1);
}

static int send_hid_command(int cmd, u8 *payload, int payload_size)
{
    u8 packet[RAW_HID_PACKET_SIZE];
    u8 msg[CMD_PACKET_BUF_SIZE];
    int cmd_size = payload_size + CMD_PACKET_HEADER_SIZE;
    msg[0] = cmd_size & 0xff;
    msg[1] = cmd_size >> 8;
    msg[2] = cmd;
    if (payload)
        memcpy(msg + CMD_PACKET_HEADER_SIZE, payload, payload_size);
    int i;
    int count = (cmd_size + RAW_HID_PAYLOAD_SIZE - 1)/ RAW_HID_PAYLOAD_SIZE;
    for (i = 0; i < count; i++) {
        packet[0] = i;
        if ((i + 1) == count)
            packet[0] |= 0x80;
        int offset = RAW_HID_PAYLOAD_SIZE * i;
        int to_copy = RAW_HID_PAYLOAD_SIZE;
        if ((offset + to_copy) > cmd_size) {
            to_copy = cmd_size - offset;
        }
        memcpy(packet + RAW_HID_HEADER_SIZE,
               msg + offset,
               to_copy);
        IOHIDDeviceSetReport(hid_dev, kIOHIDReportTypeOutput, 0, packet, RAW_HID_PACKET_SIZE);
        //TODO: validate return
    }
    return 0;
}

static void handle_command(int command, void *p)
{
    switch (command) {
    case SIGNETDEV_CMD_OPEN:
        if (hid_dev != NULL) {
            command_response(0);
        } else {
            g_opening_connection = 1;
            command_response(-1);
        }
        break;
    case SIGNETDEV_CMD_CANCEL_OPEN:
        g_opening_connection = 0;
        break;
    case SIGNETDEV_CMD_CLOSE:
        if (hid_dev != NULL) {
            IOHIDDeviceClose(hid_dev, kIOHIDOptionsTypeNone);
            hid_dev = NULL;
        }
        break;
	case SIGNETDEV_CMD_QUIT:
		pthread_exit(NULL);
		break;
	case SIGNETDEV_CMD_MESSAGE: {
		struct send_message_req *msg = (struct send_message_req *)p;
		msg->next = NULL;
		if (!g_head_message) {
			g_head_message = msg;
		}
		if (g_tail_message)
			g_tail_message->next = msg;
		g_tail_message = msg;
		CFRunLoopStop(CFRunLoopGetCurrent());
		} break;
	case SIGNETDEV_CMD_CANCEL_MESSAGE: {
		struct send_message_req *msg = (struct send_message_req *)p;
		msg->next = NULL;
		if (!g_head_cancel_message) {
		    g_head_cancel_message = msg;
		}
		if (g_tail_cancel_message)
		    g_tail_cancel_message->next = msg;
		g_tail_cancel_message = msg;
		CFRunLoopStop(CFRunLoopGetCurrent());
		} break;
	}
}

void command_pipe_callback(CFFileDescriptorRef f, CFOptionFlags callBackTypes, void *info)
{
    (void)callBackTypes;
    (void)info;
    intptr_t v[2];
    int rc = read(g_command_pipe[0], v, sizeof(intptr_t) * 2);
    if (rc == sizeof(intptr_t) * 2) {
        handle_command(v[0], (void *)v[1]);
        CFFileDescriptorEnableCallBacks(f, kCFFileDescriptorReadCallBack);
    } else {
        handle_error();
    }
}

static void handle_exit(void *arg)
{
    (void)arg;
}

static void detach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
        (void)context;
        (void)r;
        (void)hid_mgr;
        if (dev == hid_dev && hid_dev != NULL) {
            hid_dev = NULL;
        }
	if (g_current_read_message) {
		message_response(g_current_read_message, SIGNET_ERROR_DISCONNECT);
		free_message(&g_current_read_message);
		g_current_read_message = NULL;
	}
        if (g_device_closed_cb) {
            g_device_closed_cb(g_device_closed_cb_param);
        }
}

struct hid_packet {
    u8 data[RAW_HID_PACKET_SIZE];
    struct hid_packet *next;
};

struct hid_packet *hid_packet_first = NULL;
struct hid_packet *hid_packet_last = NULL;

void free_message(struct send_message_req **req)
{
	if ((*req)->payload)
		free((*req)->payload);
	free(*req);
	*req = NULL;
}

void message_response(struct send_message_req *msg, int rc)
{
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
                       resp_len);
}

static void input_callback(void *context, IOReturn ret, void *sender, IOHIDReportType type, uint32_t id, uint8_t *data, CFIndex len)
{
    (void)type;
    (void)id;
    (void)sender;
    (void)ret;
    (void)context;
    static u8 recv_packet[RAW_HID_PACKET_SIZE];
    static int recv_byte = 0;
    int i = 0;
    while (i <= len) {
        int rem = RAW_HID_PACKET_SIZE - recv_byte;
        int to_copy;
        if (len <= rem) {
            to_copy = len;
        } else {
            to_copy = rem;
        }
        memcpy(recv_packet + recv_byte, data + i, to_copy);
        recv_byte += to_copy;
        i += to_copy;
        len -= to_copy;
        if (recv_byte == RAW_HID_PACKET_SIZE) {
            recv_byte = 0;
            struct hid_packet *p = (struct hid_packet *)malloc(sizeof(struct hid_packet));
            memcpy(p->data, recv_packet, RAW_HID_PACKET_SIZE);
            p->next = NULL;
            if (hid_packet_first == NULL) {
                hid_packet_first = p;
                hid_packet_last = p;
            } else {
                hid_packet_last->next = p;
                hid_packet_last = p;
            }
        }
    }
    CFRunLoopStop(CFRunLoopGetCurrent());
}

static u8 g_input_buffer[1024];

static void attach_callback(void *context, IOReturn r, void *hid_mgr, IOHIDDeviceRef dev)
{
        (void)context;
        (void)r;
        (void)hid_mgr;
        if (IOHIDDeviceOpen(dev, kIOHIDOptionsTypeNone) != kIOReturnSuccess) return;

        IOHIDDeviceScheduleWithRunLoop(dev, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOHIDDeviceRegisterInputReportCallback(dev, g_input_buffer, sizeof(g_input_buffer),
                input_callback, NULL);
        hid_dev = dev;
        if (g_opening_connection) {
            g_opening_connection = 0;
            if (g_device_opened_cb) {
                g_device_opened_cb(g_device_opened_cb_param);
            }
        }
}

static int process_hid_input()
{
	static int s_recv_packet_count;

	if (hid_packet_first == NULL)
		return 1;
	struct hid_packet *cur = hid_packet_first;
	struct hid_packet *next = hid_packet_first->next;
	int seq = cur->data[0] & 0x7f;
	int last = cur->data[0] >> 7;
	if (seq == 0x7f) {
		int event_type = cur->data[RAW_HID_HEADER_SIZE + 1];
		int resp_len =  cur->data[RAW_HID_HEADER_SIZE + 2];
		void *data = (void *)(cur->data + RAW_HID_HEADER_SIZE + 3);
		signetdev_priv_handle_device_event(event_type, data, resp_len);
	} else if (g_current_read_message) {
		if (seq == 0) {
			s_recv_packet_count = cur->data[RAW_HID_HEADER_SIZE] + (cur->data[RAW_HID_HEADER_SIZE + 1] << 8) - CMD_PACKET_HEADER_SIZE;
			if (g_current_read_message->resp_code) {
			    *g_current_read_message->resp_code = cur->data[RAW_HID_HEADER_SIZE + 2];
			}
			memcpy(g_current_read_message->resp,
			    cur->data + RAW_HID_HEADER_SIZE + CMD_PACKET_HEADER_SIZE,
			    RAW_HID_PAYLOAD_SIZE - CMD_PACKET_HEADER_SIZE);
		} else {
			int to_read = RAW_HID_PAYLOAD_SIZE;
			int offset = RAW_HID_PAYLOAD_SIZE * seq - CMD_PACKET_HEADER_SIZE;
			if ((offset + RAW_HID_PAYLOAD_SIZE) > s_recv_packet_count) {
			    to_read = (s_recv_packet_count - offset);
			}
			memcpy(g_current_read_message->resp + offset,
			    cur->data + RAW_HID_HEADER_SIZE,
			    to_read);
		}
		if (last) {
			message_response(g_current_read_message, s_recv_packet_count);
			free_message(&g_current_read_message);
		}
	} else {
		//TODO: respond to this. input packets with no command
		return 0;
	}
	free(hid_packet_first);
	hid_packet_first = next;
	return 0;
}

void *transaction_thread(void *arg)
{
    (void)arg;
    CFNumberRef num;
    IOReturn ret;
    CFMutableDictionaryRef dict;

    hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    int vid = USB_VENDOR_ID;
    num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &vid);
    CFDictionarySetValue(dict, CFSTR(kIOHIDVendorIDKey), num);
    CFRelease(num);

    int pid = USB_PRODUCT_ID;
    num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pid);
    CFDictionarySetValue(dict, CFSTR(kIOHIDProductIDKey), num);
    CFRelease(num);

    int usage_page = USB_RAW_HID_USAGE_PAGE;
    num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage_page);
    CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsagePageKey), num);
    CFRelease(num);

    int usage = USB_RAW_HID_USAGE;
    num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    CFDictionarySetValue(dict, CFSTR(kIOHIDPrimaryUsageKey), num);
    CFRelease(num);

    IOHIDManagerSetDeviceMatching(hid_manager, dict);
    CFRelease(dict);

    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOHIDManagerRegisterDeviceMatchingCallback(hid_manager, attach_callback, NULL);
    IOHIDManagerRegisterDeviceRemovalCallback(hid_manager, detach_callback, NULL);
    ret = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);

    CFFileDescriptorRef fdref = CFFileDescriptorCreate(kCFAllocatorDefault, g_command_pipe[0], 1, command_pipe_callback, NULL);
    CFFileDescriptorEnableCallBacks(fdref, kCFFileDescriptorReadCallBack);
    CFRunLoopSourceRef source = CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);
    CFRelease(source);

    pthread_cleanup_push(handle_exit, NULL);

	while (1) {
		CFRunLoopRun();
		int done = 0;
		if (!g_current_write_message && (g_head_message || g_head_cancel_message)) {
			if (g_head_cancel_message) {
				g_current_write_message = g_head_cancel_message;
				g_head_cancel_message = g_head_cancel_message->next;
				if (!g_head_cancel_message) {
					g_tail_cancel_message = NULL;
				}
			} else if (!g_current_read_message) {
				g_current_write_message = g_head_message;
				if (g_head_message->resp || g_head_message->resp_code)
					g_current_read_message = g_head_message;
				g_head_message = g_head_message->next;
				if (!g_head_message) {
					g_tail_message = NULL;
				}
			}
			if (g_current_write_message) {
				send_hid_command(g_current_write_message->dev_cmd,
						 g_current_write_message->payload,
						 g_current_write_message->payload_size);
				if (!g_current_write_message->resp && !g_current_write_message->resp_code) {
					message_response(g_current_write_message, 0);
				}
				g_current_write_message = NULL;
			}
		}
		while (!done) {
			done = process_hid_input();
		}
	}
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
    return NULL;
}

void signetdev_priv_platform_init()
{
	pipe(g_command_pipe);
	pipe(g_command_resp_pipe);
	fcntl(g_command_pipe[0], F_SETFL, O_NONBLOCK);
    pthread_create(&g_worker_thread, NULL, transaction_thread, NULL);
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
