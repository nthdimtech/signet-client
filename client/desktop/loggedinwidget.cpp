#include "loggedinwidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QListView>
#include <QLineEdit>
#include <QDir>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QGroupBox>
#include <QComboBox>
#include <QFrame>
#include <QApplication>
#include <QUrl>
#include <QDesktopServices>
#include <QRadioButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <QStringList>
#include <QDebug>

#include "esdb.h"
#include "esdbmodel.h"
#include "aspectratiopixmaplabel.h"
#include "editaccount.h"
#include "searchlistbox.h"
#include "searchfilteredit.h"
#include "buttonwaitdialog.h"
#include "signetapplication.h"
#include "esdbactionbar.h"
#include "generictypedesc.h"
#include "esdbgenericmodule.h"
#include "generictype/esdbgenerictypemodule.h"
#include "generictype/generictypeactionbar.h"
#include "genericactionbar.h"
#include "bookmarkactionbar.h"
#include "accountactionbar.h"
#include "generictypedesc.h"

extern "C" {
#include "signetdev/host/signetdev.h"
};

#include "../desktop/mainwindow.h"

#include "bookmark.h"
#include "generic.h"
#include "datatypelistmodel.h"

int iconAccount::matchQuality(esdbEntry *entry)
{
	int quality = 0;
	QString acct_name = entry->getTitle();
	QString acct_url = entry->getUrl();
	if (!QString::compare(acct_name, name, Qt::CaseInsensitive)) {
		quality += 2;
	}

	if (acct_name.startsWith(name, Qt::CaseInsensitive)) {
		quality++;
	}
	QUrl acctNameUrl(acct_name);
	QUrl acctUrl(acct_url);

	QUrl *compUrl = nullptr;
	if (acctUrl.isValid()) {
		compUrl = &acctUrl;
	}
	if (acctNameUrl.isValid()) {
		compUrl = &acctNameUrl;
	}

	if (compUrl && urlObj.isValid()) {
		QString iconUrlPath = urlObj.path();
		QStringList l = compUrl->path().split('/');
		if (l.length()) {
			QString &domain = l.first();
			if (domain.endsWith(iconUrlPath)) {
				quality += 2;
			}
		}
	}
	return quality;
}

LoggedInWidget::typeData::typeData(esdbTypeModule *_module) :
	module(_module),
	entries(nullptr),
	actionBar(nullptr),
	filteredList(nullptr),
	model(nullptr),
	expanded(false)
{
	entries = new QMap<int, esdbEntry *>();
	filteredList = new QList<esdbEntry *>();
	model = new EsdbModel(module, filteredList);
}

LoggedInWidget::typeData::~typeData()
{
	delete entries;
	delete filteredList;
	delete model;
	delete module;
	actionBar->deleteLater();
}

void LoggedInWidget::getCurrentGroups(QString typeName, QStringList &groups)
{
    auto entryMap = typeNameToEntryMap(typeName);
    if (entryMap) {
        for (auto entry : *entryMap) {
            if (!groups.contains(entry->getPath())) {
                groups.append(entry->getPath());
            }
        }
    }
}

