#include "signetapplication.h"

#ifndef Q_OS_ANDROID
#include "desktop/mainwindow.h"
#else
#include "android/signetdevicemanager.h"
#endif

#include <QMenu>
#include <QDesktopWidget>
#include <QApplication>

extern "C" {
#include "signetdev/host/signetdev.h"
#include "crypto_scrypt.h"
};

#define DEFAULT_SCRYPT_N_VALUE_LOG2 12
#define DEFAULT_SCRYPT_N_VALUE (1<<DEFAULT_SCRYPT_N_VALUE_LOG2)
#define DEFAULT_SCRYPT_R_VALUE 32
#define DEFAULT_SCRIPT_P_VALUE 1

SignetApplication *SignetApplication::g_singleton = NULL;

void SignetApplication::deviceOpenedS(void *this_)
{
	((SignetApplication *)this_)->deviceOpened();
}

void SignetApplication::deviceClosedS(void *this_)
{
	((SignetApplication *)this_)->deviceClosed();
}

void SignetApplication::connectionErrorS(void *this_)
{
	((SignetApplication *)this_)->connectionError();
}

SignetApplication::SignetApplication(int &argc, char **argv) :
#ifdef Q_OS_ANDROID
	QApplication(argc, argv),
#else
	QtSingleApplication("qtsingle-app-signetdev-" + QString(USB_VENDOR_ID) + "-" + QString(USB_SIGNET_DESKTOP_PRODUCT_ID) ,argc, argv),
#endif
	m_signetAsyncListener(NULL)
{
	g_singleton = this;
	qRegisterMetaType<signetdevCmdRespInfo>();
	qRegisterMetaType<signetdev_startup_resp_data>();
	qRegisterMetaType<signetdev_get_progress_resp_data>();
	qRegisterMetaType<cleartext_pass>();
	qRegisterMetaType<QVector<int> >("QVector<int>");
}

void SignetApplication::mainDestroyed()
{
	::signetdev_deinitialize_api();
}

void SignetApplication::generateScryptKey(const QString &password, QByteArray &key, const QByteArray &salt, unsigned int N, unsigned int r, unsigned int s)
{
	QByteArray password_utf8 = password.toUtf8();
	crypto_scrypt((u8 *)password_utf8.data(), password_utf8.size(),
		      (u8 *)salt.data(), salt.size(),
		      N, r, s,
		      (u8 *)key.data(), key.size());

}

void SignetApplication::generateKey(const QString &password, QByteArray &key, const QByteArray &hashfn, const QByteArray &salt, int keyLength)
{
	QByteArray s = password.toUtf8();
	key.resize(keyLength);
	memset(key.data(), 0, key.length());

	int fn = hashfn.at(0);

	switch(fn) {
	case 0: {
		int N = DEFAULT_SCRYPT_N_VALUE;
		int r = DEFAULT_SCRYPT_R_VALUE;
		int p = DEFAULT_SCRIPT_P_VALUE;
		QByteArray actual_salt("rand", 4);
		generateScryptKey(password, key, actual_salt, N, r, p);
	}
	break;
	case 1: {
		if (hashfn.size() >= 5) {
			unsigned int N = ((unsigned int )1) << hashfn.at(1);
			unsigned int r = ((unsigned int)hashfn.at(2)) + (((unsigned int)hashfn.at(3))<<8);
			unsigned int p = (unsigned int)hashfn.at(4);
			generateScryptKey(password, key, salt, N, r, p);
		}
	}
	break;
	default:
		break;
	}
}

void SignetApplication::deviceEventS(void *cb_param, int event_type, void *data, int data_len)
{
	Q_UNUSED(data_len);
	SignetApplication *this_ = (SignetApplication *)cb_param;;
	switch(event_type) {
	case 1:
		if (this_->m_signetAsyncListener) {
			this_->m_signetAsyncListener->signetdevEventAsync(event_type);
		}
		this_->signetdevEvent(event_type);
		break;
	case 2:
		this_->signetdevTimerEvent(((u8 *)data)[0]);
		break;
	}
}

