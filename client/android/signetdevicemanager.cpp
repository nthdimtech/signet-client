#include "signetdevicemanager.h"
#include "signetapplication.h"
#include "keygeneratorthread.h"
#include "esdbgroupmodel.h"

#include <android/log.h>

#include <QQmlContext>
#include <QStringLiteral>
#include <QQuickView>
#include <QListView>
#include <QList>
#include <QStringList>
#include <QClipboard>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#include <QtAndroidExtras/QAndroidJniObject>

#include <algorithm>

class EsdbEntryModel : public QAbstractListModel
{
	QList<esdbEntry *> *m_entries;
public:
	EsdbEntryModel(QList<esdbEntry *> *entries);
	QVariant data(const QModelIndex &index, int role) const;
	int rowCount(const QModelIndex &parent) const;
public slots:
	QString text(int index);
};

EsdbEntryModel::EsdbEntryModel(QList<esdbEntry *> *entries) :
	m_entries(entries)
{
}

QVariant EsdbEntryModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		if (index.row() < m_entries->size()) {
			return m_entries->at(index.row())->getTitle();
		}
	}
	return QVariant();
}

int EsdbEntryModel::rowCount(const QModelIndex &parent) const
{
	return m_entries->length();
}

QString EsdbEntryModel::text(int index)
{
	if (index != -1) {
		return m_entries->at(index)->getTitle();
	} else {
		return QString();
	}
}

SignetDeviceManager::SignetDeviceManager(QQmlApplicationEngine &engine, QObject *parent) :
	QObject(parent),
	m_qmlEngine(engine)
{
	SignetApplication *app = SignetApplication::get();

	m_model = new EsdbEntryModel(&m_entriesFiltered);
	m_groupModel = new EsdbGroupModel(&m_groupsSorted);

	m_keyGeneratorThread = new KeyGeneratorThread();
	connect(m_keyGeneratorThread, SIGNAL(finished()), this, SLOT(keyGenerationFinished()));

	QObject *mainLoader = findQMLObject("mainLoader");
	connect(mainLoader, SIGNAL(loaded()), this, SLOT(loaded()));

	app->setAsyncListener(this);

	connect(app, SIGNAL(deviceOpened()), this, SLOT(deviceOpened()));
	connect(app, SIGNAL(deviceClosed()), this, SLOT(deviceClosed()));
	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this, SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	connect(app, SIGNAL(signetdevGetProgressResp(signetdevCmdRespInfo, signetdev_get_progress_resp_data)),
		this, SLOT(signetdevGetProgressResp(signetdevCmdRespInfo, signetdev_get_progress_resp_data)));
	connect(app, SIGNAL(signetdevStartupResp(signetdevCmdRespInfo, signetdev_startup_resp_data)),
		this, SLOT(signetdevStartupResp(signetdevCmdRespInfo, signetdev_startup_resp_data)));
	connect(app, SIGNAL(signetdevReadBlockResp(signetdevCmdRespInfo, QByteArray)),
		this, SLOT(signetdevReadBlockResp(signetdevCmdRespInfo, QByteArray)));
	connect(app, SIGNAL(signetdevReadAllUIdsResp(signetdevCmdRespInfo, int, QByteArray, QByteArray)),
		this, SLOT(signetdevReadAllUIdsResp(signetdevCmdRespInfo, int, QByteArray, QByteArray)));
	connect(app, SIGNAL(signetdevReadUIdResp(signetdevCmdRespInfo, QByteArray, QByteArray)),
		this, SLOT(signetdevReadUIdResp(signetdevCmdRespInfo, QByteArray, QByteArray)));
	connect(app, SIGNAL(signetdevEvent(int)), this, SLOT(signetdevEvent(int)));
	connect(app, SIGNAL(connectionError()), this, SLOT(connectionError()));

	QObject::connect(&m_connectingTimer, SIGNAL(timeout()), this, SLOT(connectingTimer()));

	enterDeviceState(SignetApplication::STATE_CONNECTING);
	int rc = signetdev_open_connection();
	if (!rc) {
		deviceOpened();
	}
}

