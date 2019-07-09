#ifndef ESDBACTIONBAR_H
#define ESDBACTIONBAR_H

#include <QWidget>

struct esdbEntry;
struct block;
struct esdbTypeModule;
class QPushButton;
class LoggedInWidget;
class ButtonWaitDialog;
class ButtonWaitWidget;

#include "loggedinwidget.h"

class EsdbActionBar : public QWidget
{
	Q_OBJECT
protected:
	LoggedInWidget *m_parent;
	bool m_writeEnabled;
	bool m_typeEnabled;
	QString m_typeName;
	esdbEntry *m_selectedEntry;
	QList<QPushButton *> m_allButtons;
	QPushButton *addButton(const QString &tooltip, const QString &imagePath, bool writeOp = false, bool typeOp = false);
	QPushButton *addButton(QPushButton *button, bool writeOp = false, bool typeOp = false);
	QPushButton *addDeleteButton()
	{
		return addButton("Delete", ":/images/delete.png", true, false);
	}
	QPushButton *addOpenButton()
	{
		return addButton("Open", ":/images/open.png");
	}
	QPushButton *addBrowseButton()
	{
		return addButton("Browse", ":/images/browse.png");
	}

	void deleteEntry();
    void openEntry(esdbEntry *entry);
	void browseUrl(esdbEntry *entry);

	enum intent {
		INTENT_NONE,
		INTENT_OPEN_ENTRY,
		INTENT_COPY_ENTRY,
		INTENT_TYPE_ENTRY
	};
	void accessEntry(esdbEntry *entry, int intent, QString message, bool backgroundApp);
public:
	explicit EsdbActionBar(LoggedInWidget *parent, QString typeName, bool writeEnabled, bool typeEnabled);

	esdbEntry *selectedEntry()
	{
		return m_selectedEntry;
	}

	QString typeName() const
	{
		return m_typeName;
	}

	void selectEntry(esdbEntry *entry);

	virtual void entrySelected(esdbEntry *entry)
	{
		Q_UNUSED(entry);
	}

    virtual bool accessEntryComplete(esdbEntry *entry, int intent)
	{
		Q_UNUSED(entry);
		Q_UNUSED(intent);
        return true;
	}

	virtual void deleteEntryComplete(esdbEntry *entry)
	{
		Q_UNUSED(entry);
	}

	virtual int esdbType() = 0;
	virtual void newInstanceUI(int id, const QString &name) = 0;

	virtual void defaultAction(esdbEntry *entry)
	{
		Q_UNUSED(entry);
	}

	void idTaskComplete(bool error, int id, esdbEntry *entry, enum LoggedInWidget::ID_TASK task, int intent);

signals:
	void background();
	void abort();
private slots:
    void buttonWaitTimeout();
    void buttonWaitCanceled();
};

#endif // ESDBACTIONBAR_H
