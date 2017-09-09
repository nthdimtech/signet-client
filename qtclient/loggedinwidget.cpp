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

#include "esdb.h"
#include "esdbmodel.h"
#include "aspectratiopixmaplabel.h"
#include "editaccount.h"
#include "newaccount.h"
#include "searchlistbox.h"
#include "searchfilteredit.h"
#include "buttonwaitdialog.h"
#include "mainwindow.h"
#include "signetapplication.h"
#include "esdbactionbar.h"
#include "generictypedesc.h"
#include "esdbgenericmodule.h"
#include "genericactionbar.h"

extern "C" {
#include "common.h"
#include "signetdev.h"
}

#include "bookmark.h"
#include "generic.h"
#include "mainwindow.h"

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

	QUrl *compUrl = NULL;
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

 LoggedInWidget::LoggedInWidget(MainWindow *mw, QProgressBar *loading_progress, QWidget *parent) : QWidget(parent),
	m_activeType(0),
	m_selectedEntry(NULL),
	m_filterLabel(NULL),
	m_newAcctButton(NULL),
	m_populating(true),
	m_searchListbox(NULL),
	m_loadingProgress(loading_progress),
	m_filterEdit(NULL),
	m_accountGroup(NULL),
	m_signetdevCmdToken(-1),
	m_id(-1),
	m_idTask(ID_TASK_NONE)
{
	Q_UNUSED(mw);
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

	m_activeType = 0;

	genericTypeDesc *place = new genericTypeDesc(-1);
	place->name = "";
	m_genericDecoder = new esdbGenericModule(place, this);

	m_accounts = new esdbAccountModule(this);
	m_bookmarks = new esdbBookmarkModule(this);
	m_dataTypeModules.push_back(m_accounts);
	m_dataTypeModules.push_back(m_bookmarks);

	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChanged(QWidget*,QWidget*)));

	connect(app, SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)), this,
		SLOT(signetdevCmdResp(signetdevCmdRespInfo)));

	connect(app, SIGNAL(signetdevReadIdResp(signetdevCmdRespInfo, QByteArray, QByteArray)), this,
		SLOT(signetdevReadIdResp(signetdevCmdRespInfo, QByteArray, QByteArray)));

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
	for (auto iter : m_dataTypeModules) {
		auto data = new QMap<int, esdbEntry *>();
		m_entriesByType.push_back(data);
		auto filteredList = new QList<esdbEntry *>();
		m_filteredLists.push_back(filteredList);
		auto esdbModel = new EsdbModel(iter, filteredList);
		m_esdbModel.push_back(esdbModel);
		EsdbActionBar *actionBar = iter->newActionBar();
		connect(actionBar, SIGNAL(background()), this, SIGNAL(background()));
		m_actionBarStack->addWidget(actionBar);
	}

	m_filterLabel = new AspectRatioPixmapLabel();
	QIcon icn = QIcon(":/images/search.png");
	QPixmap pm = icn.pixmap(QSize(128,128));
	m_filterLabel->setPixmap(pm);
	m_filterLabel->setToolTip("Account search");

	m_searchListbox = new SearchListbox(m_filterEdit, m_esdbModel.at(m_activeType));
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
	for (auto dataType : m_dataTypeModules) {
		m_viewSelector->addItem(dataType->m_name);
	}
	m_viewSelector->insertSeparator(6);
	m_viewSelector->setCurrentIndex(m_activeType);
	connect(m_viewSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(currentTypeIndexChanged(int)));

	QBoxLayout *top_layout = new QBoxLayout(QBoxLayout::TopToBottom);
	top_layout->setAlignment(Qt::AlignTop);
	top_layout->addLayout(filter_layout);
	top_layout->addWidget(m_viewSelector);
	top_layout->addWidget(m_searchListbox);
	top_layout->addWidget(m_actionBarStack);
	top_layout->setAlignment(m_actionBarStack, Qt::AlignBottom);

	setLayout(top_layout);

	selectEntry(NULL);

	m_searchListbox->setEnabled(false);
	m_filterEdit->setEnabled(false);
	m_newAcctButton->setEnabled(false);
	m_filterLabel->setEnabled(false);
	m_populating = true;

	m_idTask = ID_TASK_READ_POPULATE;
	m_taskIntent = ID_TASK_NONE;
	m_id = MIN_ID;
	::signetdev_read_id_async(NULL, &m_signetdevCmdToken, m_id);
}

void LoggedInWidget::currentTypeIndexChanged(int idx)
{
	m_activeType = idx;
	m_searchListbox->setModel(m_esdbModel.at(idx));
	m_selectedEntry = NULL;
	m_actionBarStack->setCurrentIndex(idx);
	populateEntryList(m_activeType, m_filterEdit->text());
	m_filterEdit->setFocus();
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
	Q_UNUSED(code);
	open();
}