void SignetDeviceManager::keyGenerationFinished()
{
	::signetdev_login(NULL, &m_signetdevCmdToken,
			  (u8 *)m_keyGeneratorThread->getKey().data(),
			  m_keyGeneratorThread->getKey().length(), 0);
}

void SignetDeviceManager::connectingTimer()
{
	setLoaderSource("no_device.qml");
}

void SignetDeviceManager::deviceOpened()
{
	m_connectingTimer.stop();
	::signetdev_startup(NULL, &m_signetdevCmdToken);
}

void SignetDeviceManager::connectionError()
{
	enterDeviceState(SignetApplication::STATE_CONNECTING);
	int rc = signetdev_open_connection();
	if (rc == 0) {
		deviceOpened();
	}
}

void SignetDeviceManager::deviceClosed()
{
}

QObject *SignetDeviceManager::findQMLObject(QString name)
{
	QObject *ret = NULL;
	for (auto o : m_qmlEngine.rootObjects()) {
		ret = o->findChild<QObject *>(name);
		if (ret)
			break;
	}
	return ret;
}

void SignetDeviceManager::filterEntries(QString groupName, QString search)
{
	m_entriesFiltered.clear();
	for (auto e : m_entries) {
		if (e->getPath() == groupName && e->getTitle().startsWith(search, Qt::CaseInsensitive)) {
			m_entriesFiltered.push_back(e);
		}
	}
	struct {
		bool operator() (esdbEntry *& i, esdbEntry *& j) {
			return (QString::compare(i->getTitle(), j->getTitle(), Qt::CaseInsensitive) < 0);
		}
	} entrySortOp;
	std::sort(m_entries.begin(), m_entries.end(), entrySortOp);
	m_model->layoutChanged();
}

void SignetDeviceManager::setLoaderSource(QString str)
{
	QObject *mainLoader = findQMLObject("mainLoader");
	if (mainLoader) {
		mainLoader->setProperty("source", QUrl("qrc:/" + str));
	}
}

void SignetDeviceManager::loaded()
{
	switch (m_deviceState) {
	case SignetApplication::STATE_LOGGED_OUT: {
		QObject *loader = findQMLObject("mainLoader");
		QObject *loginComponent = loader->findChild<QObject *>("loginComponent");
		if (loginComponent) {
			connect(loginComponent, SIGNAL(loginSignal(QString)), this, SLOT(loginSignal(QString)));
			connect(this, SIGNAL(badPasswordEntered()), loginComponent, SLOT(badPasswordEntered()));
		}
		} break;
	case SignetApplication::STATE_LOGGED_IN_LOADING_ACCOUNTS: {
		QObject *loader = findQMLObject("mainLoader");
		m_loadingProgress = loader->findChild<QObject *>("loadingProgress");
		m_entriesLoaded = 0;
		::signetdev_read_all_uids(NULL, &m_signetdevCmdToken, 1);
		} break;
	case SignetApplication::STATE_LOGGED_IN: {
		QObject *loader = findQMLObject("mainLoader");
		QObject *unlockedItem = loader->findChild<QObject *>("unlockedItem");
		if (unlockedItem) {
			connect(unlockedItem, SIGNAL(lockSignal()), this, SLOT(lockSignal()));
			connect(unlockedItem, SIGNAL(filterTextChangedSignal(QString)), this, SLOT(filterTextChangedSignal(QString)));
			connect(unlockedItem, SIGNAL(filterGroupChangedSignal(int)), this, SLOT(filterGroupChangedSignal(int)));
			connect(unlockedItem, SIGNAL(copyUsernameSignal(int)), this, SLOT(copyUsernameSignal(int)));
			connect(unlockedItem, SIGNAL(copyPasswordSignal(int)), this, SLOT(copyPasswordSignal(int)));
		}

		} break;
	default:
		break;
	}
}