void SignetApplication::setAsyncListener(SignetAsyncListener *l)
{
	m_signetAsyncListener = l;
}

void SignetApplication::commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int end_device_state, int messages_remaining, int resp_code, void *resp_data)
{
	signetdevCmdRespInfo info;
	info.param = cmd_user_param;
	info.cmd = cmd;
	info.token = cmd_token;
	info.resp_code = resp_code;
	info.end_device_state = end_device_state;
	info.messages_remaining = messages_remaining;

	SignetApplication *this_ = (SignetApplication *)cb_param;
	switch(cmd) {
	case SIGNETDEV_CMD_READ_ALL_UIDS: {
		if (resp_data && resp_code == OKAY) {
			signetdev_read_all_uids_resp_data *resp = (signetdev_read_all_uids_resp_data *)resp_data;
			QByteArray data((char *)resp->data, resp->size);
			QByteArray mask((char *)resp->mask, resp->size);
			this_->signetdevReadAllUIdsResp(info, resp->uid, data, mask);
		} else {
			QByteArray data;
			QByteArray mask;
			this_->signetdevReadAllUIdsResp(info, -1, data, mask);
		}
	} break;
	case SIGNETDEV_CMD_GET_RAND_BITS: {
		signetdev_get_rand_bits_resp_data *resp = (signetdev_get_rand_bits_resp_data *)resp_data;
		QByteArray data((const char *)resp->data, resp->size);
		this_->signetdevGetRandBits(info, data);
	}
	break;
	case SIGNETDEV_CMD_READ_BLOCK: {
		QByteArray blk((const char *)resp_data, BLK_SIZE);
		this_->signetdevReadBlockResp(info, blk);
	}
	break;
	case SIGNETDEV_CMD_GET_PROGRESS: {
		signetdev_get_progress_resp_data *resp = (signetdev_get_progress_resp_data *)resp_data;
		this_->signetdevGetProgressResp(info, *resp);
	}
	break;
	case SIGNETDEV_CMD_STARTUP: {
		QByteArray hashfn;
		QByteArray salt;
		signetdev_startup_resp_data resp;
		if (resp_data && (resp_code == OKAY || resp_code == UNKNOWN_DB_FORMAT)) {
			signetdev_startup_resp_data *resp_ = (signetdev_startup_resp_data *)resp_data;
			resp = *resp_;
		}
		this_->signetdevStartupResp(info, resp);
	}
	break;
	case SIGNETDEV_CMD_READ_UID: {
		if (resp_data && resp_code == OKAY) {
			signetdev_read_uid_resp_data *resp = (signetdev_read_uid_resp_data *)resp_data;
			QByteArray data((char *)resp->data, resp->size);
			QByteArray mask((char *)resp->mask, resp->size);
			this_->signetdevReadUIdResp(info, data, mask);
		} else {
			QByteArray data;
			QByteArray mask;
			this_->signetdevReadUIdResp(info, data, mask);
		}
	}
	break;
	case SIGNETDEV_CMD_READ_CLEARTEXT_PASSWORD_NAMES: {
		QStringList l;
		QVector<int> f;
		if (resp_data && resp_code == OKAY) {
			for (int i = 0; i < NUM_CLEARTEXT_PASS; i++) {
				const u8 *data = ((const u8 *)resp_data) + i * (CLEARTEXT_PASS_NAME_SIZE + 1);
				u8 format = data[0];
				f.append(format);
				QString s = QString::fromUtf8(((const char *)data) + 1);
				l.push_back(s);
			}
		}
		this_->signetdevReadCleartextPasswordNames(info, f, l);
	}
	break;
	case SIGNETDEV_CMD_READ_CLEARTEXT_PASSWORD: {
		cleartext_pass resp;
		if (resp_code == OKAY && resp_data) {
			cleartext_pass *resp_ = (cleartext_pass *)resp_data;
			resp = *resp_;
		}
		this_->signetdevReadCleartextPassword(info, resp);
	}
	break;
	default:
		this_->signetdevCmdResp(info);
	}
}

