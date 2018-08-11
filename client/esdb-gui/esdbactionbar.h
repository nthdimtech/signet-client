#ifndef ESDBACTIONBAR_H
#define ESDBACTIONBAR_H

#include <QWidget>

struct esdbEntry;
struct block;
struct esdbTypeModule;

class EsdbActionBar : public QWidget
{
	Q_OBJECT
protected:
	esdbEntry *m_selectedEntry;
public:
	explicit EsdbActionBar(QWidget *parent = 0);
	esdbEntry *selectedEntry()
	{
		return m_selectedEntry;
	}
	virtual void selectEntry(esdbEntry *entry)
	{
		m_selectedEntry = entry;
	}

	virtual void getEntryDone(esdbEntry *entry, int intent) = 0;

	virtual int esdbType()
	{
		return -1;
	}

	virtual void defaultAction(esdbEntry *entry)
	{
		Q_UNUSED(entry);
	}
	virtual void idTaskComplete(int id, int task, int intent)
	{
		Q_UNUSED(id);
		Q_UNUSED(task);
		Q_UNUSED(intent);
	}

	virtual void newInstanceUI(int id, const QString &name)
	{
		Q_UNUSED(id);
		Q_UNUSED(name);
	}
signals:
	void background();
	void abort();
};

#endif // ESDBACTIONBAR_H