void SignetDeviceManager::loginSignal(QString password)
{
	__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "Login password: %s", password.toLatin1().data());
	SignetApplication *app = SignetApplication::get();
	m_keyGeneratorThread->setParams(password, app->getHashfn(), app->getSalt(), app->getKeyLength());
	m_keyGeneratorThread->start();
}

void SignetDeviceManager::lockSignal()
{
	__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "Lock");
	::signetdev_logout(NULL, &m_signetdevCmdToken);
}

void SignetDeviceManager::filterTextChangedSignal(QString text)
{
	m_filterEntry = text;
	filterEntries(m_filterGroup, m_filterEntry);
}

void SignetDeviceManager::filterGroupChangedSignal(int index)
{
	if (index == -1) {
		m_filterGroup = QString();
	} else {
		m_filterGroup = m_groupsSorted.at(index);
	}
	if (m_filterGroup == QString("Unsorted")) {
		m_filterGroup = QString();
	}
	filterEntries(m_filterGroup, m_filterEntry);
}

void SignetDeviceManager::copyUsernameSignal(int index)
{
	if (index >= 0) {
		account *a = (account *)m_entriesFiltered.at(index);
		QClipboard *clipboard = SignetApplication::clipboard();
		clipboard->setText(a->userName);
	}
}

void SignetDeviceManager::copyPasswordSignal(int index)
{
	if (index >= 0) {
		account *a = (account *)m_entriesFiltered.at(index);
		::signetdev_read_uid(NULL, &m_signetdevCmdToken, a->id, 0);
	}
}

void SignetDeviceManager::enterDeviceState(int state)
{
	if (m_deviceState == SignetApplication::STATE_LOGGED_IN &&
			state != SignetApplication::STATE_LOGGED_IN) {
		m_groups.clear();
		m_groupsSorted.clear();
		m_entriesFiltered.clear();
		m_model->layoutChanged();
		for (auto e : m_entries) {
			delete e;
		}
		m_entries.clear();
	}
	m_deviceState = (SignetApplication::device_state)(state);
	__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "MainWindow(): Device state %d", state);

	switch (m_deviceState) {
	case SignetApplication::STATE_CONNECTING: {
		m_connectingTimer.setSingleShot(true);
		m_connectingTimer.setInterval(1000);
		m_connectingTimer.start();
		setLoaderSource("connecting.qml");
	}
	break;
	case SignetApplication::STATE_LOGGED_OUT: {
		setLoaderSource("login.qml");
	} break;
	case SignetApplication::STATE_LOGGED_IN_LOADING_ACCOUNTS: {
		setLoaderSource("loading_entries.qml");
	}
	break;
	case SignetApplication::STATE_LOGGED_IN: {
		m_qmlEngine.rootContext()->setContextProperty("entryModel", m_model);
		m_qmlEngine.rootContext()->setContextProperty("groupModel", m_groupModel);
		setLoaderSource("unlocked.qml");
	}
	break;
	default:
		break;
	}
}

void SignetDeviceManager::signetdevStartupResp(signetdevCmdRespInfo info, signetdev_startup_resp_data resp)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	int code = info.resp_code;
	SignetApplication *app = SignetApplication::get();
	QByteArray salt;
	int keyLength;
	int saltLength;
	int root_block_format = resp.root_block_format;
	int device_state = resp.device_state;
	int db_format = resp.db_format;
	if (root_block_format >= 2) {
		saltLength = AES_256_KEY_SIZE;
		keyLength = AES_256_KEY_SIZE;
	}
	salt = QByteArray((const char *)resp.salt, saltLength);
	app->setSaltLength(saltLength);
	app->setSalt(salt);
	app->setHashfn(QByteArray((const char *)resp.hashfn, HASH_FN_SZ));
	app->setKeyLength(keyLength);
	app->setDBFormat(db_format);
	app->setConnectedFirmwareVersion(resp.fw_major_version, resp.fw_minor_version, resp.fw_step_version);

	switch (code) {
	case UNKNOWN_DB_FORMAT:
		enterDeviceState(SignetApplication::STATE_UNINITIALIZED);
		break;
	case OKAY:
		switch (device_state) {
		case LOGGED_OUT:
			enterDeviceState(SignetApplication::STATE_LOGGED_OUT);
			setLoaderSource("login.qml");
			break;
		case UNINITIALIZED:
			enterDeviceState(SignetApplication::STATE_UNINITIALIZED);
			break;
		}
		break;
		return;
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		return;
	default:
		abort();
		return;
	}
}

