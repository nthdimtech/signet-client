#ifndef GENERICACTIONBAR_H
#define GENERICACTIONBAR_H

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

struct genericTypeDesc;

class GenericActionBar : public EsdbActionBar
{
	Q_OBJECT
	LoggedInWidget *m_parent;
	ButtonWaitDialog *m_buttonWaitDialog;
	esdbTypeModule *m_module;
	genericTypeDesc *m_typeDesc;
	QDialog *m_newEntryDlg;

	//Helpers
	void browseUrl(esdbEntry *entry);
	void openEntry(esdbEntry *entry);
	QPushButton *addButton(const QString &tooltip, const QString &imagePath);

	virtual int esdbType();

	//Overrides
	void selectEntry(esdbEntry *entry);
	void defaultAction(esdbEntry *entry);
	void newInstanceUI(int id, const QString &name);
	void getEntryDone(esdbEntry *entry, int intent);
	void idTaskComplete(int id, int task, int intent);

	enum intent {
		NONE,
		OPEN_ACCOUNT
	};

	QPushButton *m_browseButton;
	QList<QPushButton *> m_allButtons;
public:
	GenericActionBar(esdbTypeModule *module, genericTypeDesc *desc, LoggedInWidget *parent);

	genericTypeDesc *typeDesc()
	{
		return m_typeDesc;
	}
public slots:
	void entryCreated(esdbEntry *entry);
	void newEntryFinished(int);
	void deleteEntryFinished(int);
	void openEntryFinished(int);
	void browseUrlUI();
	void openEntryUI();
	void deleteEntryUI();
signals:
	void abort();
};

#endif // GENERICACTIONBAR_H