LoggedInWidget::LoggedInWidget(QProgressBar *loading_progress, MainWindow *mw, QWidget *parent) : QWidget(parent),
	m_activeType(nullptr),
	m_selectedEntry(nullptr),
	m_filterLabel(nullptr),
	m_newAcctButton(nullptr),
	m_populating(true),
	m_populatingCantRead(0),
	m_searchListbox(nullptr),
	m_loadingProgress(loading_progress),
	m_filterEdit(nullptr),
	m_accountGroup(nullptr),
	m_signetdevCmdToken(-1),
	m_id(-1),
    m_idTask(ID_TASK_NONE)
{
	m_genericIcon = QIcon(":images/generic-entry.png");
	m_icon_accounts.append(
	    iconAccount("facebook")
	);
	m_icon_accounts.append(
	    iconAccount("kickstarter")
	);
	m_icon_accounts.append(
	    iconAccount("twitter")
	);
	m_icon_accounts.append(
	    iconAccount("linkedin")
	);
	m_icon_accounts.append(
	    iconAccount("patreon")
	);
	m_icon_accounts.append(
	    iconAccount("gmail")
	);
	m_icon_accounts.append(
	    iconAccount("github")
	);
	m_icon_accounts.append(
	    iconAccount("paypal")
	);
	m_icon_accounts.append(
	    iconAccount("apple")
	);
	m_icon_accounts.append(
	    iconAccount("macrofab")
	);
	m_icon_accounts.append(
	    iconAccount("fandango")
	);
	m_icon_accounts.append(
	    iconAccount("indiegogo")
	);
	m_icon_accounts.append(
	    iconAccount("slack")
	);
	m_icon_accounts.append(
	    iconAccount("qt","qt.io","qt.png")
	);
	m_icon_accounts.append(
	    iconAccount("instagram")
	);
	m_icon_accounts.append(
	    iconAccount("Crowd Supply", "crowdsupply.com", "crowdsupply.png")
	);
	m_icon_accounts.append(
	    iconAccount("chase","chase.com", "chase_bank.png")
	);
	m_icon_accounts.append(
	    iconAccount("dropbox")
	);
	m_icon_accounts.append(
	    iconAccount("tumblr")
	);
	m_icon_accounts.append(
	    iconAccount("steam")
	);
	m_icon_accounts.append(
	    iconAccount("amazon")
	);
	m_icon_accounts.append(
	    iconAccount("microsoft")
	);
	m_icon_accounts.append(
	    iconAccount("reddit")
	);

	m_activeTypeIndex = 0;

	m_dataTypesModel = new DataTypeListModel();

	bool fromFile = mw->getDatabaseFileName().size();

	genericTypeDesc *miscTypeDesc = new genericTypeDesc(-1);
	miscTypeDesc->name = "Misc";
	miscTypeDesc->typeId = 0;
	m_genericModules.push_back(new esdbGenericModule(miscTypeDesc));

	m_writeEnabled = !fromFile;
	m_typeEnabled = !fromFile;

	m_accounts = new esdbAccountModule();
	typeData *accountsTypeData = new typeData(m_accounts);
	accountsTypeData->actionBar = new AccountActionBar(this, m_writeEnabled, m_typeEnabled);
	m_typeData.push_back(accountsTypeData);
	m_dataTypesModel->addModule(m_accounts, true);

	m_bookmarks = new esdbBookmarkModule();
	typeData *bookmarksTypeData = new typeData(m_bookmarks);
	bookmarksTypeData->actionBar = new BookmarkActionBar(m_bookmarks, this, m_writeEnabled, m_typeEnabled);
	m_typeData.push_back(bookmarksTypeData);
	m_dataTypesModel->addModule(m_bookmarks, true);

	m_miscTypeData = new typeData(m_genericModules[0]);
	m_miscTypeData->actionBar = new GenericActionBar(this, m_miscTypeData->module, miscTypeDesc, m_writeEnabled, m_typeEnabled);
	m_typeData.push_back(m_miscTypeData);
	m_dataTypesModel->addModule(m_miscTypeData->module, true);

	m_genericTypeModule = new esdbGenericTypeModule();
	typeData *genericTypeData = new typeData(m_genericTypeModule);
	genericTypeData->actionBar = new GenericTypeActionBar(this, genericTypeData->module, m_writeEnabled, m_typeEnabled);
	m_typeData.push_back(genericTypeData);
	m_dataTypesModel->addModule(m_genericTypeModule, true);

	m_activeType = m_typeData.at(m_activeTypeIndex);

	SignetApplication *app = SignetApplication::get();
    connect(app, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)));
    connect(app, SIGNAL(selectUrl(QString)), this, SLOT(selectUrl(QString)));

	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)), this,
		SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	connect(app, SIGNAL(signetdevReadUIdResp(signetdevCmdRespInfo, QByteArray, QByteArray)), this,
		SLOT(signetdevReadUIdResp(signetdevCmdRespInfo, QByteArray, QByteArray)));

	connect(app, SIGNAL(signetdevReadAllUIdsResp(signetdevCmdRespInfo, int, QByteArray, QByteArray)), this,
		SLOT(signetdevReadAllUIdsResp(signetdevCmdRespInfo, int, QByteArray, QByteArray)));

	m_filterEdit = new SearchFilterEdit();
	QObject::connect(m_filterEdit, SIGNAL(textEdited(QString)),
			 this, SLOT(filterTextEdited(QString)));
	QObject::connect(m_filterEdit, SIGNAL(returnPressed()),
			 this, SLOT(filterEditPressed()));

	QIcon plus_icn = QIcon(":/images/plus.png");

	m_newAcctButton = new QPushButton(plus_icn, "");
	m_newAcctButton->setAutoDefault(true);
	m_newAcctButton->setToolTip("New account");
	QObject::connect(m_newAcctButton, SIGNAL(pressed()), this, SLOT(newEntryUI()));

	m_actionBarStack = new QStackedWidget();
	for (auto iter : m_typeData) {
		connect(iter->actionBar, SIGNAL(background()), this, SIGNAL(background()));
		connect(iter->actionBar, SIGNAL(abort()), this, SIGNAL(abort()));
		m_actionBarStack->addWidget(iter->actionBar);
	}

	m_filterLabel = new AspectRatioPixmapLabel();
	QIcon icn = QIcon(":/images/search.png");
	QPixmap pm = icn.pixmap(QSize(128,128));
	m_filterLabel->setPixmap(pm);
	m_filterLabel->setToolTip("Account search");

	m_searchListbox = new SearchListbox(m_filterEdit, m_activeType->model);
	m_searchListbox->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_searchListbox, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
	connect(m_searchListbox, SIGNAL(pressed(QModelIndex)), this, SLOT(pressed(QModelIndex)));
	connect(m_searchListbox, SIGNAL(selected(QModelIndex)), this, SLOT(selected(QModelIndex)));
	connect(m_searchListbox, SIGNAL(filterTextChanged(QString)), this, SLOT(filterTextChanged(QString)));
	connect(m_searchListbox, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

	m_filterEdit->setSearchListbox(m_searchListbox);

	QBoxLayout *filter_layout = new QBoxLayout(QBoxLayout::LeftToRight);
	filter_layout->addWidget(m_filterLabel);
	filter_layout->addWidget(m_filterEdit);
	filter_layout->addWidget(m_newAcctButton);
	m_filterLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_filterEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
	m_newAcctButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_viewSelector = new QComboBox();
	m_viewSelector->setModel(m_dataTypesModel);
	m_viewSelector->setCurrentIndex(m_activeTypeIndex);
	connect(m_viewSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(currentTypeIndexChanged(int)));

	QBoxLayout *top_layout = new QBoxLayout(QBoxLayout::TopToBottom);
	top_layout->setAlignment(Qt::AlignTop);

	if (mw->getDatabaseFileName().size()) {
		QLabel *l = new QLabel();
		l->setText("Database file: " + mw->getDatabaseFileName());
		QFrame *f = new QFrame();
		f->setFrameStyle(QFrame::HLine);
		top_layout->addWidget(l);
		top_layout->addWidget(f);
	}

	top_layout->addLayout(filter_layout);
	top_layout->addWidget(m_viewSelector);
	top_layout->addWidget(m_searchListbox);
	top_layout->addWidget(m_actionBarStack);
	top_layout->setAlignment(m_actionBarStack, Qt::AlignBottom);

	setLayout(top_layout);

	selectEntry(nullptr);

	connect(m_searchListbox, SIGNAL(expanded(QModelIndex)),
		this, SLOT(expanded(QModelIndex)));
	connect(m_searchListbox, SIGNAL(collapsed(QModelIndex)),
		this, SLOT(collapsed(QModelIndex)));

	m_searchListbox->setEnabled(false);
	m_filterEdit->setEnabled(false);
	m_newAcctButton->setEnabled(false);
	m_filterLabel->setEnabled(false);
	m_populating = true;
	m_populatingCantRead = 0;
	m_loadingProgress->setMinimum(0);
	m_loadingProgress->setMaximum(1);

	::signetdev_read_all_uids(nullptr, &m_signetdevCmdToken, 1);
}

