#ifndef ESDBGROUPMODEL_H
#define ESDBGROUPMODEL_H

#include <QAbstractListModel>

class EsdbGroupModel : public QAbstractListModel
{
	Q_OBJECT
	QStringList *m_entries;
public:
	EsdbGroupModel(QStringList *entries);
	QVariant data(const QModelIndex &index, int role) const;
	int rowCount(const QModelIndex &parent) const;
public slots:
	QString text(int index);
};
#endif // ESDBGROUPMODEL_H
