#ifndef BOOKMARKACTIONBAR_H
#define BOOKMARKACTIONBAR_H

#include "esdbactionbar.h"

#include <QObject>
#include <QList>

class LoggedInWidget;
class ButtonWaitDialog;
class QPushButton;

struct esdbTypeModule;
class LoggedInWidget;
struct esdbEntry;
struct block;
class NewBookmark;

class BookmarkActionBar : public EsdbActionBar
{
	Q_OBJECT
	NewBookmark *m_newEntryDlg;
	esdbTypeModule *m_module;

	//Overrides
	void entrySelected(esdbEntry *entry);
	void defaultAction(esdbEntry *entry);
	void newInstanceUI(int id, const QString &name);

	QPushButton *m_browseButton;
	QList<QPushButton *> m_allButtons;

	int esdbType();

public:
	BookmarkActionBar(esdbTypeModule *module, LoggedInWidget *parent, bool writeEnabled = true, bool typeEnabled = true);
public slots:
	void entryCreated(esdbEntry *entry);
	void newEntryFinished(int);
	void browseUrlUI();
	void openEntryUI();
	void deleteEntryUI();
};

#endif // BOOKMARKACTIONBAR_H