void LoggedInWidget::signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int uid, QByteArray data, QByteArray mask)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	bool do_abort = false;
	int code = info.resp_code;

	switch (code) {
	case OKAY:
	case ID_INVALID:
	case BUTTON_PRESS_CANCELED:
	case BUTTON_PRESS_TIMEOUT:
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default:
		do_abort = true;
		return;
	}

	if (code == OKAY && uid != -1) {
		block *b = new block();
		b->data = data;
		b->mask = mask;
		getEntryDone(uid, code, b, false);
	}

	if (m_loadingProgress->maximum() == 1) {
		m_loadingProgress->setMinimum(0);
		m_loadingProgress->setMaximum(info.messages_remaining);
	}
	m_loadingProgress->setValue(m_loadingProgress->maximum() - info.messages_remaining);
	if (!info.messages_remaining) {
		if (m_populatingCantRead) {
			SignetApplication::messageBoxError(QMessageBox::Warning,
							   "Unlocking",
							   QString::number(m_populatingCantRead) +
							   " entries could not be read because they were written by a newer client version."
							   " You must upgrade your client to access all of your data",
							   this);
		}
		m_populatingCantRead = 0;
		m_populating = false;
		for (auto entry : m_entries) {
			int index = entryToIndex(entry);
			if (index >= 0) {
				m_typeData.at(index)->entries->insert(entry->id, entry);
			} else {
				//TODO
			}
		}
		m_searchListbox->setEnabled(true);
		m_filterEdit->setEnabled(true);
		m_newAcctButton->setEnabled(true);
		m_filterLabel->setEnabled(true);
		populateEntryList(m_activeType, m_searchListbox->filterText());
		emit enterDeviceState(SignetApplication::STATE_LOGGED_IN);
		m_signetdevCmdToken = -1;
	}

	if (do_abort) {
		abort();
	}
}

