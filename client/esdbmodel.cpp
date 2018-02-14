#include "esdbmodel.h"
#include <QModelIndex>
#include <QDebug>
#include <QIcon>
#include "esdbtypemodule.h"

void EsdbModelGroupItem::refreshEntry(EsdbModel *m, const QModelIndex &parent, esdbEntry *ent, int rank, const QStringList &path, int first)
{
	if (first == path.size()) {
		int row = 0;
		for (EsdbModelItem *item : m_items) {
			if (item->isLeafItem()) {
				EsdbModelLeafItem *leafItem = (EsdbModelLeafItem *) item;
				if(leafItem->name() == ent->getTitle()) {
					leafItem->setRank(rank);
					leafItem->setLeafEntry(ent);
					return;
				}
			}
			row++;
		}
		EsdbModelLeafItem *leafItem = new EsdbModelLeafItem(ent, rank, this);
		m->beginInsertRows(parent, row, row);
		m_items.append(leafItem);
		m->endInsertRows();
	} else {
		int row = 0;
		for (EsdbModelItem *item : m_items) {
		       if (!item->isLeafItem()) {
				EsdbModelGroupItem *groupItem = (EsdbModelGroupItem *) item;
				if (groupItem->name() == path.at(first)) {
					groupItem->setRank(rank);
					groupItem->refreshEntry(m, m->createIndex(row, 0, groupItem), ent, rank, path, first + 1);
					return;
				}
		       }
		       row++;
		}
		EsdbModelGroupItem *groupItem = new EsdbModelGroupItem(path.at(first), rank, this);
		m->beginInsertRows(parent, row, row);
		m_items.append(groupItem);
		m->endInsertRows();
		groupItem->refreshEntry(m, m->createIndex(row, 0, groupItem), ent, rank, path, first + 1);
	}
}

void EsdbModel::refresh(bool useGroups)
{
	m_rootItem->clearPointers();
	int rank = 0;
	for (esdbEntry *ent : *m_entries) {
		QString path = ent->getPath();
		QStringList pathList;
		//TODO: handle "//"
		if (path.size()) {
			pathList = ent->getPath().split(QString('/'));
			if (!pathList.at(0).size()) {
				pathList.removeAt(0);
			}
		}
		if (useGroups && !pathList.size()) {
			pathList.append("Unsorted");
		}
		m_rootItem->refreshEntry(this, QModelIndex(), ent, rank, pathList, 0);
		rank++;
	}
	m_rootItem->cullEmptyItems(QModelIndex(), this);
	m_rootItem->sortItems();
}

class EsdbModelItemCompare {
public:
	bool operator()(const EsdbModelItem *r, const EsdbModelItem *l)
	{
		return r->rank() < l->rank();
	}
};

static EsdbModelItemCompare s_esdbModelItemCompare;

void EsdbModelGroupItem::sortItems()
{
	std::sort(m_items.begin(), m_items.end(), s_esdbModelItemCompare);
	for (EsdbModelItem *item : m_items) {
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *group = (EsdbModelGroupItem * )item;
			group->sortItems();
		}
	}
}

void EsdbModelGroupItem::cullEmptyItems(const QModelIndex &parent, EsdbModel *m)
{
	QList<EsdbModelItem *>::iterator iter = m_items.begin();
	int row = 0;
	while (iter != m_items.end()) {
		EsdbModelItem *item = *iter;
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *g = (EsdbModelGroupItem *)(item);
			g->cullEmptyItems(m->createIndex(row, 0, this), m);
			if (g->rowCount() == 0) {
				m->beginRemoveRows(parent, row, row);
				g->deleteLater();
				iter = m_items.erase(iter);
				m->endRemoveRows();
				continue;
			}
		} else {
			EsdbModelLeafItem *i = (EsdbModelLeafItem *)(item);
			if (!i->leafNode()) {
				m->beginRemoveRows(parent, row, row);
				i->deleteLater();
				iter = m_items.erase(iter);
				m->endRemoveRows();
				continue;
			}
		}
		row++;
		iter++;
	}
}

