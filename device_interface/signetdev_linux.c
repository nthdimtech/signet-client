#include "signetdev_priv.h"
#include "signetdev.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

extern signetdev_conn_err_t g_error_handler;
extern void *g_error_handler_param;

static pthread_t worker_thread;

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

static u8 g_async_resp[CMD_PACKET_PAYLOAD_SIZE];
static int g_async_resp_code;

enum signetdev_commands {
	SIGNETDEV_CMD_OPEN,
	SIGNETDEV_CMD_CANCEL_OPEN,
	SIGNETDEV_CMD_CLOSE,
	SIGNETDEV_CMD_MESSAGE,
	SIGNETDEV_CMD_QUIT,
	SIGNETDEV_CMD_CANCEL_MESSAGE
};

static int g_command_pipe[2];
static int g_command_resp_pipe[2];
static int g_opening_connection;
static int g_poll_fd = -1;
static int g_inotify_fd = -1;

struct signetdev_connection {
	int fd;
	struct send_message_req *tail_message;
	struct send_message_req *head_message;
	struct send_message_req *tail_cancel_message;
	struct send_message_req *head_cancel_message;
	struct send_message_req *current_write_message;
	struct send_message_req *current_read_message;

	u8 tx_msg[CMD_PACKET_BUF_SIZE];
	int tx_msg_size;
	int tx_msg_packet_seq;
	int tx_msg_packet_count;

	u8 tx_packet_buf[RAW_HID_PACKET_SIZE + 1];
	int tx_packet_buf_pos;

	u8 rx_packet_buf[RAW_HID_PACKET_SIZE + 1];
	int rx_packet_buf_pos;

	int resp_buffer[CMD_PACKET_BUF_SIZE];
	int resp_code;
	int expected_resp_size;
};

struct signetdev_connection g_connection;

static struct send_message_req **pending_message()
{
	struct signetdev_connection *conn = &g_connection;
	struct send_message_req **msg = NULL;
	if (conn->current_read_message) {
		msg = &conn->current_read_message;
	} else if (conn->current_write_message && !conn->current_write_message->interrupt) {
		msg = &conn->current_write_message;
	}
	return msg;
}

static void queue_hid_command(int dev_cmd, u8 *payload, int payload_size)
{
	struct signetdev_connection *conn = &g_connection;
	int cmd_size = payload_size + CMD_PACKET_HEADER_SIZE;
	conn->tx_msg[0] = cmd_size & 0xff;
	conn->tx_msg[1] = cmd_size >> 8;
	conn->tx_msg[2] = dev_cmd;
	if (payload)
		memcpy(conn->tx_msg + CMD_PACKET_HEADER_SIZE, payload, payload_size);

	conn->tx_msg_packet_seq = -1;
	conn->tx_packet_buf_pos = RAW_HID_PACKET_SIZE+1;
	conn->tx_msg_packet_count = (cmd_size + RAW_HID_PAYLOAD_SIZE - 1)/ RAW_HID_PAYLOAD_SIZE;
}

void signetdev_priv_handle_error()
{
	struct signetdev_connection *conn = &g_connection;
	if (conn->fd != -1) {
		close(conn->fd);
		conn->fd  = -1;
	}
	if (g_error_handler) {
		g_error_handler(g_error_handler_param);
	}
}

static void command_response(int rc)
{
	char resp = rc;
	write(g_command_resp_pipe[1], &resp, 1);
}

static void finalize_message(struct send_message_req *msg ,int rc)
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
						   resp_len);
	}
	free(msg->payload);
	free(msg);
}

static void handle_exit(void *arg)
{
	struct signetdev_connection *conn = &g_connection;
	(void)arg;
	struct send_message_req **msg = pending_message();
	if (msg) {
		finalize_message(*msg, SIGNET_ERROR_QUIT);
		*msg = NULL;
	}
	if (conn->fd != -1)
		close(conn->fd);
	if (g_poll_fd != -1)
		close(g_poll_fd);
	if (g_inotify_fd != -1)
		close(g_inotify_fd);
	g_poll_fd = -1;
	g_inotify_fd = -1;
}