void  SignetDeviceManager::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	bool do_abort = false;
	int code = info.resp_code;

	switch (code) {
	case OKAY:
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case BAD_PASSWORD:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default:
		do_abort = true;
		return;
	}
	switch (info.cmd) {
	case SIGNETDEV_CMD_LOGIN:
		if (info.resp_code == BAD_PASSWORD) {
			badPasswordEntered();
		} else if (info.resp_code == OKAY){
			enterDeviceState(SignetApplication::STATE_LOGGED_IN_LOADING_ACCOUNTS);
		} else {
			__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "Unknown error");
		}
		break;
	case SIGNETDEV_CMD_LOGOUT:
		if (info.resp_code == OKAY) {
			enterDeviceState(SignetApplication::STATE_LOGGED_OUT);
		}
		break;
	default:
		break;
	}
	if (do_abort) {
		abort();
	}
}

void SignetDeviceManager::signetdevEventAsync(int eventType)
{
	__android_log_print(ANDROID_LOG_DEBUG, "SIGNET_ACTIVITY", "Button Event");
	QAndroidJniObject::callStaticMethod<void>("com/nthdimtech/SignetService", "foregroundActivity");
}

void SignetDeviceManager::signetdevEvent(int eventType)
{
}

void SignetDeviceManager::signetdevGetProgressResp(signetdevCmdRespInfo info, signetdev_get_progress_resp_data data)
{

}

void SignetDeviceManager::signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	block *b = new block();
	b->data = data;
	b->mask = mask;
	esdbEntry_1 tmp(id);
	tmp.fromBlock(b);
	m_entriesLoaded++;
	if (m_loadingProgress) {
		m_loadingProgress->setProperty("from", QVariant(0));
		m_loadingProgress->setProperty("to", QVariant(info.messages_remaining + m_entriesLoaded));
		m_loadingProgress->setProperty("value", QVariant(m_entriesLoaded));
	}
	if (tmp.type == ESDB_TYPE_ACCOUNT) {
		esdbEntry *entry = m_acctTypeModule.decodeEntry(id, tmp.revision, NULL, b);
		if (entry) {
			m_entries.push_back(entry);
		}
		QString group = entry->getPath();
		if (!group.size()) {
			group = "Unsorted";
		}
		if (!m_groups.contains(group)) {
			m_groups.insert(group);
			m_groupsSorted.append(group);
		}
	}
	if (!info.messages_remaining) {
		struct {
			bool operator() (const QString &i, const QString &j) {
				return (QString::compare(i, j, Qt::CaseInsensitive) < 0);
			}
		} groupSortOp;
		std::sort(m_groupsSorted.begin(), m_groupsSorted.end(), groupSortOp);
		m_groupModel->layoutChanged();
		filterEntries("","");
		enterDeviceState(SignetApplication::STATE_LOGGED_IN);
	}
}

void SignetDeviceManager::signetdevReadBlockResp(signetdevCmdRespInfo info, QByteArray block)
{
	Q_UNUSED(info);
	Q_UNUSED(block);
}

void SignetDeviceManager::signetdevReadUIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask)
{
	if (info.resp_code == OKAY) {
		block *b = new block();
		b->data = data;
		b->mask = mask;
		esdbEntry_1 tmp(-1);
		tmp.fromBlock(b);
		QClipboard *clipboard = SignetApplication::get()->clipboard();
		esdbEntry *entry = m_acctTypeModule.decodeEntry(-1, tmp.revision, NULL, b);
		if (entry) {
			account *a = (account *)entry;
			clipboard->setText(a->password);
			delete entry;
		}
	}
}