void EsdbModelGroupItem::clearPointers()
{
	for (EsdbModelItem *item : m_items) {
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *g = (EsdbModelGroupItem *)(item);
			g->clearPointers();
		} else {
			EsdbModelLeafItem *i = (EsdbModelLeafItem *)(item);
			i->setLeafEntry(NULL);
		}
	}
}

EsdbModel::EsdbModel(esdbTypeModule *module, QList<esdbEntry *> *entries) :
	m_entries(entries)
{
	Q_UNUSED(module);
	m_rootItem = new EsdbModelGroupItem("", -1, NULL);
	bool hasGroups = false;
	for (auto x : *m_entries) {
		esdbEntry *entry = x;
		if (entry->getPath().size()) {
			hasGroups = true;
		}
	}
	refresh(hasGroups);
}

int EsdbModel::rowCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
	EsdbModelItem *i;
	if (parent.isValid()) {
		i = (EsdbModelItem *)parent.internalPointer();
		if (i->isLeafItem()) {
			return 0;
		} else {
			return ((EsdbModelGroupItem *)i)->rowCount();
		}
	} else {
		return m_rootItem->rowCount();
	}
}

QModelIndex EsdbModel::index(int row)
{
	return index(row, 0, createIndex(row, 0, (void *)m_rootItem));
}

QModelIndex EsdbModel::index(int row, int column, const QModelIndex &parent) const
{
	EsdbModelGroupItem *g;
	if (!parent.isValid()) {
		g = m_rootItem;
	} else {
		g = (EsdbModelGroupItem *)parent.internalPointer();
	}

	EsdbModelItem *item = g->child(row);
	if (item) {
		return createIndex(row, column, (void *)g->child(row));
	} else {
		return QModelIndex();
	}
}

int EsdbModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}

QModelIndex EsdbModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();

	EsdbModelItem *childItem = (EsdbModelItem *)(child.internalPointer());
	EsdbModelItem *parentItem = childItem->parent();

	if (parentItem == m_rootItem)
	    return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

QVariant EsdbModel::data(const QModelIndex & index, int role) const
{
	EsdbModelItem *i = (EsdbModelItem *)index.internalPointer();
	if (i) {
		return i->data(role);
	} else {
		return QVariant();
	}
}

void EsdbModel::changed(bool useGroups)
{
	refresh(useGroups);
	emit layoutChanged();
}

QModelIndex EsdbModel::findEntry(EsdbModelGroupItem *g, esdbEntry *ent)
{
	int row = 0;
	for (auto item : g->m_items) {
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *g = (EsdbModelGroupItem *)(item);
			QModelIndex index = findEntry(g, ent);
			if (index.isValid()) {
				return index;
			}
		} else {
			EsdbModelLeafItem *l = (EsdbModelLeafItem *)(item);
			if (l->leafNode() == ent) {
				return createIndex(row, 0, l);
			}
		}
		row++;
	}
	return QModelIndex();
}

QModelIndex EsdbModel::findEntry(esdbEntry *ent)
{
	return findEntry(m_rootItem, ent);
}

int EsdbModelLeafItem::row()
{
	if (m_parent)
	    return m_parent->m_items.indexOf((EsdbModelItem*)(this));

	return 0;
}

int EsdbModelGroupItem::row()
{
	if (m_parent)
	    return m_parent->m_items.indexOf((EsdbModelItem*)(this));

	return 0;
}

QVariant EsdbModelLeafItem::data(int role)
{
	switch(role) {
	case Qt::DisplayRole:
		return QVariant(m_name);
	case Qt::DecorationPropertyRole:
	case Qt::DecorationRole:
		if (!m_icon.isNull()) {
			return m_icon;
		}
	}
	return QVariant();
}

QVariant EsdbModelGroupItem::data(int role)
{
	switch(role) {
	case Qt::DisplayRole:
		return QVariant(m_name);
	default:
		return QVariant();
	}
}