static void handle_error()
{
	struct signetdev_connection *conn = &g_connection;
	if (conn->fd) {
		close(conn->fd);
		conn->fd = -1;
	}
	if (g_error_handler) {
		g_error_handler(g_error_handler_param);
	}
	struct send_message_req **msg = pending_message();
	if (msg) {
		finalize_message(*msg, SIGNET_ERROR_DISCONNECT);
		*msg = NULL;
	}

}

static int attempt_raw_hid_write()
{
	struct signetdev_connection *conn = &g_connection;
	if (conn->tx_packet_buf_pos == (RAW_HID_PACKET_SIZE+1)) {
		conn->tx_packet_buf_pos = 0;
		conn->tx_msg_packet_seq++;
		if (conn->tx_msg_packet_seq >= conn->tx_msg_packet_count) {
			if (!conn->current_write_message->resp) {
				finalize_message(conn->current_write_message, conn->tx_msg_size);
				conn->current_write_message = NULL;
			} else {
				//TODO: check that we aren't already reading a message
				conn->current_read_message = conn->current_write_message;
				conn->current_write_message = NULL;
				conn->rx_packet_buf_pos = 0;
				conn->expected_resp_size = 0;
			}
			return 0;
		}
		int pidx = 0;
		conn->tx_packet_buf[pidx++] = 0;
		conn->tx_packet_buf[pidx] = conn->tx_msg_packet_seq;
		if ((conn->tx_msg_packet_seq + 1) == conn->tx_msg_packet_count)
			conn->tx_packet_buf[pidx] |= 0x80;
		pidx++;
		memcpy(conn->tx_packet_buf + pidx,
			   conn->tx_msg + RAW_HID_PAYLOAD_SIZE * conn->tx_msg_packet_seq,
			   RAW_HID_PAYLOAD_SIZE);
		pidx += RAW_HID_PAYLOAD_SIZE;
	}
	int rc = write(conn->fd, conn->tx_packet_buf + conn->tx_packet_buf_pos, RAW_HID_PACKET_SIZE + 1 - conn->tx_packet_buf_pos);
	if (rc == -1 && errno == EAGAIN) {
		return 1;
	} else if (rc == -1) {
		handle_error();
	} else {
		conn->tx_packet_buf_pos += rc;
	}
	return 0;
}

static int attempt_raw_hid_read()
{
	struct signetdev_connection *conn = &g_connection;
	int rc = read(conn->fd, conn->rx_packet_buf + conn->rx_packet_buf_pos, RAW_HID_PACKET_SIZE - conn->rx_packet_buf_pos);
	if (rc == -1 && errno == EAGAIN) {
		return 1;
	} else if (rc == -1) {
		handle_error();
	} else {
		conn->rx_packet_buf_pos += rc;
		if (conn->rx_packet_buf_pos == RAW_HID_PACKET_SIZE) {
			conn->rx_packet_buf_pos = 0;
			int seq = conn->rx_packet_buf[0] & 0x7f;
			int last = conn->rx_packet_buf[0] >> 7;
			if (seq == 0x7f) {
				int event_type = conn->rx_packet_buf[RAW_HID_HEADER_SIZE + 1];
				int resp_len =  conn->rx_packet_buf[RAW_HID_HEADER_SIZE + 2];
				void *data = (void *)(conn->rx_packet_buf + RAW_HID_HEADER_SIZE + 3);
				signetdev_priv_handle_device_event(event_type, data, resp_len);
			} else if (conn->current_read_message) {
				if (seq == 0) {
					conn->expected_resp_size = conn->rx_packet_buf[RAW_HID_HEADER_SIZE] + (conn->rx_packet_buf[RAW_HID_HEADER_SIZE + 1] << 8) - CMD_PACKET_HEADER_SIZE;
					if (conn->current_read_message->resp_code) {
						*conn->current_read_message->resp_code = conn->rx_packet_buf[RAW_HID_HEADER_SIZE + 2];
					}
					memcpy(conn->current_read_message->resp,
						conn->rx_packet_buf + RAW_HID_HEADER_SIZE + CMD_PACKET_HEADER_SIZE,
						RAW_HID_PAYLOAD_SIZE - CMD_PACKET_HEADER_SIZE);
				} else {
					int to_read = RAW_HID_PAYLOAD_SIZE;
					int offset = (RAW_HID_PAYLOAD_SIZE * seq) - CMD_PACKET_HEADER_SIZE;
					if ((offset + to_read) > conn->expected_resp_size) {
						to_read = (conn->expected_resp_size - offset);
					}
					memcpy(conn->current_read_message->resp + offset, conn->rx_packet_buf + RAW_HID_HEADER_SIZE, to_read);
				}
				if (last) {
					finalize_message(conn->current_read_message, conn->expected_resp_size);
					conn->current_read_message = NULL;
					return 0;
				}
			}
		}
	}
	return 0;
}

