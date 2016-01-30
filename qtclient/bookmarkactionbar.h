#ifndef BOOKMARKACTIONBAR_H
#define BOOKMARKACTIONBAR_H

#include "esdbactionbar.h"

#include <QObject>
#include <QList>

class LoggedInWidget;
class ButtonWaitDialog;
class QPushButton;

class esdbTypeModule;
class LoggedInWidget;
struct esdbEntry;
struct block;
class NewBookmark;

class BookmarkActionBar : public EsdbActionBar
{
	Q_OBJECT
	LoggedInWidget *m_parent;
	ButtonWaitDialog *m_buttonWaitDialog;
	NewBookmark *m_newEntryDlg;
	esdbTypeModule *m_module;

	//Helpers
	void browseUrl(esdbEntry *entry);
	QPushButton *addButton(const QString &tooltip, const QString &imagePath);

	//Overrides
	void selectEntry(esdbEntry *entry);
	void defaultAction(esdbEntry *entry);
	void newInstanceUI(int id, const QString &name);
	void getEntryDone(esdbEntry *entry, int intent);
	void idTaskComplete(int id, int intent);

	enum intent {
		NONE
	};

	QPushButton *m_browseButton;
	QList<QPushButton *> m_allButtons;
public:
	BookmarkActionBar(esdbTypeModule *module, LoggedInWidget *parent);
public slots:
	void entryCreated(esdbEntry *entry);
	void newEntryFinished(int);
	void deleteEntryFinished(int);
	void browseUrlUI();
	void openEntryUI();
	void deleteEntryUI();
};

#endif // BOOKMARKACTIONBAR_H
