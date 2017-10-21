#ifndef SIGNETDEVSERVER_H
#define SIGNETDEVSERVER_H

#include "signetdevserver.h"
#include "signetdevserverconnection.h"

#include <QObject>
#include <QList>
#include <QQueue>
#include <QMap>
#include <QJsonObject>

class QWebSocket;
class QWebSocketServer;

struct signetdevCmdRespInfo;
struct signetdev_get_progress_resp_data;
struct signetdev_startup_resp_data;
struct signetdev_read_id_resp_data;
struct signetdev_read_all_id_resp_data;

struct signetdevCmdRespInfo {
	void *param;
	int token;
	int cmd;
	int resp_code;
	int end_device_state;
	int messages_remaining;
};

struct signetdevServerCommand {
	int targetDeviceState;
	int clientToken;
	int serverToken;
	int command;
	int messagesRemaining;
	signetdevServerConnection *conn;
	QJsonObject params;
};

class signetdevServer : public QObject
{
	Q_OBJECT
	QList<signetdevServerConnection *> m_connections;
	QWebSocketServer *m_socketServer;
	QQueue<signetdevServerCommand *> m_commandQueue;
	QMap<QString, int> m_deviceStateMap;
	QMap<QString, int> m_commandMap;
	int lookupIntMap(const QMap<QString, int> &map,
			  const QString &key,
			  int defaultValue = 0);
	void processQueue();
	signetdevServerCommand *m_activeCommand;
	void processActiveCommand();
	int m_deviceState;

	QByteArray getBinaryParam(const QString &key);
	QString getStringParam(const QString &key);
	int getIntParam(const QString &key, int defaultValue = -1);
public:
	explicit signetdevServer(QObject *parent = 0);
	static void deviceOpenedS(void *this_);
	static void deviceClosedS(void *this_);
	static void connectionErrorS(void *this_);
	static void deviceEventS(void *cb_param, int event_type, void *data, int data_len);
	static void commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int end_device_state, int messages_remaining, int resp_code, void *resp_data);
	void init();

	void deviceOpened();
	void deviceClosed();
	void connectionError();
	void signetdevCmdResp(const signetdevCmdRespInfo &info);
	void signetdevGetProgressResp(const signetdevCmdRespInfo &info, const signetdev_get_progress_resp_data *resp);
	void signetdevStartupResp(const signetdevCmdRespInfo &info, const signetdev_startup_resp_data *resp);
	void signetdevReadIdResp(const signetdevCmdRespInfo &info, const signetdev_read_id_resp_data *resp);
	void signetdevReadAllIdResp(const signetdevCmdRespInfo &info, const signetdev_read_all_id_resp_data *resp);
	void signetdevReadBlockResp(const signetdevCmdRespInfo &info, const char *data);
	void signetdevEvent(int event_type);
	void signetdevTimerEvent(int seconds_remaining);
	void textMessageReceived(signetdevServerConnection *conn, const QString &message);
public slots:
	void newConnection();
};


#endif // SIGNETDEVSERVER_H