static int attempt_open_connection()
{
	struct signetdev_connection *conn = &g_connection;
	if (conn->fd >= 0) {
		g_opening_connection = 0;
		return 0;
	}
	int fd = open("/dev/signet", O_RDWR | O_NONBLOCK);
	if (fd >= 0) {
		memset(conn, 0, sizeof(g_connection));
		conn->fd = fd;
		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
		ev.data.fd = conn->fd;
		int rc = epoll_ctl(g_poll_fd, EPOLL_CTL_ADD, conn->fd, &ev);
		if (rc)
			pthread_exit(NULL);
		g_opening_connection = 0;
		return 0;
	} else {
		g_opening_connection = 1;
		return -1;
	}
}

static void handle_command(int command, void *p)
{
	int rc;
	struct signetdev_connection *conn = &g_connection;
	switch (command) {
	case SIGNETDEV_CMD_OPEN:
		rc = attempt_open_connection();
		command_response(rc);
		break;
	case SIGNETDEV_CMD_CANCEL_OPEN:
		if (g_opening_connection) {
			g_opening_connection = 0;
			command_response(-1);
		}
		break;
	case SIGNETDEV_CMD_CLOSE:
		if (conn->fd >= 0) {
			epoll_ctl(g_poll_fd, EPOLL_CTL_DEL, conn->fd, NULL);
			close(conn->fd);
			conn->fd = -1;
		}
		break;
	case SIGNETDEV_CMD_MESSAGE: {
		struct signetdev_connection *conn = &g_connection;
		struct send_message_req *msg = (struct send_message_req *)p;
		msg->next = NULL;
		if (!conn->head_message) {
			conn->head_message = msg;
		}
		if (conn->tail_message)
			conn->tail_message->next = msg;
		conn->tail_message = msg;
	} break;
	case SIGNETDEV_CMD_QUIT: {
		pthread_exit(NULL);
	} break;
	case SIGNETDEV_CMD_CANCEL_MESSAGE: {
		struct send_message_req *msg = (struct send_message_req *)p;
		msg->next = NULL;
		if (!conn->head_cancel_message) {
			conn->head_cancel_message = msg;
		}
		if (conn->tail_cancel_message)
			conn->tail_cancel_message->next = msg;
		conn->tail_cancel_message = msg;
	} break;
	}
}

static void command_pipe_io_iter()
{
	int done = 0;
	while (!done) {
		intptr_t v[2];
		int rc = read(g_command_pipe[0], v, sizeof(intptr_t) * 2);
		if (rc == sizeof(intptr_t) * 2) {
			handle_command(v[0], (void *)v[1]);
		} else if (rc == -1 && errno == EAGAIN) {
			done = 1;
		} else {
			handle_error();
		}
	}
}