void LoggedInWidget::expandTreeItems(QModelIndex parent)
{
	for (int i = 0; i < m_activeType->model->rowCount(parent); i++) {
		QModelIndex child = m_activeType->model->index(i, 0, parent);
		if (!(static_cast<EsdbModelItem *>(child.internalPointer())->isLeafItem())) {
			m_searchListbox->setExpanded(child, true);
			expandTreeItems(child);
		}
	}
}

void LoggedInWidget::expandTreeItems()
{
	expandTreeItems(QModelIndex());
	m_activeType->expanded = true;
}

void LoggedInWidget::currentTypeIndexChanged(int idx)
{
	if (idx < m_typeData.size()) {
		m_activeTypeIndex = idx;
		m_activeType = m_typeData.at(m_activeTypeIndex);
		m_searchListbox->setModel(m_activeType->model);

		m_selectedEntry = nullptr;
		m_actionBarStack->setCurrentIndex(idx);
		populateEntryList(m_activeType, m_filterEdit->text());
		m_filterEdit->setFocus();
	}
}

LoggedInWidget::~LoggedInWidget()
{
	if (m_accounts)
		delete m_accounts;
}

void LoggedInWidget::open()
{
	m_filterEdit->setFocus();
}

void LoggedInWidget::signetDevEvent(int code)
{
	switch (code) {
	case 1: //TODO: fix magic number
		open();
		break;
	}
}

void LoggedInWidget::expanded(QModelIndex index)
{
	if (m_activeType)
		m_activeType->model->expand(index, true);
	if (!m_selectedEntry && m_searchListbox->filterText().size()) {
		selectFirstVisible();
	}
}

void LoggedInWidget::collapsed(QModelIndex index)
{
	if (m_activeType)
        m_activeType->model->expand(index, false);
}

int LoggedInWidget::scoreUrlMatch(const QUrl &a, const QUrl &b)
{
    int score = 0;
    QStringList urlHostAparts = a.host().split(".");
    QStringList urlHostBparts = b.host().split(".");

    int hostMatches = 0;
    for (int j = 0; j < std::min(urlHostAparts.size(), urlHostBparts.size()); j++) {
        QString partA = urlHostAparts[urlHostAparts.size() - j - 1];
        QString partB = urlHostBparts[urlHostBparts.size() - j - 1];
        if (!QString::compare(partA, partB, Qt::CaseInsensitive)) {
            hostMatches++;
        } else if (j == 1) {
            hostMatches = 0;
            break;
        } else if (j > 2){
            break;
        }
    }

    if (hostMatches) {
        score += hostMatches;
        //TODO: give some credit for partial path matches
        if (!QString::compare(a.path(), b.path(), Qt::CaseInsensitive)) {
            score++;
            if (!QString::compare(a.scheme(), b.scheme(), Qt::CaseInsensitive)) {
                score++;
            }
        }
    }
    return score;
}

void LoggedInWidget::selectUrl(QString url)
{
    QUrl selectedUrl(url, QUrl::TolerantMode);
    qDebug() << "selectUrl" << url;
    esdbEntry *highestScoreMatch = nullptr;
    int highestScore = 0;
    for (auto entry : *m_activeType->entries) {
        QString entUrlStr = entry->getUrl();
        QUrl entryUrl(entUrlStr, QUrl::TolerantMode);
        if (!entryUrl.scheme().size()) {
            entUrlStr = "http://" + entUrlStr;
            entryUrl.setUrl(entUrlStr);
        }
        if (!entryUrl.isValid()) {
            continue;
        }

        int score = scoreUrlMatch(selectedUrl, entryUrl);
        if (score > highestScore) {
            highestScoreMatch = entry;
            highestScore = score;
        }
    }
    if (highestScoreMatch) {
        qDebug() << "Selecting :" << highestScoreMatch->getTitle();
        QModelIndex idx = m_activeType->model->findEntry(highestScoreMatch);
        m_searchListbox->setCurrentIndex(idx);
        m_searchListbox->scrollTo(idx);
        selectEntry(highestScoreMatch);
    }
}

void LoggedInWidget::beginIDTask(int id, enum ID_TASK task, int intent, EsdbActionBar *bar)
{
	m_id = id;
	m_idTask = task;
	m_taskIntent = intent;
	m_taskActionBar = bar;
	switch (m_idTask) {
	case ID_TASK_DELETE:
		::signetdev_update_uid(nullptr, &m_signetdevCmdToken, m_id, 0, nullptr, nullptr);
		break;
	case ID_TASK_READ:
		::signetdev_read_uid(nullptr, &m_signetdevCmdToken, m_id, 0);
		break;
	default:
		break;
	}
}

