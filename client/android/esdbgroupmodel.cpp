#include "esdbgroupmodel.h"

EsdbGroupModel::EsdbGroupModel(QStringList *entries) :
	m_entries(entries)
{
}

QVariant EsdbGroupModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		if (index.row() < m_entries->size()) {
			return m_entries->at(index.row());
		}
	}
	return QVariant();
}

int EsdbGroupModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_entries->length();
}

QString EsdbGroupModel::text(int index)
{
	if (index != -1) {
		return m_entries->at(index);
	} else {
		return QString();
	}
}
