#ifndef SIGNETAPPLICATION_H
#define SIGNETAPPLICATION_H

#include <QObject>
#include <QMessageBox>
#include <qdatetime.h>
#include <qmetatype.h>
#ifndef Q_OS_ANDROID
#include <QtSingleApplication>
class SystemTray;
class MainWindow;
#else
#include <QApplication>
#include <QQmlApplicationEngine>
class SignetDeviceManager;
#endif
#include <QVector>

class QMessageBox;
class QByteArray;
class QString;

#ifdef WITH_BROWSER_PLUGINS
class QWebSocketServer;
class QWebSocket;
class websocketHandler;
#endif

struct signetdevCmdRespInfo {
	void *param;
	int token;
	int cmd;
	int resp_code;
	int end_device_state;
	int messages_remaining;
};

extern "C" {
#include "signetdev/host/signetdev.h"
#include "signetdev/common/signetdev_hc_common.h"
}

#include "systemtray.h"

Q_DECLARE_METATYPE(signetdevCmdRespInfo)
Q_DECLARE_METATYPE(signetdev_startup_resp_data)
Q_DECLARE_METATYPE(signetdev_get_progress_resp_data)
Q_DECLARE_METATYPE(cleartext_pass)
Q_DECLARE_METATYPE(enum signetdev_device_type)

class SignetAsyncListener
{
public:
	virtual void signetdevEventAsync(int eventType) = 0;
};

#ifdef Q_OS_ANDROID
class SignetApplication : public QApplication
#else
class SignetApplication : public QtSingleApplication
#endif
{
	Q_OBJECT
	static SignetApplication *g_singleton;
	QString m_dbFilename;
public:
	SignetApplication(int &argc, char **argv);
	~SignetApplication();
	void init(bool startInTray, QString emulateFilename);
	static QMessageBox *messageBoxError(QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent);
	static QMessageBox *messageBoxWarn(const QString &title, const QString &text, QWidget *parent);
	static SignetApplication *get()
	{
		return g_singleton;
	}
private:
	int m_iconPixelsDefault;
	int m_textPixelsDefault;
	int m_largeTextPixelsDefault;
#ifndef Q_OS_ANDROID
	SystemTray *m_systray;
	MainWindow *m_main_window;

#ifdef WITH_BROWSER_PLUGINS
	QWebSocketServer *m_webSocketServer;
	QList<websocketHandler *> m_openWebSockets;
	QStringList m_webSocketOriginWhitelist;
	static const int s_maxWebSocketConnections = 10;
	int m_nextSocketId;
#endif
#else
	QQmlApplicationEngine m_qmlEngine;
	SignetDeviceManager *m_signetDeviceManager;
#endif
	static void deviceOpenedS(enum signetdev_device_type dev_type, void *this_);
	static void deviceClosedS(void *this_);
	static void commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int end_device_state, int messages_remaining, int resp_code, const void *resp_data);
	static void deviceEventS(void *cb_param, int event_type, const void *data, int data_len);
	static void connectionErrorS(void *cb_param);
	static void generateScryptKey(const QString &password, QByteArray &key, const QByteArray &salt, unsigned int N, unsigned int r, unsigned int s);
	QByteArray m_hashfn;
	QByteArray m_salt;
	int m_keyLength;
	int m_saltLength;
	int m_DBFormat;
	int m_fwVersionMaj;
	int m_fwVersionMin;
	int m_fwVersionStep;
	enum signetdev_device_type m_deviceType;
	enum hc_boot_mode m_bootMode;
	SignetAsyncListener *m_signetAsyncListener;
public:
	int defaultIconHeight() {
		return m_iconPixelsDefault;
	}
	int defaultTextHeight() {
		return m_textPixelsDefault;
	}
	int largeTextHeight() {
		return m_largeTextPixelsDefault;
	}
#ifdef Q_OS_ANDROID
	QQmlApplicationEngine &qmlEngine()
	{
		return m_qmlEngine;
	}
#else
#ifdef WITH_BROWSER_PLUGINS
	void websocketResponse(int socketId, const QString &response);