esdbTypeModule *LoggedInWidget::esdbEntryToModule(esdbEntry *entry)
{
	esdbTypeModule *module = nullptr;

	switch (entry->type) {
	case ESDB_TYPE_ACCOUNT:
		module = m_accounts;
		break;
	case ESDB_TYPE_BOOKMARK:
		module = m_bookmarks;
		break;
	case ESDB_TYPE_GENERIC_TYPE_DESC:
		module = m_genericTypeModule;
		break;
	case ESDB_TYPE_GENERIC: {
		generic *g = static_cast<generic *>(entry);
		if (g->typeId < m_genericModules.size()) {
			module = m_genericModules[g->typeId];
		}
		if (!module) {
			module = m_miscTypeData->module;
		}
		} break;
	}
	return module;
}

void LoggedInWidget::entryChanged(int id)
{
	esdbEntry *entry = m_entries[id];
	if (entry) {
		entryIconCheck(entry);
		int typeIdx = entryToIndex(entry);
		typeData *td = m_typeData.at(typeIdx);
		if (entry->type == ESDB_TYPE_GENERIC_TYPE_DESC) {
			m_dataTypesModel->moduleChanged(td->module);
		}
		populateEntryList(td, m_filterEdit->text());
	}
}

void LoggedInWidget::signetdevCmdResp(signetdevCmdRespInfo info)
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
	case SIGNET_ERROR_DISCONNECT:
	case SIGNET_ERROR_QUIT:
		break;
	default:
		do_abort = true;
		return;
	}

	switch (info.cmd) {
	case SIGNETDEV_CMD_TYPE:
		break;
	case SIGNETDEV_CMD_UPDATE_UID: {
		if (m_idTask == ID_TASK_DELETE) {
			EsdbActionBar *bar = getActiveActionBar();
			bar->idTaskComplete(false, m_id, nullptr, m_idTask, m_taskIntent);
			if (code == OKAY) {
				auto iter = m_entries.find(m_id);
				if (m_activeType->module == m_genericTypeModule) {
					genericTypeDesc *e = static_cast<genericTypeDesc *>(*iter);
					for (auto typeIter = m_typeData.begin(); typeIter != m_typeData.end(); typeIter++) {
						struct typeData *d = (*typeIter);
						if (d->module->name() == e->name) {
							m_miscTypeData->entries->unite(*(d->entries));
							m_dataTypesModel->removeModule(d->module);
							m_genericModules[e->typeId] = nullptr;
							delete d;
							m_typeData.erase(typeIter);
							break;
						}
					}
				}
				m_entries.erase(iter);
				m_activeType->entries->erase(m_activeType->entries->find(m_id));
				populateEntryList(m_activeType, m_filterEdit->text());
			}
			m_idTask = ID_TASK_NONE;
		}
		} break;
	default:
		break;
	}

	if (do_abort) {
		abort();
	}
}

void LoggedInWidget::signetdevReadUIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	int code = info.resp_code;
	block *b = new block();
	b->data = data;
	b->mask = mask;
	getEntryDone(m_id, code, b, true);
	m_idTask = ID_TASK_NONE;
}

void LoggedInWidget::getSelectedAccountRect(QRect &r)
{
	QRect s = m_searchListbox->visualRect(m_searchListbox->currentIndex());
	r.setTopLeft(m_searchListbox->mapToGlobal(s.topLeft()));
	r.setBottomRight(m_searchListbox->mapToGlobal(s.bottomRight()));
}

void LoggedInWidget::deselectEntry()
{
	m_filterEdit->setText(m_searchListbox->filterText());
	getActiveActionBar()->selectEntry(nullptr);
	m_filterEdit->setFocus();
	m_selectedEntry = nullptr;
	m_searchListbox->setCurrentIndex(QModelIndex());
}

void LoggedInWidget::selectEntry(esdbEntry *entry)
{
	//TODO: make sure the entry type matches
	m_selectedEntry = entry;
	if (!entry) {
		m_filterEdit->setText(m_searchListbox->filterText());
		getActiveActionBar()->selectEntry(nullptr);
	} else {
		m_searchListbox->setFocus();
		EsdbActionBar *bar = getActionBarByEntry(entry);
		if (bar)
			bar->selectEntry(entry);
		QString title = entry->getTitle();
		bool wordStart;
		int wordLocation;
		int loc = entry->matchLocation(m_searchListbox->filterText(), wordStart, wordLocation);

		int start = m_searchListbox->filterText().size();
		if (start) {
			m_filterEdit->setText(title);
			if (loc >= 0)
				m_filterEdit->setSelection(loc, start);
		} else {
			m_filterEdit->setText(QString());
			m_filterEdit->setFocus();
		}
	}
}

void LoggedInWidget::customContextMenuRequested(QPoint pt)
{
	selected(m_searchListbox->indexAt(pt));
	esdbEntry *entry = m_selectedEntry;
	if (m_selectedEntry) {
		EsdbActionBar *bar = getActionBarByEntry(entry);
		if (bar)
			bar->defaultAction(entry);
	}
}