void LoggedInWidget::beginIDTask(int id, enum ID_TASK task, int intent)
{
	m_id = id;
	m_idTask = task;
	m_taskIntent = intent;
	::signetdev_open_id_async(NULL, &m_signetdevCmdToken, m_id);
}

void LoggedInWidget::entryChanged(int id)
{
	esdbEntry *entry = m_entries[id];
	entryIconCheck(entry);
	switch (entry->type) {
	case ESDB_TYPE_ACCOUNT:
		//TODO: bad magic number
		populateEntryList(0, m_filterEdit->text());
		break;
	case ESDB_TYPE_BOOKMARK:
		//TODO: bad magic number
		populateEntryList(1, m_filterEdit->text());
		break;
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
	case SIGNETDEV_CMD_OPEN_ID:
		if (code == OKAY) {
			switch (m_idTask) {
			case ID_TASK_DELETE:
				::signetdev_delete_id_async(NULL, &m_signetdevCmdToken, m_id);
				break;
			case ID_TASK_READ:
				::signetdev_read_id_async(NULL, &m_signetdevCmdToken, m_id);
				break;
			default:
				m_idTask = ID_TASK_NONE;
				break;
			}
		} else {
			m_idTask = ID_TASK_NONE;
		}
		break;
	case SIGNETDEV_CMD_DELETE_ID: {
		EsdbActionBar *bar = getActiveActionBar();
		bar->idTaskComplete(m_id, m_taskIntent);
		if (code == OKAY) {
			::signetdev_close_id_async(NULL, &m_signetdevCmdToken, m_id);
			m_entries.erase(m_entries.find(m_id));
			//TODO: too long line
			m_entriesByType.at(m_activeType)->erase(m_entriesByType.at(m_activeType)->find(m_id));
			populateEntryList(m_activeType, m_filterEdit->text());
		} else {
			m_idTask = ID_TASK_NONE;
		}
	}
	break;
	case SIGNETDEV_CMD_CLOSE_ID:
		m_id = -1;
		m_idTask = ID_TASK_NONE;
		break;
	}

	if (do_abort) {
		abort();
	}
}

void LoggedInWidget::signetdevReadIdResp(signetdevCmdRespInfo info,
	QByteArray data,  QByteArray mask)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
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

	if (code == OKAY || code == ID_INVALID) {
		block *b = new block();
		b->data = data;
		b->mask = mask;
		this->getEntryDone(m_id, code, b);
		if (m_idTask == ID_TASK_READ_POPULATE) {
			m_id++;
			if (m_id <= MAX_ID) {
				::signetdev_read_id_async(NULL, &m_signetdevCmdToken, m_id);
			} else {
				m_idTask = ID_TASK_NONE;
			}
		}
	} else {
		m_idTask = ID_TASK_NONE;
	}

	if (do_abort) {
		abort();
	}
}

void LoggedInWidget::getSelectedAccountRect(QRect &r)
{
	QRect s = m_searchListbox->visualRect(m_searchListbox->currentIndex());
	r.setTopLeft(m_searchListbox->mapToGlobal(s.topLeft()));
	r.setBottomRight(m_searchListbox->mapToGlobal(s.bottomRight()));
}

void LoggedInWidget::selectEntry(esdbEntry *entry)
{
	//TODO: make sure the entry type matches
	m_selectedEntry = entry;
	if (!entry) {
		m_searchListbox->setCurrentIndex(QModelIndex());
		getActiveActionBar()->selectEntry(NULL);
	} else {
		EsdbActionBar *bar = getActionBarByEntry(entry);
		if (bar)
			bar->selectEntry(entry);
		QString title = entry->getTitle();
		int start = m_searchListbox->filterText().size();
		if (start) {
			m_filterEdit->setText(title);
			int length = title.size() - start;
			m_filterEdit->setSelection(start, length);
		} else {
			m_filterEdit->setText("");
		}
	}
}

void LoggedInWidget::customContextMenuRequested(QPoint pt)
{
	QPoint globalPos = m_searchListbox->mapToGlobal(pt);
	QMenu ctx;
	QAction *login = ctx.addAction("Login");
	QAction *open = ctx.addAction("Open");
	QAction *browse = ctx.addAction("Browse");
	QAction *type_username = ctx.addAction("Type username");
	QAction *type_password = ctx.addAction("Type password");
	QAction *del = ctx.addAction("Delete");
	connect(login, SIGNAL(triggered(bool)), this, SLOT(type_account_user_pass_ui()));
	connect(open, SIGNAL(triggered(bool)), this, SLOT(open_account_ui()));
	connect(browse, SIGNAL(triggered(bool)), this, SLOT(browse_url_ui()));
	connect(type_username, SIGNAL(triggered(bool)), this, SLOT(type_account_user_ui()));
	connect(type_password, SIGNAL(triggered(bool)), this, SLOT(type_account_pass_ui()));
	connect(del, SIGNAL(triggered(bool)), this, SLOT(delete_account_ui()));
	ctx.exec(globalPos);
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
	    (next != m_searchListbox && next != m_filterEdit && next != NULL)) {
	}
	if (next == m_filterEdit) {
		m_filterEdit->setText(m_searchListbox->filterText());
	}
}

