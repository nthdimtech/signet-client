#include "signetdev_priv.h"
#include "signetdev_unix.h"
#include "signetdev.h"

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDLib.h>

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

struct signetdev_connection {
	struct send_message_req *tail_message;
	struct send_message_req *head_message;

	struct send_message_req *tail_cancel_message;
	struct send_message_req *head_cancel_message;

	struct send_message_req *current_write_message;
	struct send_message_req *current_read_message;
};

struct signetdev_connection g_connection;

static void handle_error()
{
    if (g_error_handler) {
	g_error_handler(g_error_handler_param);
    }
}

int issue_command(int command, void *p)
{
    intptr_t v[2] = {command, (intptr_t)p};
    write(g_command_pipe[1], v, sizeof(intptr_t) * 2);
    char cmd_resp;
    read(g_command_resp_pipe[0], &cmd_resp, 1);
    return cmd_resp;
}

void issue_command_no_resp(int command, void *p)
{
    intptr_t v[2] = {command, (intptr_t)p};
    write(g_command_pipe[1], v, sizeof(intptr_t) * 2);
}

void signetdev_priv_handle_error()
{
    handle_error();
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
	int cmd_sz = signetdev_priv_prepare_message(msg, cmd, payload, payload_size);
	int i;
	int count = signetdev_priv_message_packet_count(cmd_sz);
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
		signetdev_priv_finalize_message(&g_current_read_message, SIGNET_ERROR_DISCONNECT);
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
	static int s_expected_messages_remaining;

	if (hid_packet_first == NULL)
		return 1;
	struct hid_packet *cur = hid_packet_first;
	struct hid_packet *next = hid_packet_first->next;
	int seq = cur->data[0] & 0x7f;
	int last = cur->data[0] >> 7;
	if (seq == 0x7f) {
		int event_type = cur->data[RAW_HID_HEADER_SIZE];
		int resp_len =  cur->data[RAW_HID_HEADER_SIZE + 1];
		void *data = (void *)(cur->data + RAW_HID_HEADER_SIZE + 2);
		signetdev_priv_handle_device_event(event_type, data, resp_len);
	} else if (g_current_read_message) {
		if (seq == 0) {
			s_recv_packet_count = cur->data[RAW_HID_HEADER_SIZE] + (cur->data[RAW_HID_HEADER_SIZE + 1] << 8) - CMD_PACKET_HEADER_SIZE;
			s_expected_messages_remaining = cur->data[RAW_HID_HEADER_SIZE + 3] + (cur->data[RAW_HID_HEADER_SIZE + 4] << 8);
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
			if (s_expected_messages_remaining == 0) {
				signetdev_priv_finalize_message(&g_current_read_message, s_recv_packet_count);
			} else {
				signetdev_priv_message_send_resp(g_current_read_message, s_recv_packet_count, s_expected_messages_remaining);
			}
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
					signetdev_priv_message_send_resp(g_current_write_message, 0);
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
