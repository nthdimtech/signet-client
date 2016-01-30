#ifndef ACCOUNTMODEL_H
#define ACCOUNTMODEL_H
#include <QList>
#include <QIcon>

#include "account.h"

#include <QAbstractListModel>

class esdbTypeModule;
struct esdbModelGroup;

class EsdbModel : public QAbstractItemModel
{
public:
	EsdbModel(esdbTypeModule *module, QList<esdbEntry *> *entries);
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	void changed();
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &child) const;
	QModelIndex index(int row);
private:
	QList<esdbEntry *> *m_accounts;
	QList<esdbModelGroup *> m_groups;
};

#endif // ACCOUNTMODEL_H
