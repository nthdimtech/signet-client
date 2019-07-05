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
	esdbTypeModule *m_module;
	genericTypeDesc *m_typeDesc;
	QDialog *m_newEntryDlg;

	virtual int esdbType();

	//Overrides
	virtual void entrySelected(esdbEntry *entry);
	void defaultAction(esdbEntry *entry);
	void newInstanceUI(int id, const QString &name);

	QPushButton *m_browseButton;
    bool accessEntryComplete(esdbEntry *entry, int intent) override;
public:
	GenericActionBar(LoggedInWidget *parent, esdbTypeModule *module, genericTypeDesc *typeDesc, bool writeEnabled, bool typeEnabled);

	genericTypeDesc *typeDesc()
	{
		return m_typeDesc;
	}
public slots:
	void entryCreated(esdbEntry *entry);
	void newEntryFinished(int);
	void browseUrlUI();
	void openEntryUI();
	void deleteEntryUI();
};

#endif // GENERICACTIONBAR_H
