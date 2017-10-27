#include "signetdevserver.h"
#include "signetdevserverconnection.h"

extern "C" {
#include "signetdev.h"
};

#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>

int signetdevServer::lookupIntMap(const QMap<QString, int> &map, const QString &key, int defaultValue)
{
	QMap<QString, int>::const_iterator iter = map.find(key);
	if (iter != map.end()) {
		return iter.value();
	} else {
		return defaultValue;
	}
}

QString signetdevServer::lookupStrMap(const QMap<int, QString> &map, int key, const QString &defaultValue)
{
	QMap<int, QString>::const_iterator iter = map.find(key);
	if (iter != map.end()) {
		return iter.value();
	} else {
		return defaultValue;
	}
}

void signetdevServer::processQueue()
{
	if (!m_activeCommand && m_commandQueue.size()) {
		m_activeCommand = m_commandQueue.front();
		m_commandQueue.pop_front();
		if (m_activeCommand) {
			processActiveCommand();
		}
	}
}

void signetdevServer::processActiveCommand()
{
	if (!m_activeCommand) {
		return;
	}
	if (m_deviceState != m_activeCommand->targetDeviceState) {
		delete m_activeCommand;
		m_activeCommand = NULL;
		//TODO: return error to client about state mismatch
		processQueue();
		return;
	}
	switch (m_activeCommand->command) {
	case SIGNETDEV_CMD_LOGOUT:
		signetdev_logout_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_WIPE:
		signetdev_wipe_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_BUTTON_WAIT:
		signetdev_button_wait_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_BEGIN_DEVICE_BACKUP:
		signetdev_begin_device_backup_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_END_DEVICE_BACKUP:
		signetdev_end_device_backup_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_BEGIN_DEVICE_RESTORE:
		signetdev_begin_device_restore_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_END_DEVICE_RESTORE:
		signetdev_end_device_restore_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_BEGIN_UPDATE_FIRMWARE:
		signetdev_begin_update_firmware_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_RESET_DEVICE:
		signetdev_reset_device_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_STARTUP:
		signetdev_startup_async(this, &m_activeCommand->serverToken);
		break;
	case SIGNETDEV_CMD_DELETE_ID:
		signetdev_delete_id_async(this,
			&m_activeCommand->serverToken,
			getIntParam("id", -1));
		break;
	case SIGNETDEV_CMD_READ_ID:
		signetdev_read_id_async(this,
			&m_activeCommand->serverToken,
			getIntParam("id", -1),
			getIntParam("masked", -1));
		break;
	case SIGNETDEV_CMD_WRITE_ID: {
		QByteArray data = getBinaryParam("data");
		QByteArray mask = getBinaryParam("mask");
		int maskSize = ((data.size()+7)/8);
		if (mask.size() != maskSize) {
			//TODO: protocol error
			break;
		}
		signetdev_write_id_async(this,
			&m_activeCommand->serverToken,
			getIntParam("id", -1),
			data.size(),
			(const u8 *)data.data(),
			(const u8 *)mask.data());
		} break;
	case SIGNETDEV_CMD_TYPE: {
		QString keys = getStringParam("keys");
		QByteArray keysUTF8 = keys.toUtf8();
		signetdev_type_async(this,
			&m_activeCommand->serverToken,
			(const u8 *)keysUTF8.data(),
			keysUTF8.size());
		} break;
	case SIGNETDEV_CMD_BEGIN_INITIALIZE_DEVICE: {
		QByteArray key = getBinaryParam("key");
		QByteArray hashFn = getBinaryParam("hashFn");
		QByteArray salt = getBinaryParam("salt");
		QByteArray randData = getBinaryParam("randData");
		signetdev_begin_initialize_device_async(this,
			&m_activeCommand->serverToken,
			(const u8 *)key.data(), key.length(),
			(const u8 *)hashFn.data(), hashFn.length(),
			(const u8 *)salt.data(), salt.length(),
			(const u8 *)randData.data(), randData.length());
		} break;
	case SIGNETDEV_CMD_READ_BLOCK:
		signetdev_read_block_async(this,
			&m_activeCommand->serverToken,
			getIntParam("idx"));
		break;
	case SIGNETDEV_CMD_WRITE_BLOCK: {
		QByteArray data = getBinaryParam("data");
		if (data.size() != BLK_SIZE) {
			//TODO: protocol error
			break;
		}
		signetdev_write_block_async(this,
			&m_activeCommand->serverToken,
			getIntParam("idx"),
			(const u8 *)data.data());
		} break;
	}
}

QByteArray signetdevServer::getBinaryParam(const QString &key)
{
	QString stringVal = m_activeCommand->params.value(key).toString();
	return QByteArray::fromBase64(stringVal.toUtf8());
}

