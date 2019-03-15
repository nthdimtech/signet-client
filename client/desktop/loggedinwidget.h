#ifndef LOGGEDINWIDGET_H
#define LOGGEDINWIDGET_H

#include <QObject>
#include <QWidget>
#include <QList>
#include <QModelIndex>
#include <QDialog>
#include <QUrl>
#include <QMap>
#include <QVector>
#include <QIcon>

class MainWindow;
class CommThread;
class QPushButton;
class QDialog;
class EsdbModel;
class QListView;
class QLineEdit;
class ButtonWaitDialog;
class QGroupBox;
class QComboBox;
class QFrame;
class QRadioButton;
class QProgressBar;
class QComboBox;
class QStackedWidget;
#include <map>

struct account;
struct block;
struct esdbEntry;

class SearchListbox;
class SearchFilterEdit;
class MainWindow;
class QLabel;
class AspectRatioPixmapLabel;
class NewAccount;
class esdbGenericModule;
class esdbGenericTypeModule;
class QUrl;
#include "signetapplication.h"
#include "esdbtypemodule.h"
#include "esdbaccountmodule.h"
#include "esdbbookmarkmodule.h"
#include "../desktop/mainwindow.h"

struct iconAccount {
	QString name;
	QString url;
	QString iconName;
	QUrl urlObj;
	iconAccount(const QString &_name,
	            const QString &_url,
	            const QString &_icon_name) :
	        name(_name),
	        url(_url),
	        iconName(_icon_name),
	        urlObj(_url)
	{

	}
	iconAccount(const QString &_name) :
	        name(_name),
	        url(_name),
	        iconName(_name),
	        urlObj(_name)
	{
		url.append(".com");
		iconName.append(".png");
	}

	int matchQuality(esdbEntry *acct);
};

struct entryAction {
	QString name;
	QString tooltip;
	QIcon icon;
};

struct esdbTypeData {
	QList<esdbEntry *> m_filteredList;
};

class EsdbActionBar;
class DataTypeListModel;
struct genericTypeDesc;

class LoggedInWidget : public QWidget
{
	Q_OBJECT
	QIcon m_genericIcon;

	bool m_fileMode;

	QList<iconAccount> m_icon_accounts;
	QMap<int, esdbEntry *> m_entries;

	struct typeData {
		esdbTypeModule *module;
		QMap<int, esdbEntry *> *entries;
		EsdbActionBar *actionBar;
		QList<esdbEntry *> *filteredList;
		EsdbModel *model;
		bool expanded;
		typeData(esdbTypeModule *_module);
		~typeData();
	};

	DataTypeListModel *m_dataTypesModel;

	typeData *m_activeType;
	int m_activeTypeIndex;

	esdbEntry *m_selectedEntry;

	void entryIconCheck(esdbEntry *acct);

	AspectRatioPixmapLabel *m_filterLabel;
	QPushButton *m_newAcctButton;

	QMap<QString, int> m_entryActionTokens;
	QList<entryAction> m_entryActions;

	bool m_populating;
	int m_populatingCantRead;
	SearchListbox *m_searchListbox;
	QStackedWidget *m_actionBarStack;

	QProgressBar *m_loadingProgress;

	esdbAccountModule *m_accounts;
	esdbBookmarkModule *m_bookmarks;
	SearchFilterEdit *m_filterEdit;
	esdbGenericTypeModule *m_genericTypeModule;
	std::vector<esdbGenericModule *> m_genericModules;
	NewAccount *m_newAccountDlg;
	QWidget *m_accountGroup;
	QComboBox *m_viewSelector;
	int m_signetdevCmdToken;
	void accountActionsEnabled(bool enabled);
	void populateEntryList(typeData *t, QString filter);

	esdbEntry *selectedEntry();
	void clearSelection();
	void showEvent(QShowEvent *event);

	int m_id;
	int m_taskIntent;
	EsdbActionBar *m_taskActionBar;

	bool m_writeEnabled;
	bool m_typeEnabled;

	void getEntryDone(int id, int code, block *, bool task);
	int entryToIndex(esdbEntry *entry);
	EsdbActionBar *getActionBarByEntry(esdbEntry *entry);
	EsdbActionBar *getActiveActionBar();
	esdbTypeModule *getTypeModule(int type);
	void expandTreeItems();
	bool selectFirstVisible();
	bool selectFirstVisible(QModelIndex &parent);
	void expandTreeItems(QModelIndex parent);
	void deselectEntry();
	void addGenericType(genericTypeDesc *genericTypeDesc_);
	QList<typeData *> m_typeData;
    typeData *m_miscTypeData;
    int scoreUrlMatch(const QUrl &a, const QUrl &b);
public:
	enum ID_TASK {
		ID_TASK_NONE,
		ID_TASK_DELETE,
		ID_TASK_READ
	} m_idTask;
	esdbTypeModule *esdbEntryToModule(esdbEntry *entry);
	explicit LoggedInWidget(QProgressBar *loading_progress, MainWindow *mw, QWidget *parent = 0);
	~LoggedInWidget();
	void finishTask(bool deselect = true);
	void beginIDTask(int id, enum ID_TASK task, int intent, EsdbActionBar *bar);
	void getSelectedAccountRect(QRect &r);
	int getUnusedId();
	int getUnusedTypeId();
	const esdbEntry *findEntry(QString type, QString name) const;
	const QMap<int, esdbEntry *> *entryToEntryMap(esdbEntry *entry);

	QList<esdbTypeModule *> getTypeModules();
	const QMap<int, esdbEntry *> *typeNameToEntryMap(QString name);
	void getCurrentGroups(QString typeName, QStringList &groups);
signals:
	void abort();
	void enterDeviceState(int);
	void background();
public slots:
	void open();
	void entryChanged(int id);
	void signetdevReadUIdResp(signetdevCmdRespInfo info, QByteArray data, QByteArray mask);
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void signetdevReadAllUIdsResp(signetdevCmdRespInfo info, int id, QByteArray data, QByteArray mask);
	void entryCreated(QString typeName, esdbEntry *entry);
	void filterEditPressed();
	void currentTypeIndexChanged(int idx);
	void focusChanged(QWidget *prev, QWidget *next);
	void selected(QModelIndex idx);
	void activated(QModelIndex idx);
	void pressed(QModelIndex idx);
	void selectEntry(esdbEntry *);
	void filterTextEdited(QString text);
	void filterTextChanged(QString text);
	void newEntryUI();
	void customContextMenuRequested(QPoint pt);
	void signetDevEvent(int);
	void expanded(QModelIndex index);
	void collapsed(QModelIndex index);
private slots:
    void selectUrl(QString url);
};

#endif // LOGGEDINWIDGET_H
