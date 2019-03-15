#ifndef ESDBACTIONBAR_H
#define ESDBACTIONBAR_H

#include <QWidget>

struct esdbEntry;
struct block;
struct esdbTypeModule;
class QPushButton;
class LoggedInWidget;
class ButtonWaitDialog;

class EsdbActionBar : public QWidget
{
	Q_OBJECT
	bool m_accessDeselect;
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
	ButtonWaitDialog *m_buttonWaitDialog;

	enum intent {
		INTENT_NONE,
		INTENT_OPEN_ENTRY,
		INTENT_COPY_ENTRY,
		INTENT_TYPE_ENTRY
	};
	void accessEntry(esdbEntry *entry, int intent, QString message, bool backgroundApp, bool deselect);
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

	virtual void accessEntryComplete(esdbEntry *entry, int intent)
	{
		Q_UNUSED(entry);
		Q_UNUSED(intent);
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

	void idTaskComplete(bool error, int id, esdbEntry *entry, int task, int intent);

signals:
	void background();
	void abort();
private slots:
	void openEntryFinished(int);
	void accessAccountFinished(int code);
	void deleteEntryFinished(int);
};

#endif // ESDBACTIONBAR_H
