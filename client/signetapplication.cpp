#include "signetapplication.h"

#ifndef Q_OS_ANDROID
#include "desktop/mainwindow.h"
#ifdef WITH_BROWSER_PLUGINS
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#endif
#else
#include "android/signetdevicemanager.h"
#endif

#include <QMenu>
#include <QDesktopWidget>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>

extern "C" {
#include "signetdev/host/signetdev.h"
#include "crypto_scrypt.h"
};

#include "systemtray.h"

#include <websockethandler.h>

#define DEFAULT_SCRYPT_N_VALUE_LOG2 12
#define DEFAULT_SCRYPT_N_VALUE (1<<DEFAULT_SCRYPT_N_VALUE_LOG2)
#define DEFAULT_SCRYPT_R_VALUE 32
#define DEFAULT_SCRIPT_P_VALUE 1

SignetApplication *SignetApplication::g_singleton = nullptr;

void SignetApplication::deviceOpenedS(enum signetdev_device_type dev_type, void *this_)
{
	((SignetApplication *)this_)->deviceOpened(dev_type);
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
	QtSingleApplication("signetdev-1209-df11", argc, argv),
#endif
	m_signetAsyncListener(nullptr),
	m_bootMode(HC_BOOT_UNKNOWN_MODE)
{
#ifdef WITH_BROWSER_PLUGINS
	m_nextSocketId = 0;
    m_webSocketServer = nullptr;
#endif
	setStyleSheet("QPushButton {qproperty-iconSize: 32px;}\n QAbstractItemView {qproperty-iconSize: 32px;}");
	g_singleton = this;
	m_systray = new SystemTray();
	qRegisterMetaType<signetdevCmdRespInfo>();
	qRegisterMetaType<signetdev_startup_resp_data>();
	qRegisterMetaType<signetdev_get_progress_resp_data>();
	qRegisterMetaType<cleartext_pass>();
	qRegisterMetaType<QVector<int> >("QVector<int>");
	qRegisterMetaType<enum signetdev_device_type>();
}

void SignetApplication::generateScryptKey(const QString &password, QByteArray &key, const QByteArray &salt, unsigned int N, unsigned int r, unsigned int s)
{
	QByteArray password_utf8 = password.toUtf8();
	crypto_scrypt((u8 *)password_utf8.data(), password_utf8.size(),
		      (u8 *)salt.data(), salt.size(),
		      N, r, s,
		      (u8 *)key.data(), key.size());

}

#ifdef WITH_BROWSER_PLUGINS
void SignetApplication::websocketResponse(int socketId, const QString &response)
{
	Q_UNUSED(socketId);
	for (auto socketHandler : m_openWebSockets) {
		if (socketHandler->id() == socketId) {
			socketHandler->websocketResponse(response);
		}
	}
}
#endif

void SignetApplication::getFirmwareVersion(int &major, int &minor, int &step)
{
	switch (m_deviceType) {
	case SIGNETDEV_DEVICE_HC:
		major = 0;
		minor = 1;
		step = 2;
		break;
	case SIGNETDEV_DEVICE_ORIGINAL:
		major = 1;
		minor = 3;
		step = 4;
		break;
	default:
		major = 0;
		minor = 0;
		step = 0;
		break;
	}
}

void SignetApplication::generateKey(const QString &password, QByteArray &key, const QByteArray &hashfn, const QByteArray &salt, int keyLength)
{
	QByteArray s = password.toUtf8();
	key.resize(keyLength);
	memset(key.data(), 0, (size_t)key.length());

	int fn = hashfn.at(0);

	switch(fn) {
	case 0: {
		unsigned int N = DEFAULT_SCRYPT_N_VALUE;
		unsigned int r = DEFAULT_SCRYPT_R_VALUE;
		unsigned int p = DEFAULT_SCRIPT_P_VALUE;
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

void SignetApplication::deviceEventS(void *cb_param, int event_type, const void *data, int data_len)
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

void SignetApplication::commandRespS(void *cb_param, void *cmd_user_param, int cmd_token, int cmd, int end_device_state, int messages_remaining, int resp_code, const void *resp_data)
{
	signetdevCmdRespInfo info;
	info.param = cmd_user_param;
	info.cmd = cmd;
	info.token = cmd_token;
	info.resp_code = resp_code;
	info.end_device_state = end_device_state;
	info.messages_remaining = messages_remaining;

	SignetApplication *this_ = static_cast<SignetApplication *>(cb_param);
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
	}
	break;
	case SIGNETDEV_CMD_GET_RAND_BITS: {
		signetdev_get_rand_bits_resp_data *resp = (signetdev_get_rand_bits_resp_data *)resp_data;
		QByteArray data((const char *)resp->data, resp->size);
		this_->signetdevGetRandBits(info, data);
	}
	break;
	case SIGNETDEV_CMD_READ_BLOCK: {
		QByteArray blk((const char *)resp_data, ::signetdev_device_block_size());
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
		break;
	}
}

void SignetApplication::startWebsocketServer()
{
#ifndef Q_OS_ANDROID
#ifdef WITH_BROWSER_PLUGINS
    if (m_webSocketServer == nullptr) {
        m_webSocketServer = new QWebSocketServer("Signet", QWebSocketServer::NonSecureMode, this);
        m_webSocketServer->listen(QHostAddress::LocalHost, 10910);
        connect(m_webSocketServer, SIGNAL(newConnection()), this, SLOT(newWebSocketConnection()));
    }
#endif
#endif
}

void SignetApplication::stopWebsocketServer()
{
#ifndef Q_OS_ANDROID
#ifdef WITH_BROWSER_PLUGINS
    if (m_webSocketServer) {
        m_webSocketServer->close();
        for (auto s : m_openWebSockets) {
            s->disconnect();
            s->deleteLater();
        }
        m_webSocketServer->deleteLater();
        m_webSocketServer = nullptr;
    }
#endif
#endif
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

	QObject::connect(this, SIGNAL(messageReceived(QString)), m_main_window, SLOT(messageReceived(QString)));
	QObject::connect(m_systray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			 this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
	QObject::connect(this, SIGNAL(signetdevEvent(int)), m_main_window, SLOT(signetDevEvent(int)));

	m_main_window->setWindowTitle("Signet");
	QIcon app_icon = QIcon(":/images/signet.png");
	m_main_window->setWindowIcon(app_icon);
	m_systray->setIcon(app_icon);
	m_systray->show();
	if (!startInTray) {
		m_main_window->show();
	}
#else
	m_signetDeviceManager = new SignetDeviceManager(m_qmlEngine, this);
	Q_UNUSED(startInTray);
	Q_UNUSED(dbFilename);
#endif
}

bool SignetApplication::isDeviceEmulated()
{
	return m_main_window && m_main_window->getDatabaseFileName().size();
}

QMessageBox *SignetApplication::messageBoxError(QMessageBox::Icon icon, const QString &title, const QString &text, QWidget *parent)
{
	QMessageBox *box = new QMessageBox(icon, title, text, QMessageBox::Ok, parent);
	box->setWindowModality(Qt::WindowModal);
	box->setAttribute(Qt::WA_DeleteOnClose);
	box->show();
	return box;
}

QMessageBox *SignetApplication::messageBoxWarn(const QString &title, const QString &text, QWidget *parent)
{
	QMessageBox *box = new QMessageBox(QMessageBox::Warning, title, text, QMessageBox::Ok, parent);
	box->setWindowModality(Qt::WindowModal);
	box->setAttribute(Qt::WA_DeleteOnClose);
	box->show();
	return box;
}


SignetApplication::~SignetApplication()
{
#ifndef Q_OS_ANDROID
	if (m_systray)
		delete m_systray;
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

#ifdef WITH_BROWSER_PLUGINS
#include "websockethandler.h"

void SignetApplication::newWebSocketConnection()
{
	while (true) {
		QWebSocket *nextConnection = m_webSocketServer->nextPendingConnection();
		if (!nextConnection)
			break;
		bool acceptConnection = false;
		if (m_webSocketOriginWhitelist.contains(nextConnection->origin()) || nextConnection->origin().startsWith(QString("moz-extension://")) ||
		    nextConnection->origin().startsWith(QString("chrome-extension://"))) {
			if (m_openWebSockets.size() < s_maxWebSocketConnections) {
				acceptConnection = true;
			}
		}

		if (acceptConnection) {
			auto *handler = new websocketHandler(nextConnection, m_nextSocketId++, this);
			m_openWebSockets.append(handler);
			connect(handler, SIGNAL(done(websocketHandler *)), this, SLOT(websocketHandlerDone(websocketHandler *)));
			connect(handler, SIGNAL(websocketMessage(int, QString)), this, SIGNAL(websocketMessage(int, QString)));
			connect(handler, SIGNAL(websocketMessage(int, QString)), this, SLOT(websocketMessage_(int, QString)));
		} else {
			nextConnection->close();
			nextConnection->deleteLater();
		}
	}
}

void SignetApplication::websocketMessage_(int id, QString message)
{
	Q_UNUSED(id);
	auto document = QJsonDocument::fromJson(message.toUtf8());
	if (document.isObject()) {
		auto obj = document.object();
		QString msgType = obj["messageType"].toString();
		if (msgType == "show" && m_main_window) {
			m_main_window->open();
		}
	}
}

void SignetApplication::websocketHandlerDone(websocketHandler *handler)
{
	m_openWebSockets.removeOne(handler);
	handler->deleteLater();
}
#endif

#endif
