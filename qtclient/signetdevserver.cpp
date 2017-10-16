#include "signetdevserver.h"
#include "signetdevserverconnection.h"

extern "C" {
#include "signetdev.h"
};

#include <QWebSocketServer>
#include <QWebSocket>

signetdevServer::signetdevServer(QObject *parent) : QObject(parent)
{

}

void signetdevServer::init()
{
	signetdev_initialize_api();
	signetdev_set_device_opened_cb(deviceOpenedS, this);
	signetdev_set_device_closed_cb(deviceClosedS, this);
	signetdev_set_command_resp_cb(commandRespS, this);
	signetdev_set_device_event_cb(deviceEventS, this);
	signetdev_set_error_handler(connectionErrorS, this);

	m_socketServer = new QWebSocketServer("signet",
				QWebSocketServer::SslMode::SecureMode,
				this);
	connect(m_socketServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
	if (m_socketServer->listen(QHostAddress::LocalHost, USB_VENDOR_ID)) {
		(void)(0);
	}
}

void signetdevServer::deviceOpenedS(void *this_)
{
	((signetdevServer *)this_)->deviceOpened();
}

void signetdevServer::deviceClosedS(void *this_)
{
	((signetdevServer *)this_)->deviceClosed();
}

void signetdevServer::connectionErrorS(void *this_)
{
	((signetdevServer *)this_)->connectionError();
}

void signetdevServer::commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int messages_remaining, int resp_code, void *resp_data)
{
	signetdevCmdRespInfo info;
	info.param = cmd_user_param;
	info.cmd = cmd;
	info.token = cmd_token;
	info.resp_code = resp_code;
	info.messages_remaining = messages_remaining;

	signetdevServer *this_ = (signetdevServer *)cb_param;
	switch(cmd) {
	case SIGNETDEV_CMD_READ_ALL_ID: {
		const signetdev_read_all_id_resp_data *resp = (const signetdev_read_all_id_resp_data *)resp_data;
		this_->signetdevReadAllIdResp(info, resp);
	} break;
	case SIGNETDEV_CMD_READ_BLOCK: {
		this_->signetdevReadBlockResp(info, (const char *)resp_data);
	} break;
	case SIGNETDEV_CMD_GET_PROGRESS: {
		const signetdev_get_progress_resp_data *resp = (const signetdev_get_progress_resp_data *)resp_data;
		this_->signetdevGetProgressResp(info, resp);
	} break;
	case SIGNETDEV_CMD_STARTUP: {
		const signetdev_startup_resp_data *resp = (const signetdev_startup_resp_data *)resp_data;
		this_->signetdevStartupResp(info, resp);
	} break;
	case SIGNETDEV_CMD_READ_ID: {
		const signetdev_read_id_resp_data *resp = (const signetdev_read_id_resp_data *)resp_data;
		this_->signetdevReadIdResp(info, resp);
	} break;
	default:
		this_->signetdevCmdResp(info);
		break;
	}
}


void signetdevServer::deviceEventS(void *cb_param, int event_type, void *data, int data_len)
{
	Q_UNUSED(data_len);
	signetdevServer *this_ = (signetdevServer *)cb_param;;
	switch(event_type) {
	case 1:
		this_->signetdevEvent(event_type);
		break;
	case 2:
		this_->signetdevTimerEvent(((u8 *)data)[0]);
		break;
	}
}

/*
 * Socket handlers
 */

void signetdevServer::newConnection()
{
	QWebSocket *socket = m_socketServer->nextPendingConnection();
	if (socket) {
		auto *conn = new signetdevServerConnection(socket, this);
		m_connections.append(conn);
	}
}

/*
 * Event handlers
 */

void signetdevServer::deviceOpened()
{

}

void signetdevServer::deviceClosed()
{

}

void signetdevServer::connectionError()
{

}

void signetdevServer::signetdevEvent(int event_type)
{

}

void signetdevServer::signetdevTimerEvent(int seconds_remaining)
{

}

/*
 * Command response handlers
 */

void signetdevServer::signetdevCmdResp(const signetdevCmdRespInfo &info)
{

}

void signetdevServer::signetdevGetProgressResp(const signetdevCmdRespInfo &info, const signetdev_get_progress_resp_data *resp)
{

}

void signetdevServer::signetdevStartupResp(const signetdevCmdRespInfo &info, const signetdev_startup_resp_data *resp)
{

}

void signetdevServer::signetdevReadIdResp(const signetdevCmdRespInfo &info, const signetdev_read_id_resp_data *resp)
{

}

void signetdevServer::signetdevReadAllIdResp(const signetdevCmdRespInfo &info, const signetdev_read_all_id_resp_data *resp)
{

}

void signetdevServer::signetdevReadBlockResp(const signetdevCmdRespInfo &info, const char *data)
{

}

