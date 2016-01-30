#include "esdbmodel.h"
#include <QModelIndex>
#include <QDebug>
#include <QIcon>
#include "esdbtypemodule.h"

struct esdbModelGroup {
	QString categoryName;
	QList<esdbEntry *> entries;
};

EsdbModel::EsdbModel(esdbTypeModule *module, QList<esdbEntry *> *accounts) :
	m_accounts(accounts)
{
	Q_UNUSED(module);
}

int EsdbModel::rowCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);

	if (parent.isValid()) {
		return 0;
	} else {
		return m_accounts->size();
	}
}

QModelIndex EsdbModel::index(int row)
{
	return index(row, 0, QModelIndex());
}

QModelIndex EsdbModel::index(int row, int column, const QModelIndex &parent)
const
{
	if (parent.isValid()) {
		return QModelIndex();
	} else {
		if (row < rowCount(parent)) {
			return createIndex(row, column, row);
		} else {
			return QModelIndex();
		}
	}
}

int EsdbModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return 1;
}

QModelIndex EsdbModel::parent(const QModelIndex &child) const
{
	Q_UNUSED(child);
	return QModelIndex();
}

QVariant EsdbModel::data(const QModelIndex & index, int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}
	int row = index.row();

	int n_accounts = m_accounts->size();
	if (row >= n_accounts) {
		return QVariant();
	}
	esdbEntry *acct = m_accounts->at(row);
	QString name = acct->getTitle();
	switch(role) {
	case Qt::DisplayRole:
		return QVariant(name);
	case Qt::DecorationRole: {
		if (acct->hasIcon()) {
			return acct->getIcon();
		}
		return QVariant();
	}
	break;
	default:
		return QVariant();
	}
}

void EsdbModel::changed()
{
	emit layoutChanged();
}