EsdbActionBar *LoggedInWidget::getActiveActionBar()
{
	return (EsdbActionBar *)m_actionBarStack->widget(m_activeType);
}

int LoggedInWidget::esdbTypeToIndex(int type)
{
	int ret = -1;
	switch (type) {
	case ESDB_TYPE_ACCOUNT:
		return 0;
		break;
	case ESDB_TYPE_BOOKMARK:
		return 1;
		break;
	}
	return ret;
}

int LoggedInWidget::esdbEntryToIndex(esdbEntry *entry)
{
	int ret = -1;
	switch (entry->type) {
	case ESDB_TYPE_ACCOUNT:
		return 0;
		break;
	case ESDB_TYPE_BOOKMARK:
		return 1;
		break;
	case ESDB_TYPE_GENERIC: {
		generic *g = (generic *)entry;
		for (int i = 0; i < m_actionBarStack->count(); i++) {
			EsdbActionBar *bar = (EsdbActionBar *)m_actionBarStack->widget(i);
			if (bar->esdbType() == ESDB_TYPE_GENERIC) {
				GenericActionBar *genericBar = (GenericActionBar *)bar;
				if (genericBar->typeDesc()->name == g->typeName) {
					return i;
				}
			}
		}
	}
	break;
	}
	return ret;
}

EsdbActionBar *LoggedInWidget::getActionBarByEntry(esdbEntry *entry)
{
	EsdbActionBar *bar = NULL;
	int index = esdbEntryToIndex(entry);
	if (index >= 0) {
		bar = (EsdbActionBar *)m_actionBarStack->widget(index);
	}
	return bar;
}

esdbTypeModule *LoggedInWidget::getTypeModule(int type)
{
	esdbTypeModule *module = NULL;
	if (type == ESDB_TYPE_GENERIC) {
		return m_genericDecoder;
	} else {
		int index = esdbTypeToIndex(type);
		if (index >= 0) {
			module = m_dataTypeModules.at(index);
		}
	}
	return module;
}

void LoggedInWidget::filterTextChanged(QString text)
{
	populateEntryList(m_activeType, text);
	QList<esdbEntry *> *filteredList = m_filteredLists.at(m_activeType);
	if (filteredList->size() == 0 || text.size() == 0) {
		m_filterEdit->setFocus();
	}
	if (!selectedEntry() && filteredList->size()) {
		if (text.size() == 0) {
			m_filterEdit->setFocus();
		}
		m_searchListbox->setCurrentIndex(m_esdbModel.at(m_activeType)->index(0));
		selectEntry(filteredList->at(0));
	} else {
		selectEntry(selectedEntry());
	}
}

void LoggedInWidget::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	m_filterEdit->setFocus();
}

void LoggedInWidget::getEntryDone(int id, int code, block *blk)
{
	esdbEntry *entry = NULL;

	int exists = m_entries.count(id);

	if (exists) {
		entry = m_entries[id];
		if (m_idTask != ID_TASK_READ_POPULATE) {
			EsdbActionBar *bar = getActionBarByEntry(entry);
			if (bar)
				bar->idTaskComplete(id, m_taskIntent);
		}
	}

	if (code != OKAY && code != ID_INVALID && code != BUTTON_PRESS_CANCELED && code != BUTTON_PRESS_TIMEOUT) {
		if (code != SIGNET_ERROR_DISCONNECT && code != SIGNET_ERROR_QUIT) {
			emit abort();
		}
		return;
	}

	if (blk && code == OKAY) {
		esdbEntry_1 tmp(id);
		tmp.fromBlock(blk);
		esdbTypeModule *module = getTypeModule(tmp.type);
		entry = module->decodeEntry(id, tmp.revision, entry, blk);
		EsdbActionBar *bar = getActionBarByEntry(entry);
		if (entry && bar) {
			bar->getEntryDone(entry, m_taskIntent);
		} else {
			//TODO
		}
	}

	if (entry) {
		if (!exists) {
			entryIconCheck(entry);
			m_entries[id] = entry;
			int index = esdbEntryToIndex(entry);
			if (index >= 0) {
				(*m_entriesByType.at(index))[id] = entry;
				if (!m_populating) {
					populateEntryList(m_activeType, m_searchListbox->filterText());
				}
			}
		}
	}
	if (blk)
		delete blk;

	if (m_populating) {
		m_loadingProgress->setMinimum(MIN_ID);
		m_loadingProgress->setMaximum(MAX_ID);
		m_loadingProgress->setValue(id);
	}

	if (m_populating && id == MAX_ID) {
		m_populating = false;
		m_searchListbox->setEnabled(true);
		m_filterEdit->setEnabled(true);
		m_newAcctButton->setEnabled(true);
		m_filterLabel->setEnabled(true);
		populateEntryList(m_activeType, m_searchListbox->filterText());
		emit enterDeviceState(MainWindow::STATE_LOGGED_IN);
	}

}

