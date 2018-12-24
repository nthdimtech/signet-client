#include "datatypelistmodel.h"
#include "esdbtypemodule.h"

QVariant DataTypeListModel::data(const QModelIndex &index, int role) const
{
	Q_UNUSED(role);
	if (!index.isValid()) {
		return QVariant();
	}
	int row = index.row();
	if (row < m_builtInTypes.size()) {
		return QVariant(m_builtInTypes.at(row)->name());
	} else if (row >= m_builtInTypes.size() && row < (m_builtInTypes.size() + m_dynamicTypes.size())) {
		return QVariant(m_dynamicTypes.at(row - m_builtInTypes.size())->name());
	} else {
		return QVariant();
	}
}

int DataTypeListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_dynamicTypes.size() + m_builtInTypes.size();
}

void DataTypeListModel::addModule(esdbTypeModule *m, bool builtIn)
{
	QModelIndex parent = QModelIndex();
	int rowCount_ = rowCount(parent);
	beginInsertRows(parent, rowCount_, rowCount_);
	if (builtIn) {
		m_builtInTypes.append(m);
	} else {
		m_dynamicTypes.append(m);
	}
	endInsertRows();
}

void DataTypeListModel::removeModule(esdbTypeModule *m)
{
	QModelIndex parent = QModelIndex();
	for (int i = 0; i < m_dynamicTypes.size(); i++) {
		if (m_dynamicTypes.at(i) == m) {
			int row = i + m_builtInTypes.size();
			beginRemoveRows(parent, row, row);
			m_dynamicTypes.removeAt(i);
			endRemoveRows();
			break;
		}
	}
}

void DataTypeListModel::moduleChanged(esdbTypeModule *m)
{
	for (int i = 0; i < m_dynamicTypes.size(); i++) {
		if (m_dynamicTypes.at(i) == m) {
			int row = i + m_builtInTypes.size();
			QVector<int> roles;
			emit dataChanged(createIndex(row, 0), createIndex(row, 0), roles);
			break;
		}
	}
}

DataTypeListModel::DataTypeListModel()
{

}