void LoggedInWidget::filterTextEdited(QString text)
{
	Q_UNUSED(text);
	m_searchListbox->setFilterText(m_filterEdit->text());
	if (text.size()) {
		m_searchListbox->setFocus();
	}
	filterTextChanged(m_searchListbox->filterText());
}

void LoggedInWidget::focusChanged(QWidget *prev, QWidget *next)
{
	if ((prev == m_searchListbox || prev == m_filterEdit) &&
	    (next != m_searchListbox && next != m_filterEdit && next != nullptr)) {
	}
	if (next == m_filterEdit) {
		m_filterEdit->setText(m_searchListbox->filterText());
	}
}

EsdbActionBar *LoggedInWidget::getActiveActionBar()
{
	return static_cast<EsdbActionBar *>(m_actionBarStack->widget(m_activeTypeIndex));
}

int LoggedInWidget::entryToIndex(esdbEntry *entry)
{
	esdbTypeModule *module = esdbEntryToModule(entry);
	if (module) {
		for (int i = 0; i < m_typeData.size(); i++) {
			if (m_typeData.at(i)->module == module) {
				return i;
			}
		}
	}
	return -1;
}

const QMap<int, esdbEntry *> *LoggedInWidget::entryToEntryMap(esdbEntry *entry)
{
	int index = entryToIndex(entry);
	return m_typeData[index]->entries;
}

const QMap<int, esdbEntry *> *LoggedInWidget::typeNameToEntryMap(QString name)
{
	for (auto td : m_typeData) {
		if (td->module->name() == name) {
			return td->entries;
		}
	}
	return nullptr;
}

EsdbActionBar *LoggedInWidget::getActionBarByEntry(esdbEntry *entry)
{
	EsdbActionBar *bar = nullptr;
	int index = entryToIndex(entry);
	if (index >= 0) {
		bar = static_cast<EsdbActionBar *>(m_actionBarStack->widget(index));
	}
	return bar;
}

esdbTypeModule *LoggedInWidget::getTypeModule(int type)
{
	esdbTypeModule *module = nullptr;
	switch (type) {
	case ESDB_TYPE_GENERIC: {
		module = m_miscTypeData->module;
		} break;
	case ESDB_TYPE_GENERIC_TYPE_DESC:
		module = m_genericTypeModule;
		break;
	case ESDB_TYPE_ACCOUNT:
		module = m_accounts;
		break;
	case ESDB_TYPE_BOOKMARK:
		module = m_bookmarks;
		break;
	default:
		break;
	}
	return module;
}

void LoggedInWidget::filterTextChanged(QString text)
{
	populateEntryList(m_activeType, text);
	QList<esdbEntry *> *filteredList = m_activeType->filteredList;
	if (!selectedEntry() && filteredList->size()) {
		if (text.size() == 0) {
			selectEntry(nullptr);
			m_searchListbox->setCurrentIndex(QModelIndex());
		} else {
			selectFirstVisible();
		}
	} else {
		selectEntry(selectedEntry());
	}
}

void LoggedInWidget::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	m_filterEdit->setFocus();
}

const esdbEntry *LoggedInWidget::findEntry(QString type, QString name) const
{
	for (auto t : m_typeData) {
		if (t->module->name() == type) {
			QMap<int, esdbEntry *>::const_iterator iter;
			for (iter = t->entries->cbegin(); iter != t->entries->cend(); iter++) {
				if (iter.value()->getFullTitle() == name) {
					return iter.value();
				}
			}
		}
	}
	return nullptr;
}

QList<esdbTypeModule *> LoggedInWidget::getTypeModules()
{
	QList<esdbTypeModule *> modules;
	for (auto t : m_typeData) {
		modules.append(t->module);
	}
	return modules;
}

void LoggedInWidget::getEntryDone(int id, int code, block *blk, bool task)
{
	esdbEntry *entry = nullptr;

	int exists = m_entries.count(id);

	EsdbActionBar *bar = nullptr;

	if (exists && task) {
		entry = m_entries[id];
		bar = m_taskActionBar;
	}

	if (code != OKAY && code != ID_INVALID && code != BUTTON_PRESS_CANCELED && code != BUTTON_PRESS_TIMEOUT) {
		if (code != SIGNET_ERROR_DISCONNECT && code != SIGNET_ERROR_QUIT) {
			emit abort();
		}
		if (bar)
			bar->idTaskComplete(true, id, nullptr, m_idTask, m_taskIntent);
		return;
	}

	if (blk && code == OKAY) {
		esdbEntry_1 tmp(id);
		tmp.fromBlock(blk);
		esdbTypeModule *module = getTypeModule(tmp.type);
		if (module) {
			entry = module->decodeEntry(id, tmp.revision, entry, blk);
			if (entry) {
				if (bar) {
					bar->idTaskComplete(false, id, entry, m_idTask, m_taskIntent);
				} else {
					if (entry->type == ESDB_TYPE_GENERIC_TYPE_DESC) {
						genericTypeDesc *genericTypeDesc_ = static_cast<genericTypeDesc *>(entry);
						addGenericType(genericTypeDesc_);
					}
				}
			} else if (m_populating) {
				m_populatingCantRead++;
			}
		} else {

		}
	}

	if (entry) {
		if (!exists) {
			entryIconCheck(entry);
			m_entries[id] = entry;
			if (!m_populating) {
				int index = entryToIndex(entry);
				if (index >= 0) {
					m_typeData.at(index)->entries->insert(id, entry);
					populateEntryList(m_activeType, m_searchListbox->filterText());
				}
			}
		}
	}
	if (blk)
		delete blk;
}

