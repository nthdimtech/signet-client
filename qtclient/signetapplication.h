#ifndef SIGNETAPPLICATION_H
#define SIGNETAPPLICATION_H

#include <QObject>
#include <QMessageBox>
#include <QtSingleApplication>

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
};

extern "C" {
#include "signetdev.h"
}

Q_DECLARE_METATYPE(signetdevCmdRespInfo)
Q_DECLARE_METATYPE(signetdev_read_id_resp_data)
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
	static void generateKey(const QString &password, QByteArray &key, const QByteArray &hashfn, const QByteArray &salt);
private:
	SystemTray m_systray;
	MainWindow *m_main_window;
	static void deviceOpenedS(void *this_);
	static void deviceClosedS(void *this_);
	static void commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int resp_code, void *resp_data);
	static void deviceEventS(void *cb_param, int event_type, void *data, int data_len);
	static void connectionErrorS(void *cb_param);
	static void generateScryptKey(const QString &password, QByteArray &key, const QByteArray &salt, unsigned int N, unsigned int r, unsigned int s);
	QByteArray m_hashfn;
	QByteArray m_salt;
public:
	QByteArray getHashfn()
	{
		return m_hashfn;
	}

	QByteArray getSalt()
	{
		return m_salt;
	}

	void setHashfn(QByteArray hashfn)
	{
		m_hashfn = hashfn;
	}

	void setSalt(QByteArray salt)
	{
		m_salt = salt;
	}
signals:
	void deviceOpened();
	void deviceClosed();
	void connectionError();
	void signetdevCmdResp(const signetdevCmdRespInfo info);
	void signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data resp);
	void signetdevStartupResp(signetdevCmdRespInfo info, int device_state, QByteArray hashfn, QByteArray salt);
	void signetdevReadIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask);
	void signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block);
	void signetdevEvent(int event_type);
public slots:
	void mainDestroyed();
	void trayActivated(QSystemTrayIcon::ActivationReason reason);
};

#endif // SIGNETAPPLICATION_H