void SignetApplication::init(bool startInTray, QString dbFilename)
{
	signetdev_initialize_api();
	signetdev_set_device_opened_cb(deviceOpenedS, this);
	signetdev_set_device_closed_cb(deviceClosedS, this);
	signetdev_set_command_resp_cb(commandRespS, this);
	signetdev_set_device_event_cb(deviceEventS, this);
	signetdev_set_error_handler(connectionErrorS, this);

#ifndef Q_OS_ANDROID
	m_dbFilename = dbFilename;
	m_main_window = new MainWindow(m_dbFilename);

	connect(this, SIGNAL(connectionError()), m_main_window, SLOT(connectionError()));

	QObject::connect(m_main_window, SIGNAL(destroyed(QObject*)), this, SLOT(mainDestroyed()));
	QObject::connect(this, SIGNAL(messageReceived(QString)), m_main_window, SLOT(messageReceived(QString)));
	QObject::connect(&m_systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			 this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
	QObject::connect(this, SIGNAL(signetdevEvent(int)), m_main_window, SLOT(signetDevEvent(int)));

	m_main_window->setWindowTitle("Signet");
	QIcon app_icon = QIcon(":/images/signet.png");
	m_main_window->setWindowIcon(app_icon);
	m_systray.setIcon(app_icon);
	m_systray.show();
	if (!startInTray) {
		m_main_window->show();
	}
#else
	m_signetDeviceManager = new SignetDeviceManager(m_qmlEngine, this);
	Q_UNUSED(startInTray);
	Q_UNUSED(dbFIlename);
#endif
}

QMessageBox *SignetApplication::messageBoxError(QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent)
{
	QMessageBox *box = new QMessageBox(icon, title, text, QMessageBox::Ok, parent);
	connect(box, SIGNAL(finished(int)), box, SLOT(deleteLater()));
	box->setWindowModality(Qt::WindowModal);
	box->show();
	return box;
}

QMessageBox *SignetApplication::messageBoxWarn(const QString &title, const QString &text, QWidget *parent)
{
	QMessageBox *box = new QMessageBox(QMessageBox::Warning, title, text, QMessageBox::Ok, parent);
	connect(box, SIGNAL(finished(int)), box, SLOT(deleteLater()));
	box->setWindowModality(Qt::WindowModal);
	box->show();
	return box;
}


SignetApplication::~SignetApplication()
{
#ifndef Q_OS_ANDROID
	m_systray.hide();
#endif
}

#ifndef Q_OS_ANDROID
void SignetApplication::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
	bool is_active = m_main_window->isActiveWindow();
	switch(reason) {
	case QSystemTrayIcon::Context: {
		QMenu *popup = new QMenu();
		QAction *open_action = popup->addAction("&Open");
		QAction *quit_action = popup->addAction("&Quit");
		connect(open_action, SIGNAL(triggered(bool)), m_main_window, SLOT(open()));
		connect(quit_action, SIGNAL(triggered(bool)), m_main_window, SLOT(quit()));
		popup->exec(QCursor::pos());
		popup->deleteLater();
	}
	break;
	case QSystemTrayIcon::Trigger:
		if (!is_active) {
			m_main_window->show();
			if (m_main_window->isMinimized()) {
				m_main_window->setWindowState((m_main_window->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
			}
			m_main_window->raise();
			m_main_window->activateWindow();
		} else {
			if (m_main_window->isMinimized()) {
				m_main_window->setWindowState((m_main_window->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
				m_main_window->raise();
				m_main_window->activateWindow();
			} else {
				m_main_window->setWindowState((m_main_window->windowState() & ~Qt::WindowActive) | Qt::WindowMinimized);
				m_main_window->hide();
			}
		}
		break;
	default:
		break;
	}
}
#endif