#endif
#endif
	enum device_state {
		STATE_INVALID,
		STATE_NEVER_SHOWN,
		STATE_DISCONNECTING,
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_STARTING_DEVICE,
		STATE_RESET,
		STATE_UNINITIALIZED,
		STATE_LOGGED_OUT,
		STATE_LOGGED_IN_LOADING_ACCOUNTS,
		STATE_LOGGED_IN,
		STATE_WIPING,
		STATE_RESTORING,
		STATE_BACKING_UP,
		STATE_UPDATING_FIRMWARE,
		STATE_EXPORTING,
		STATE_BOOTLOADER
	};

	static QDate getReleaseDate()
	{
		return QDate(2020,4,11);
	}

	static int releasePeriod()
	{
		return 60;
	}

	void getClientVersion(int &major, int &minor, int &step, int &subStep)
	{
		major = 0;
		minor = 9;
		step = 16;
		subStep = 3;
	}

	void getFirmwareVersion(int &major, int &minor, int &step);

	void getConnectedFirmwareVersion(int &major, int &minor, int &step)
	{
		major = m_fwVersionMaj;
		minor = m_fwVersionMin;
		step = m_fwVersionStep;
	}

	void setConnectedFirmwareVersion(int major, int minor, int step)
	{
		m_fwVersionMaj = major;
		m_fwVersionMin = minor;
		m_fwVersionStep = step;
	}

	QByteArray getHashfn()
	{
		return m_hashfn;
	}

	QByteArray getSalt()
	{
		return m_salt;
	}

	int getKeyLength()
	{
		return m_keyLength;
	}

	int getSaltLength()
	{
		return m_keyLength;
	}

	int getDBFormat()
	{
		return m_DBFormat;
	}

	void setHashfn(QByteArray hashfn)
	{
		m_hashfn = hashfn;
	}

	void setSalt(QByteArray salt)
	{
		m_salt = salt;
	}

	void setKeyLength(int keyLength)
	{
		m_keyLength = keyLength;
	}

	void setSaltLength(int saltLength)
	{
		m_saltLength = saltLength;
	}

	void setBootMode(enum hc_boot_mode mode)
	{
		m_bootMode = mode;
	}

	void setDeviceType(enum signetdev_device_type dev_type)
	{
		m_deviceType = dev_type;
	}

	enum hc_boot_mode getBootMode()
	{
		return m_bootMode;
	}

	void setDBFormat(int DBFormat)
	{
		m_DBFormat = DBFormat;
	}

	static void generateKey(const QString &password, QByteArray &key, const QByteArray &hashfn, const QByteArray &salt, int keyLength);
	void setAsyncListener(SignetAsyncListener *l);
    bool isDeviceEmulated();
    void startWebsocketServer();
    void stopWebsocketServer();
signals:
	void deviceOpened(enum signetdev_device_type dev_type);
	void deviceClosed();
	void connectionError();
	void signetdevCmdResp(const signetdevCmdRespInfo info);
	void signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data resp);
	void signetdevStartupResp(signetdevCmdRespInfo info, signetdev_startup_resp_data resp);
	void signetdevReadUIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask);
	void signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask);
	void signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block);
	void signetdevGetRandBits(signetdevCmdRespInfo info, QByteArray block);
	void signetdevReadCleartextPasswordNames(signetdevCmdRespInfo info, QVector<int> f, QStringList l);
	void signetdevReadCleartextPassword(signetdevCmdRespInfo info, cleartext_pass pass);
	void signetdevEvent(int event_type);
	void signetdevTimerEvent(int seconds_remaining);
#ifdef WITH_BROWSER_PLUGINS
	void websocketMessage(int socketId, QString url);
#endif
public slots:
#ifndef Q_OS_ANDROID
	void trayActivated(QSystemTrayIcon::ActivationReason reason);
#endif
private slots:
#ifndef Q_OS_ANDROID
#ifdef WITH_BROWSER_PLUGINS
	void newWebSocketConnection();
	void websocketHandlerDone(websocketHandler *handler);
	void websocketMessage_(int, QString);
#endif
#endif
};

#endif // SIGNETAPPLICATION_H
