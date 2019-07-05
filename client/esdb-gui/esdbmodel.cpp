#include "esdbmodel.h"
#include "esdbtypemodule.h"

#include <QModelIndex>
#include <QIcon>
#include <QTreeView>

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
        EsdbModelLeafItem *leafItem = nullptr;
		QList<EsdbModelLeafItem *>::iterator iter = m_hiddenItems.begin();
		while (iter != m_hiddenItems.end()) {
			if ((*iter)->name() == ent->getTitle()) {
				leafItem = *iter;
				leafItem->setLeafEntry(ent);
				leafItem->setRank(rank);
				m_hiddenItems.erase(iter);
				break;
			}
			iter++;
		}
		if (!leafItem) {
			leafItem = new EsdbModelLeafItem(ent, rank, this);
		}
		m->beginInsertRows(parent, row, row);
		m_items.append(leafItem);
		m->endInsertRows();
	} else {
		int row = 0;
		for (EsdbModelItem *item : m_items) {
			if (!item->isLeafItem()) {
				EsdbModelGroupItem *groupItem = (EsdbModelGroupItem *) item;
				if (groupItem->name() == path.at(first)) {
					if (path.at(0) == "Unsorted") {
						groupItem->setRank(-1);
					} else {
						groupItem->setRank(rank);
					}
					groupItem->refreshEntry(m, m->createIndex(row, 0, groupItem), ent, rank, path, first + 1);
					return;
				}
			}
			row++;
		}
        EsdbModelGroupItem *groupItem = nullptr;
		QList<EsdbModelGroupItem *>::iterator iter = m_hiddenGroups.begin();
		while (iter != m_hiddenGroups.end()) {
			if ((*iter)->name() == path.at(first)) {
				groupItem = *iter;
				m_hiddenGroups.erase(iter);
				break;
			}
			iter++;
		}
		if (!groupItem) {
			groupItem = new EsdbModelGroupItem(path.at(first), rank, this);
		}
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
		QStringList pathListInitial;
		QStringList pathListFinal;

		QString pathEscaped = path;

		if (path.size()) {
			pathListInitial = ent->getPath().split(QString('/'));
			if (!path.startsWith("//")) {
				if (pathListInitial.size() && !pathListInitial.at(0).size()) {
					pathListInitial.removeAt(0);
				}
			}
		}

		bool appendNext = false;

		QString pathItem;
		for (int i = 0; i < pathListInitial.size(); i++) {
			QString itemSegment = pathListInitial.at(i);
			if (!itemSegment.size()) {
				pathItem.append("/");
				appendNext = true;
			} else if (appendNext) {
				pathItem.append(itemSegment);
				appendNext = false;
			} else {
				if (pathItem.size()) {
					pathListFinal.append(pathItem);
				}
				pathItem = itemSegment;
			}
		}
		if (pathItem.size()) {
			pathListFinal.append(pathItem);
		}

		if (useGroups && !pathListFinal.size()) {
			pathListFinal.append("Unsorted");
		}
		m_rootItem->refreshEntry(this, QModelIndex(), ent, rank, pathListFinal, 0);
		rank++;
	}
	m_rootItem->cullEmptyItems(QModelIndex(), this);
	createUnsortedGroups(QModelIndex(), m_rootItem);
	m_rootItem->sortItems();
}

class EsdbModelItemCompare
{
public:
	bool operator()(const EsdbModelItem *r, const EsdbModelItem *l)
	{
        if (r->isLeafItem() == l->isLeafItem()) {
            return r->name().compare(l->name(), Qt::CaseInsensitive) < 0;
        } else if (!r->isLeafItem()) {
            return false;
        } else
            return true;
	}
};

static EsdbModelItemCompare s_esdbModelItemCompare;

void EsdbModel::createUnsortedGroups(QModelIndex idx, EsdbModelGroupItem *g)
{
	bool hasLeaf = false;
	bool hasGroup = false;
	for (EsdbModelItem *item : g->m_items) {
		if (!item->isLeafItem()) {
			hasGroup = true;
		} else {
			hasLeaf = true;
		}
	}
	if (hasGroup && hasLeaf) {
		EsdbModelGroupItem *unsortedGroup = new EsdbModelGroupItem("Unsorted", 0, g);
		g->m_items.push_front(unsortedGroup);
		int row = 0;
		int destRow = 0;
		QList<EsdbModelItem *>::iterator iter = g->m_items.begin();
		while (iter != g->m_items.end()) {
			EsdbModelItem *item = *iter;
			if (item->isLeafItem()) {
				EsdbModelLeafItem *leaf = (EsdbModelLeafItem *)item;
				beginMoveRows(idx, row, row, createIndex(0, 0, unsortedGroup), destRow);
				unsortedGroup->m_items.push_back(leaf);
				leaf->m_parent = unsortedGroup;
				iter = g->m_items.erase(iter);
				endMoveRows();
				destRow++;
			} else {
				row++;
				iter++;
			}
		}
	}
	int row = 0;
	for (EsdbModelItem *item : g->m_items) {
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *child = (EsdbModelGroupItem *)item;
			createUnsortedGroups(createIndex(row, 0, child), child);
		}
		row++;
	}
}

void EsdbModelGroupItem::sortItems()
{
	for (EsdbModelItem *item : m_items) {
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *group = (EsdbModelGroupItem * )item;
			group->sortItems();
			group->setRank(group->m_items.first()->rank());
		}
	}
	std::sort(m_items.begin(), m_items.end(), s_esdbModelItemCompare);
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
				iter = m_items.erase(iter);
				m_hiddenGroups.append(g);
				m->endRemoveRows();
				continue;
			}
		} else {
			EsdbModelLeafItem *i = (EsdbModelLeafItem *)(item);
			if (!i->leafNode()) {
				m->beginRemoveRows(parent, row, row);
				iter = m_items.erase(iter);
				m_hiddenItems.append(i);
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
    m_rootItem = new EsdbModelGroupItem("", -1, nullptr, true);
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
		return i->rowCount();
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
		EsdbModelItem *item = (EsdbModelItem *)parent.internalPointer();
		if (item->isLeafItem()) {
			return QModelIndex();
		}
		g = (EsdbModelGroupItem *)item;
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

QModelIndex EsdbModel::findEntry(EsdbModelGroupItem *g, const esdbEntry *ent) const
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

QModelIndex EsdbModel::findEntry(const esdbEntry *ent) const
{
	return findEntry(m_rootItem, ent);
}

void EsdbModel::syncExpanded(QTreeView *v, QModelIndex &parent, EsdbModelGroupItem *group)
{
	int rows = rowCount(parent);
	if (group->isExpanded()) {
		v->expand(parent);
	} else {
		v->collapse(parent);
	}
	for (int i = 0; i < rows; i++) {
		QModelIndex child = index(i, 0, parent);
		EsdbModelItem *item = (EsdbModelItem *)(child.internalPointer());
		if (!item->isLeafItem()) {
			EsdbModelGroupItem *g = (EsdbModelGroupItem *)(item);
			syncExpanded(v, child, g);
		}
	}
}

void EsdbModel::syncExpanded(QTreeView *v)
{
	QModelIndex i;
	syncExpanded(v, i, m_rootItem);
}

void EsdbModel::expand(QModelIndex &index, bool expand)
{
	EsdbModelItem *item = (EsdbModelItem *)(index.internalPointer());
	if (!item->isLeafItem()) {
		EsdbModelGroupItem *g = (EsdbModelGroupItem *)(item);
		g->expanded(expand);
	}
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
