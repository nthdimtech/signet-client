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

static struct rx_message_state g_rx_message_state;

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
	struct tx_message_state msg;
	signetdev_priv_prepare_message(&msg, cmd, payload, payload_size);
	for (int i = 0; i < msg.msg_packet_count; i++) {
		signetdev_priv_advance_message_state(msg);
		IOHIDDeviceSetReport(hid_dev, kIOHIDReportTypeOutput, 0, msg.packet_buf + 1, RAW_HID_PACKET_SIZE);
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
	if (g_rx_message_state.message) {
		signetdev_priv_finalize_message(&g_rx_message_state.message, SIGNET_ERROR_DISCONNECT);
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
g_rx_message_state
static int process_hid_input()
{
	if (hid_packet_first == NULL)
		return 1;
	struct hid_packet *cur = hid_packet_first;
	struct hid_packet *next = hid_packet_first->next;
	signetdev_priv_process_rx_packet(&g_rx_message_state, cur->data);
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
			} else if (!g_rx_message_state.message) {
				g_current_write_message = g_head_message;
				if (g_head_message->resp || g_head_message->resp_code)
					g_rx_message_state.message = g_head_message;
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