void LoggedInWidget::abortProxy()
{
	emit abort();
}

void LoggedInWidget::selected(QModelIndex idx)
{
	if (idx.isValid()) {
		int index = idx.row();
		auto list = m_filteredLists.at(m_activeType);
		esdbEntry *acct = list->at(index);
		selectEntry(acct);
	}
}

void LoggedInWidget::activated(QModelIndex idx)
{
	if (idx.isValid()) {
		int index = idx.row();
		esdbEntry *entry = m_filteredLists.at(m_activeType)->at(index);
		selectEntry(entry);
		getActiveActionBar()->defaultAction(entry);
	}
}

void LoggedInWidget::pressed(QModelIndex idx)
{
	if (idx.isValid()) {
		int index = idx.row();
		esdbEntry *acct = m_filteredLists.at(m_activeType)->at(index);
		selectEntry(acct);
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

void LoggedInWidget::newEntryUI()
{
	int entriesValid[MAX_ID+1];
	for (int i = MIN_ID; i <= MAX_ID; i++) {
		entriesValid[i] = 0;
	}
	for (auto entry : m_entries) {
		entriesValid[entry->id] = 1;
	}
	int id;
	for (id = MIN_ID; id <= MAX_ID; id++) {
		if (!entriesValid[id])
			break;
	}
	if (id > MAX_ID) {
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
		selectEntry(NULL);
		m_filterEdit->setFocus();
	}
}

void LoggedInWidget::entryIconCheck(esdbEntry *entry)
{
	QList<iconAccount>::iterator iter = m_icon_accounts.begin();
	iconAccount *bestMatch = NULL;
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
		entry->clearIcon();
	}
}

void LoggedInWidget::entryCreated(EsdbActionBar *actionBar, esdbEntry *entry)
{
	if (entry) {
		m_searchListbox->setFilterText(QString());
		entryIconCheck(entry);
		m_entries[entry->id] = entry;
		int typeIndex = -1;
		int i = 0;
		for (i = 0; i < m_actionBarStack->count(); i++) {
			if (actionBar == m_actionBarStack->widget(i)) {
				typeIndex = i;
			}
		}
		if (typeIndex >= 0) {
			(*m_entriesByType.at(typeIndex))[entry->id] = entry;
			populateEntryList(typeIndex, m_filterEdit->text());
		}
	}
}

void LoggedInWidget::populateEntryList(int index, QString filter)
{
	QList<esdbEntry *> *filteredList = m_filteredLists.at(index);
	QMap<int, esdbEntry *> *data = m_entriesByType.at(index);
	EsdbModel *model = m_esdbModel.at(index);
	filteredList->clear();

	QVector<QList<esdbEntry *> > qualityGroups;
	for (auto x : *data) {
		esdbEntry *entry = x;
		int quality = entry->matchQuality(filter);
		if (quality) {
			if (qualityGroups.length() < quality) {
				qualityGroups.resize(quality);
			}
			qualityGroups[quality - 1].append(entry);
		}
	}

	int i;

	for (i = (qualityGroups.size() - 1); i >= 0; i--) {
		filteredList->append(qualityGroups[i]);
	}
	model->changed();

	if (index == m_activeType) {
		bool matched = false;
		int match_idx = -1;
		int i = 0;
		for (auto x : *filteredList) {
			if (x == m_selectedEntry) {
				match_idx = i;
				matched = true;
			}
			i++;
		}
		if (!matched) {
			if (filteredList->size() && m_selectedEntry) {
				m_searchListbox->setCurrentIndex(model->index(0));
				selectEntry(filteredList->at(0));
			} else if (selectedEntry()) {
				m_searchListbox->setCurrentIndex(QModelIndex());
				selectEntry(NULL);
			}
		} else {
			m_searchListbox->setCurrentIndex(model->index(match_idx));
		}
	}
}
