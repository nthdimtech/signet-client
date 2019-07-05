#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H
#include <QList>
#include <QIcon>
#include <QAbstractItemModel>

#include "account.h"

struct esdbTypeModule;
struct esdbModelGroup;

class EsdbModelGroupItem;
class QTreeView;

class EsdbModelItem : public QObject
{
	int m_rank;
public:
	int rank() const
	{
		return m_rank;
	}
	void setRank(int rank)
	{
		m_rank = rank;
	}

	virtual esdbEntry *leafNode() = 0;
    virtual QString name() const = 0;
	virtual int rowCount() = 0;
	virtual EsdbModelGroupItem *parent() = 0;
	virtual QVariant data(int role) = 0;
	virtual ~EsdbModelItem() {}
	virtual int row() = 0;
    virtual bool isLeafItem() const = 0 ;
	EsdbModelItem(int rank_) : m_rank(rank_) {}
	static bool LessThan(const EsdbModelItem &r, const EsdbModelItem &l);
	bool LessThan(const EsdbModelItem *r, const EsdbModelItem *l);
};

class EsdbModelLeafItem : public EsdbModelItem
{
	esdbEntry *m_item;
	QIcon m_icon;
public:
	QString m_name;
	EsdbModelGroupItem *m_parent;
	esdbEntry *leafNode()
	{
		return m_item;
	}

    bool isLeafItem() const
	{
		return true;
	}

    QString name() const
	{
		return m_name;
	}

	int row();

	void setLeafEntry(esdbEntry *e)
	{
		m_item = e;
		if (m_item) {
			m_name = m_item->getTitle();
			m_icon = m_item->getIcon();
		}
	}

	int rowCount()
	{
		return 0;
	}

	EsdbModelItem *child(int row)
	{
		Q_UNUSED(row);
		return NULL;
	}

	QVariant data(int role);

	EsdbModelGroupItem *parent()
	{
		return m_parent;
	}
	EsdbModelLeafItem(esdbEntry *item, int rank, EsdbModelGroupItem *parent) :
		EsdbModelItem(rank),
		m_item(item),
		m_parent(parent)
	{
		if (m_item) {
			m_name = m_item->getTitle();
			m_icon = m_item->getIcon();
		}
	}
};

class EsdbModel;

class EsdbModelGroupItem : public EsdbModelItem
{
public:
	QString m_name;
	EsdbModelGroupItem *m_parent;
private:
	bool m_expanded;
public:
	QList<EsdbModelItem *> m_items;
	QList<EsdbModelGroupItem *> m_hiddenGroups;
	QList<EsdbModelLeafItem *> m_hiddenItems;
	void clearPointers();

    QString name() const
	{
		return m_name;
	}

	void expanded(bool e)
	{
		m_expanded = e;
	}

	esdbEntry *leafNode()
	{
        return nullptr;
	}
	int rowCount()
	{
		return m_items.size();
	}

    bool isLeafItem() const
	{
		return false;
	}

	bool isExpanded()
	{
		return m_expanded;
	}

	void setExpanded(bool expanded)
	{
		m_expanded = expanded;
	}

	EsdbModelItem *child(int row)
	{
		if (row >= m_items.size()) {
            return nullptr;
		} else {
			return m_items.at(row);
		}
	}

	EsdbModelGroupItem *parent()
	{
		return m_parent;
	}

	EsdbModelGroupItem(QString name, int rank, EsdbModelGroupItem *parent, bool expanded = true) :
		EsdbModelItem(rank),
		m_name(name),
		m_parent(parent),
		m_expanded(expanded)
	{

	}

	QVariant data(int role);

	void addItem(EsdbModelLeafItem *c)
	{
		m_items.push_back(c);
	}
	int row();
	void refreshEntry(esdbEntry *ent, int rank, const QStringList &path, int first);
	void sortItems();
	void cullEmptyItems(const QModelIndex &parent, EsdbModel *m);
	void refreshEntry(EsdbModel *m, const QModelIndex &parent, esdbEntry *ent, int rank, const QStringList &path, int first);
};

class EsdbModel : public QAbstractItemModel
{
	void refresh(bool useGroups);
	void clearPointers();
	friend class EsdbModelGroupItem;
public:
	EsdbModel(esdbTypeModule *module, QList<esdbEntry *> *entries);
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	void changed(bool useGroups);
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &child) const;
	QModelIndex index(int row);
	QModelIndex findEntry(const esdbEntry *ent) const;
	void expand(QModelIndex &index, bool expand);
	void syncExpanded(QTreeView *v);
	void createUnsortedGroups(QModelIndex idx, EsdbModelGroupItem *g);
private:
	QModelIndex findEntry(EsdbModelGroupItem *g, const esdbEntry *ent) const;
	QList<esdbEntry *> *m_entries;
	EsdbModelGroupItem *m_rootItem;
	void syncExpanded(QTreeView *v, QModelIndex &index, EsdbModelGroupItem *group);
};

#endif // ACCOUNTMODEL_H