void LoggedInWidget::selected(QModelIndex idx)
{
	if (idx.isValid()) {
		EsdbModelItem *item = static_cast<EsdbModelItem *>(idx.internalPointer());
		if (item && item->isLeafItem()) {
			EsdbModelLeafItem *ent = static_cast<EsdbModelLeafItem *>(idx.internalPointer());
			selectEntry(ent->leafNode());
		} else {
			selectEntry(nullptr);
		}
	} else {
		selectEntry(nullptr);
	}
}

void LoggedInWidget::activated(QModelIndex idx)
{
	if (idx.isValid()) {
		EsdbModelItem *item = static_cast<EsdbModelItem *>(idx.internalPointer());
		if (item && item->isLeafItem()) {
			EsdbModelLeafItem *ent = static_cast<EsdbModelLeafItem *>(idx.internalPointer());
			esdbEntry *entry = ent->leafNode();
			getActiveActionBar()->defaultAction(entry);
		}
	}
}

void LoggedInWidget::pressed(QModelIndex idx)
{
	if (idx.isValid()) {
		EsdbModelItem *item = static_cast<EsdbModelItem *>(idx.internalPointer());
		if (item && item->isLeafItem()) {
			EsdbModelLeafItem *ent = static_cast<EsdbModelLeafItem *>(idx.internalPointer());
			esdbEntry *entry = ent->leafNode();
			selectEntry(entry);
		} else {
			m_searchListbox->setExpanded(idx, !m_searchListbox->isExpanded(idx));
		}
	}
}

esdbEntry *LoggedInWidget::selectedEntry()
{
	return m_selectedEntry;
}

void LoggedInWidget::filterEditPressed()
{
	esdbEntry *entry = selectedEntry();
	if (entry) {
		getActiveActionBar()->defaultAction(entry);
	} else if (m_filterEdit->text().size()) {
		newEntryUI();
	}
}

int LoggedInWidget::getUnusedTypeId()
{
	std::vector<int> match;
	match.push_back(0);
	match.push_back(0);
	for (esdbEntry *entry : m_entries) {
		int typeIdx = -1;
		if (entry->type == ESDB_TYPE_GENERIC) {
			generic *g = static_cast<generic *>(entry);
			typeIdx = g->typeId;
		} else if (entry->type == ESDB_TYPE_GENERIC_TYPE_DESC) {
			genericTypeDesc *gt = static_cast<genericTypeDesc *>(entry);
			typeIdx = gt->typeId;
		}
		if (typeIdx >= 0 && (typeIdx + 1) >= match.size()) {
			match.resize(typeIdx + 2, 0);
		}
		if (typeIdx >= 0) {
			match[typeIdx] = 1;
		}
	}

	for (size_t i = 1; i < match.size(); i++) {
		if (!match[i])
			return static_cast<int>(i);
	}
	return -1;
}

int LoggedInWidget::getUnusedId()
{
	int entriesValid[MAX_UID+1];
	//TODO: optimize this
	for (int i = MIN_UID; i <= MAX_UID; i++) {
		entriesValid[i] = 0;
	}
	for (auto entry : m_entries) {
		entriesValid[entry->id] = 1;
	}
	int id = -1;
	for (id = MIN_UID; id <= MAX_UID; id++) {
		if (!entriesValid[id])
			break;
	}
	if (id > MAX_UID) {
		return -1;
	} else {
		return id;
	}
}

void LoggedInWidget::newEntryUI()
{
	int id = getUnusedId();
	if (id < 0) {
		SignetApplication::messageBoxError(QMessageBox::Warning,
						   "Account creation failed",
						   "No space left on device",
						   this);
		return;
	}
	QString entryName;
	if (!m_selectedEntry) {
		entryName = m_filterEdit->text();
	}
	getActiveActionBar()->newInstanceUI(id, entryName);
}

void LoggedInWidget::finishTask(bool deselect)
{
	m_searchListbox->setFilterText(QString());
	filterTextChanged(m_searchListbox->filterText());
	if (deselect) {
		deselectEntry();
	}
}

