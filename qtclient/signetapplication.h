#ifndef SIGNETAPPLICATION_H
#define SIGNETAPPLICATION_H

#include <QObject>
#include <QMessageBox>
#include <QtSingleApplication>
#include <qdatetime.h>
#include <qmetatype.h>
#include "systemtray.h"

class QMessageBox;
class QByteArray;
class QString;
class MainWindow;

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
}

Q_DECLARE_METATYPE(signetdevCmdRespInfo)
Q_DECLARE_METATYPE(signetdev_startup_resp_data)
Q_DECLARE_METATYPE(signetdev_get_progress_resp_data)

class SignetApplication : public QtSingleApplication
{
	Q_OBJECT
	static SignetApplication *g_singleton;
public:
	SignetApplication(int &argc, char **argv);
	void init();
	static QMessageBox *messageBoxError(QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent);
	static SignetApplication *get()
	{
		return g_singleton;
	}
private:
	SystemTray m_systray;
	MainWindow *m_main_window;
	static void deviceOpenedS(void *this_);
	static void deviceClosedS(void *this_);
	static void commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int end_device_state, int messages_remaining, int resp_code, void *resp_data);
	static void deviceEventS(void *cb_param, int event_type, void *data, int data_len);
	static void connectionErrorS(void *cb_param);
	static void generateScryptKey(const QString &password, QByteArray &key, const QByteArray &salt, unsigned int N, unsigned int r, unsigned int s);
	QByteArray m_hashfn;
	QByteArray m_salt;
	int m_keyLength;
	int m_saltLength;
	int m_DBFormat;
public:
	static QDate getReleaseDate() {
		return QDate(2017,12,15);
	}

	void getClientVersion(int &major, int &minor, int &step) {
		major = 0;
		minor = 9;
		step = 6;
	}

	void getFirmwareVersion(int &major, int &minor, int &step) {
		major = 1;
		minor = 2;
		step = 2;
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

	void setDBFormat(int DBFormat)
	{
		m_DBFormat = DBFormat;
	}

	static void generateKey(const QString &password, QByteArray &key, const QByteArray &hashfn, const QByteArray &salt, int keyLength);
signals:
	void deviceOpened();
	void deviceClosed();
	void connectionError();
	void signetdevCmdResp(const signetdevCmdRespInfo info);
	void signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data resp);
	void signetdevStartupResp(signetdevCmdRespInfo info, signetdev_startup_resp_data resp);
	void signetdevReadUIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask);
	void signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask);
	void signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block);
	void signetdevGetRandBits(signetdevCmdRespInfo info, QByteArray block);
	void signetdevEvent(int event_type);
	void signetdevTimerEvent(int seconds_remaining);
public slots:
	void mainDestroyed();
	void trayActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // SIGNETAPPLICATION_H