static void raw_hid_io_iter()
{
	struct signetdev_connection *conn = &g_connection;
	//Read and/or write non-blocking until EAGAIN is returned
	int done = 0;
	while (!done) {
		if (!conn->current_write_message && (conn->head_message || conn->head_cancel_message)) {
			if (conn->head_cancel_message) {
				conn->current_write_message = conn->head_cancel_message;
				conn->head_cancel_message = conn->head_cancel_message->next;
				if (!conn->head_cancel_message) {
					conn->tail_cancel_message = NULL;
				}
			} else if (!conn->current_read_message) {
				conn->current_write_message = conn->head_message;
				conn->head_message = conn->head_message->next;
				if (!conn->head_message) {
					conn->tail_message = NULL;
				}
			}
			if (conn->current_write_message) {
				queue_hid_command(conn->current_write_message->dev_cmd,
						 conn->current_write_message->payload,
						 conn->current_write_message->payload_size);
			}
		}

		int writing_done = 1;
		int reading_done = 1;

		if (conn->current_write_message) {
			writing_done = attempt_raw_hid_write();
		}
		reading_done = attempt_raw_hid_read();

		done = reading_done && writing_done;
	}
}

static void inotify_fd_readable()
{
	while (1) {
		u8 buf[4096];
		int rc = read(g_inotify_fd, buf, 4096);
		if (rc == -1 && errno == EINTR)
			break;
		if (rc == -1)
			break;
		int idx = 0;
		while (idx < rc) {
			struct inotify_event *ev = (struct inotify_event *)(buf + idx);
			printf("Device added: %d %s\n", ev->len, ev->name);
			if (!strcmp(ev->name, "signet") && g_opening_connection) {
				rc = attempt_open_connection();
				if (rc == 0) {
					g_device_opened_cb(g_device_opened_cb_param);
					break;
				}
			}
			idx += sizeof(struct inotify_event) + ev->len;
		}
		break;
	}
}

static void *transaction_thread(void *arg)
{
	struct signetdev_connection *conn = &g_connection;
	g_opening_connection = 0;
	conn->fd = -1;

	pthread_cleanup_push(handle_exit, NULL);

	g_poll_fd = epoll_create1(0);

	struct epoll_event ev_pipe;
	struct epoll_event ev_inotify;
	int rc;

	(void)arg;

	ev_pipe.events = EPOLLIN | EPOLLET;
	ev_pipe.data.fd = g_command_pipe[0];
	rc = epoll_ctl(g_poll_fd, EPOLL_CTL_ADD, g_command_pipe[0], &ev_pipe);
	if (rc)
		pthread_exit(NULL);

	struct epoll_event events[8];

	g_inotify_fd = inotify_init1(O_NONBLOCK);
	inotify_add_watch(g_inotify_fd, "/dev", IN_CREATE | IN_DELETE);
	ev_inotify.events = EPOLLIN | EPOLLET;
	ev_inotify.data.fd = g_inotify_fd;
	rc = epoll_ctl(g_poll_fd, EPOLL_CTL_ADD, g_inotify_fd, &ev_inotify);
	if (rc)
		pthread_exit(NULL);

	while (1) {
		command_pipe_io_iter();
		if (conn->fd != -1) {
			raw_hid_io_iter();
		}
		int nfds = epoll_wait(g_poll_fd, events, 8, -1);
		int i;
		for (i = 0; i < nfds; i++) {
			if (events[i].data.fd == g_inotify_fd) {
				inotify_fd_readable();
			}
			if (events[i].data.fd == conn->fd && (events[i].events & EPOLLERR)) {
				handle_error();
			}
		}
	}
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
	return NULL;
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

int signetdev_priv_send_message_async(void *user, int token, int dev_cmd, int api_cmd, const u8 *payload, unsigned int payload_size, int get_resp)
{
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
		r->resp = g_async_resp;
		r->resp_code = &g_async_resp_code;
	} else {
		r->resp = NULL;
		r->resp_code = NULL;
	}
	r->interrupt = 0;
	issue_command_no_resp(SIGNETDEV_CMD_MESSAGE, r);
	return 0;
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
