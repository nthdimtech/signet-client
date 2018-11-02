#ifndef DATATYPELISTMODEL_H
#define DATATYPELISTMODEL_H

#include <QAbstractListModel>
#include <QList>

class esdbTypeModule;

class DataTypeListModel : public QAbstractListModel
{
	Q_OBJECT
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	QList<esdbTypeModule *> m_builtInTypes;
	QList<esdbTypeModule *> m_dynamicTypes;
public:
	void addModule(esdbTypeModule *m, bool builtIn);
	void removeModule(esdbTypeModule *m);
	DataTypeListModel();
};

#endif // DATATYPELISTMODEL_H