QString signetdevServer::getStringParam(const QString &key)
{
	return m_activeCommand->params.value(key).toString();
}

int signetdevServer::getIntParam(const QString &key, int defaultValue)
{
	return m_activeCommand->params.value(key).toInt(defaultValue);
}

signetdevServer::signetdevServer(QObject *parent) :
	QObject(parent),
	m_activeCommand(NULL)
{
	//State on power up or after a "disconnect" command
	m_deviceStateMap.insert("DISCONNECTED", DISCONNECTED);

	//After startup no database on device
	m_deviceStateMap.insert("UNINITIALIZED", UNINITIALIZED);

	//After startup database is present
	m_deviceStateMap.insert("LOGGED_OUT", LOGGED_OUT);

	//After successful login
	m_deviceStateMap.insert("LOGGED_IN", LOGGED_IN);

	//Command/transient states
	m_deviceStateMap.insert("INITIALIZING", INITIALIZING);
	m_deviceStateMap.insert("WIPING", WIPING);
	m_deviceStateMap.insert("ERASING_PAGES", ERASING_PAGES);
	m_deviceStateMap.insert("FIRMWARE_UPDATE", FIRMWARE_UPDATE);
	m_deviceStateMap.insert("BACKING_UP_DEVICE", BACKING_UP_DEVICE);
	m_deviceStateMap.insert("RESTORING_DEVICE", RESTORING_DEVICE);

	for (auto x = m_deviceStateMap.begin(); x != m_deviceStateMap.end(); x++) {
		m_invDeviceStateMap.insert(x.value(),x.key());
	}
	m_deviceState = DISCONNECTED;
	m_deviceTransientState = DISCONNECTED;

	m_commandMap.insert("STARTUP", SIGNETDEV_CMD_STARTUP);
	m_commandMap.insert("LOGOUT", SIGNETDEV_CMD_LOGOUT);
	m_commandMap.insert("LOGIN", SIGNETDEV_CMD_LOGIN);
	m_commandMap.insert("WIPE", SIGNETDEV_CMD_WIPE);
	m_commandMap.insert("BUTTON_WAIT", SIGNETDEV_CMD_BUTTON_WAIT);
	m_commandMap.insert("DISCONNECT", SIGNETDEV_CMD_DISCONNECT);
	m_commandMap.insert("GET_PROGRESS", SIGNETDEV_CMD_GET_PROGRESS);
	m_commandMap.insert("BEGIN_DEVICE_BACKUP", SIGNETDEV_CMD_BEGIN_DEVICE_BACKUP);
	m_commandMap.insert("END_DEVICE_BACKUP", SIGNETDEV_CMD_END_DEVICE_BACKUP);
	m_commandMap.insert("BEGIN_DEVICE_RESTORE", SIGNETDEV_CMD_BEGIN_DEVICE_RESTORE);
	m_commandMap.insert("END_DEVICE_RESTORE", SIGNETDEV_CMD_END_DEVICE_RESTORE);
	m_commandMap.insert("BEGIN_UPDATE_FIRMWARE", SIGNETDEV_CMD_BEGIN_UPDATE_FIRMWARE);
	m_commandMap.insert("RESET_DEVICE", SIGNETDEV_CMD_RESET_DEVICE);
	m_commandMap.insert("READ_ID", SIGNETDEV_CMD_READ_ID);
	m_commandMap.insert("WRITE_ID", SIGNETDEV_CMD_WRITE_ID);
	m_commandMap.insert("DELETE_ID", SIGNETDEV_CMD_DELETE_ID);
	m_commandMap.insert("TYPE", SIGNETDEV_CMD_TYPE);
	m_commandMap.insert("CHANGE_MASTER_PASSWORD", SIGNETDEV_CMD_CHANGE_MASTER_PASSWORD);
	m_commandMap.insert("BEGIN_INITIALIZE_DEVICE", SIGNETDEV_CMD_BEGIN_INITIALIZE_DEVICE);
	m_commandMap.insert("READ_BLOCK", SIGNETDEV_CMD_READ_BLOCK);
	m_commandMap.insert("WRITE_BLOCK", SIGNETDEV_CMD_WRITE_BLOCK);
	m_commandMap.insert("WRITE_FLASH", SIGNETDEV_CMD_WRITE_FLASH);
	m_commandMap.insert("ERASE_PAGES", SIGNETDEV_CMD_ERASE_PAGES);
	m_commandMap.insert("READ_ALL_ID", SIGNETDEV_CMD_READ_ALL_ID);
	m_commandMap.insert("ENTER_MOBILE_MODE", SIGNETDEV_CMD_ENTER_MOBILE_MODE);

	m_invCommandRespMap.insert(OKAY, "OKAY");
	m_invCommandRespMap.insert(INVALID_STATE, "INVALID_STATE");
	m_invCommandRespMap.insert(INVALID_INPUT, "INVALID_INPUT");
	m_invCommandRespMap.insert(ID_INVALID, "ID_INVALID");
	m_invCommandRespMap.insert(ID_NOT_OPEN, "ID_NOT_OPEN");
	m_invCommandRespMap.insert(TAG_INVALID, "TAG_INVALID");
	m_invCommandRespMap.insert(WRITE_FAILED, "WRITE_FAILED");
	m_invCommandRespMap.insert(NOT_LOGGED_IN, "NOT_LOGGED_IN");
	m_invCommandRespMap.insert(NOT_INITIALIZED, "NOT_INITIALIZED");
	m_invCommandRespMap.insert(BAD_PASSWORD, "BAD_PASSWORD");
	m_invCommandRespMap.insert(IN_PROGRESS, "IN_PROGRESS");
	m_invCommandRespMap.insert(NOT_ENOUGH_SPACE, "NOT_ENOUGH_SPACE");
	m_invCommandRespMap.insert(DONE, "DONE");
	m_invCommandRespMap.insert(BUTTON_PRESS_CANCELED, "BUTTON_PRESS_CANCELED");
	m_invCommandRespMap.insert(BUTTON_PRESS_TIMEOUT, "BUTTON_PRESS_TIMEOUT");
	m_invCommandRespMap.insert(UNKNOWN_DB_FORMAT, "UNKNOWN_DB_FORMAT");

	m_invEventMap.insert(1, "IDLE_BUTTON_PRESS");
	m_invEventMap.insert(2, "TIMER_EVENT");
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

void signetdevServer::textMessageReceived(signetdevServerConnection *conn, const QString &message)
{
	QJsonDocument a = QJsonDocument::fromJson(message.toUtf8());
	const QJsonObject &o = a.object();
	signetdevServerCommand *command = new signetdevServerCommand();
	command->clientToken = o.value("token").toInt(-1);
	command->messagesRemaining = o.value("messagesRemaining").toInt(0);

	command->targetDeviceState =
			lookupIntMap(m_deviceStateMap,
				o.value("deviceState").toString(),
				-1);
	command->command =
			lookupIntMap(m_commandMap,
				o.value("command").toString(),
				-1);
	command->conn = conn;
	command->params = o.value("params").toObject();
	m_commandQueue.push_back(command);
	processQueue();
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

void signetdevServer::commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int end_device_state, int messages_remaining, int resp_code, void *resp_data)
{
	signetdevCmdRespInfo info;
	info.param = cmd_user_param;
	info.cmd = cmd;
	info.token = cmd_token;
	info.resp_code = resp_code;
	info.end_device_state = end_device_state;
	info.messages_remaining = messages_remaining;

	signetdevServer *this_ = (signetdevServer *)cb_param;
	int prevDeviceState = this_->m_deviceState;
	switch (info.end_device_state) {
	case DISCONNECTED:
	case UNINITIALIZED:
	case LOGGED_OUT:
	case LOGGED_IN:
		this_->m_deviceState = info.end_device_state;
		this_->m_deviceTransientState = this_->m_deviceState;
		if (this_->m_deviceState != prevDeviceState) {
			this_->deviceStateChanged();
		}
		break;
	case INITIALIZING:
	case WIPING:
	case ERASING_PAGES:
	case FIRMWARE_UPDATE:
	case BACKING_UP_DEVICE:
	case RESTORING_DEVICE:
		this_->m_deviceTransientState = this_->m_deviceState;
		break;
	}

	this_->m_deviceState = info.end_device_state;

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
	QJsonObject params;
	sendEvent("DEVICE_OPENED", params);
}

void signetdevServer::deviceClosed()
{
	QJsonObject params;
	sendEvent("DEVICE_CLOSED", params);
}

void signetdevServer::deviceStateChanged()
{
	QJsonObject params;
	params.insert("deviceState",
		QJsonValue(lookupStrMap(m_invDeviceStateMap,
				m_deviceState,
				"UNKNOWN_STATE")));
	sendEvent("DEVICE_STATE_CHANGED", params);
}

void signetdevServer::connectionError()
{
	//TODO
}

void signetdevServer::signetdevEvent(int eventType)
{
	QJsonObject params;
	QString eventName = lookupStrMap(m_invEventMap, eventType, "UNKNOWN_EVENT");
	sendEvent(eventName, params);
}

void signetdevServer::signetdevTimerEvent(int secondsRemaining)
{
	QJsonObject params;
	params.insert("secondsRemaining", QJsonValue(secondsRemaining));
	sendEvent("TIMER_EVENT", params);
}

/*
 * Command response handlers
 */


void signetdevServer::sendJsonDocument(QJsonDocument &a)
{
	m_activeCommand->conn->m_socket->sendTextMessage(QString::fromUtf8(a.toJson()));
}

void signetdevServer::sendResponse(const signetdevCmdRespInfo &info, QJsonObject &params)
{
	QJsonDocument a;
	a.object().insert("messagesRemaining", QJsonValue(info.messages_remaining));
	a.object().insert("token", QJsonValue(m_activeCommand->clientToken));
	a.object().insert("respCode", QJsonValue(lookupStrMap(m_invCommandRespMap, info.resp_code, "UNKNOWN_RESPONSE")));
	a.object().insert("deviceState",
		QJsonValue(lookupStrMap(m_invDeviceStateMap,
				info.end_device_state,
				"UNKNOWN_STATE")));
	a.object().insert("params", params);
	sendJsonDocument(a);
}

void signetdevServer::sendEvent(const QString &eventName, QJsonObject &params)
{
	QJsonDocument a;
	a.object().insert("eventName", QJsonValue(eventName));
	a.object().insert("deviceState", QJsonValue(lookupStrMap(m_invDeviceStateMap, m_deviceState,
						 "UNKNOWN_STATE")));
	a.object().insert("params", params);
	sendJsonDocument(a);
}

void signetdevServer::signetdevCmdResp(const signetdevCmdRespInfo &info)
{
	if (info.token == m_activeCommand->serverToken) {
		QJsonObject params;
		sendResponse(info, params);
	}
}

#include <QJsonArray>

void signetdevServer::signetdevGetProgressResp(const signetdevCmdRespInfo &info, const signetdev_get_progress_resp_data *resp)
{
	if (info.token == m_activeCommand->serverToken) {
		QJsonObject params;
		if (resp) {
			params.insert("totalProgress", QJsonValue(resp->total_progress));
			params.insert("totalProgressMaximum", QJsonValue(resp->total_progress_maximum));
			//resp->
			QJsonArray progress;
			QJsonArray progressMaximum;
			for (int i = 0; i < resp->n_components; i++) {
				progress.append(QJsonValue(progress[i]));
				progress.append(QJsonValue(progressMaximum[i]));
			}
			params.insert("progress", progress);
			params.insert("progressMaximum", progressMaximum);
		}
		sendResponse(info, params);
	}
}

void signetdevServer::signetdevStartupResp(const signetdevCmdRespInfo &info, const signetdev_startup_resp_data *resp)
{
	if (info.token == m_activeCommand->serverToken) {
		QJsonObject params;
		QByteArray salt((char *)(resp->salt), sizeof(resp->salt));
		//TODO: this should be decoded or alternately have the server compute the hash
		QByteArray hashfn((char *)(resp->hashfn), sizeof(resp->hashfn));
		params.insert("salt", QJsonValue(QString::fromUtf8(salt.toBase64())));
		params.insert("hashfn", QJsonValue(QString::fromUtf8(hashfn.toBase64())));
		sendResponse(info, params);
	}
}

void signetdevServer::signetdevReadIdResp(const signetdevCmdRespInfo &info, const signetdev_read_id_resp_data *resp)
{
	if (info.token == m_activeCommand->serverToken) {
		QJsonObject params;
		QByteArray data((char *)(resp->data), resp->size);
		QByteArray mask((char *)(resp->mask), (resp->size+7)/8);
		params.insert("data", QJsonValue(QString::fromUtf8(data.toBase64())));
		params.insert("mask", QJsonValue(QString::fromUtf8(data.toBase64())));
		sendResponse(info, params);
	}
}

void signetdevServer::signetdevReadAllIdResp(const signetdevCmdRespInfo &info, const signetdev_read_all_id_resp_data *resp)
{
	if (info.token == m_activeCommand->serverToken) {
		QJsonObject params;
		QByteArray data((char *)(resp->data), resp->size);
		QByteArray mask((char *)(resp->mask), (resp->size+7)/8);
		params.insert("id", QJsonValue(resp->id));
		params.insert("data", QJsonValue(QString::fromUtf8(data.toBase64())));
		params.insert("mask", QJsonValue(QString::fromUtf8(data.toBase64())));
		sendResponse(info, params);
	}
}

void signetdevServer::signetdevReadBlockResp(const signetdevCmdRespInfo &info, const char *data)
{
	if (info.token == m_activeCommand->serverToken) {
		QJsonObject params;
		QByteArray block((char *)(data), BLK_SIZE);
		params.insert("data", QJsonValue(QString::fromUtf8(block.toBase64())));
		sendResponse(info, params);
		//TODO: fill out params
	}
}