void LoggedInWidget::entryIconCheck(esdbEntry *entry)
{
	QList<iconAccount>::iterator iter = m_icon_accounts.begin();
	iconAccount *bestMatch = nullptr;
	int bestMatchQuality = 0;
	for (; iter != m_icon_accounts.end(); iter++) {
		iconAccount &i_account = *iter;
		int quality = i_account.matchQuality(entry);
		if (quality > bestMatchQuality) {
			bestMatch = &i_account;
			bestMatchQuality = quality;
		}
	}
	if (bestMatch) {
		QString fn(":images/logos/");
		fn.append(bestMatch->iconName);
		entry->setIcon(QIcon(fn));
	} else {
		entry->setIcon(m_genericIcon);
	}
	if (entry->type == ESDB_TYPE_GENERIC_TYPE_DESC) {

	}
}


void LoggedInWidget::addGenericType(genericTypeDesc *genericTypeDesc)
{
	typeData *d = new typeData(new esdbGenericModule(genericTypeDesc));
	d->actionBar = new GenericActionBar(this, d->module, genericTypeDesc, m_writeEnabled, m_typeEnabled);
	m_typeData.push_back(d);
	m_actionBarStack->addWidget(d->actionBar);
	m_dataTypesModel->addModule(d->module, false);
	if (m_genericModules.size() <= genericTypeDesc->typeId) {
		m_genericModules.resize(genericTypeDesc->typeId + 1, nullptr);
	}
	m_genericModules[genericTypeDesc->typeId] = static_cast<esdbGenericModule *>(d->module);
}

void LoggedInWidget::entryCreated(QString typeName, esdbEntry *entry)
{
	if (entry) {
		m_searchListbox->setFilterText(QString());
		entryIconCheck(entry);
		m_entries[entry->id] = entry;
		for (auto t : m_typeData) {
			if (t->module->name() == typeName) {
				t->entries->insert(entry->id, entry);
				populateEntryList(t, m_filterEdit->text());
			}
		}
		if (entry->type == ESDB_TYPE_GENERIC_TYPE_DESC) {
			genericTypeDesc *genericTypeDesc_ = static_cast<genericTypeDesc *>(entry);
			addGenericType(genericTypeDesc_);
		}
	}
}

bool LoggedInWidget::selectFirstVisible(QModelIndex &parent)
{
	EsdbModelItem *item = static_cast<EsdbModelItem *>(parent.internalPointer());
	if (item && item->isLeafItem()) {
		m_searchListbox->setCurrentIndex(parent);
		selectEntry(item->leafNode());
		return true;
	} else {
		if (parent.isValid() && !m_searchListbox->isExpanded(parent)) {
			return false;
		}
		int rowCount = m_activeType->model->rowCount(parent);
		for (int i = 0; i < rowCount; i++) {
			QModelIndex child = m_activeType->model->index(i, 0, parent);
			if (selectFirstVisible(child)) {
				return true;
			}
		}
		return false;
	}
}


bool LoggedInWidget::selectFirstVisible()
{
	QModelIndex parent;
	if (!selectFirstVisible(parent)) {
		deselectEntry();
		QModelIndex x = m_activeType->model->index(0);
		m_searchListbox->setCurrentIndex(x);
		return false;
	}
	return true;
}

void LoggedInWidget::populateEntryList(typeData *t, QString filter)
{
	QList<esdbEntry *> *filteredList = t->filteredList;
	QMap<int, esdbEntry *> *data = t->entries;
	EsdbModel *model = t->model;
	filteredList->clear();

	bool hasGroups = false;
	QVector<QList<esdbEntry *> > qualityGroups;
	for (auto x : *data) {
		esdbEntry *entry = x;
		if (entry->getPath().size()) {
			hasGroups = true;
		}
		int quality = entry->matchQuality(filter);
		if (quality) {
			if (qualityGroups.size() < quality) {
				qualityGroups.resize(quality);
			}
			qualityGroups[quality - 1].append(entry);
		}
	}

	int i;

	for (i = (qualityGroups.size() - 1); i >= 0; i--) {
		filteredList->append(qualityGroups[i]);
	}
	model->changed(hasGroups);
	m_searchListbox->setRootIsDecorated(hasGroups);

	if (!m_activeType->expanded) {
		expandTreeItems();
	} else {
		model->syncExpanded(m_searchListbox);
	}

	if (t == m_activeType) {
		QModelIndex index = model->findEntry(m_selectedEntry);
		bool matched = index.isValid();
		if (!matched) {
			if (filteredList->size() && m_selectedEntry) {
				selectFirstVisible();
			} else if (selectedEntry()) {
				deselectEntry();
			}
		} else {
			m_searchListbox->setCurrentIndex(index);
		}
	}
}
