#include "esdbtypemodule.h"
#include "esdbmodel.h"
#include "esdbactionbar.h"

esdbTypeModule::esdbTypeModule(const QString &name) :
	m_name(name)
{
}

QVector<QStringList::const_iterator> esdbTypeModule::aliasMatch(const QVector<QStringList> &aliasedFields, const QStringList &fields) const
{
	QVector<QStringList::const_iterator> aliasMatched;

	aliasMatched.resize(aliasedFields.size());

	for (int i = 0; i < aliasedFields.size(); i++) {
		aliasMatched.replace(i, aliasedFields.at(i).cend());
	}

	int j = 0;
	for (const QStringList &aliasedField : aliasedFields) {
		for (int i = 0; i < fields.size(); i++) {
			for (QStringList::const_iterator iter = aliasedField.cbegin(); iter < aliasMatched.at(j); iter++) {
				if (!fields.at(i).compare(*iter, Qt::CaseInsensitive)) {
					aliasMatched.replace(j, iter);
					break;
				}
			}
		}
		j++;
	}

	return aliasMatched;
}


QVector<QString> esdbTypeModule::aliasMatchValues(const QVector<QStringList> &aliasedFields, const QVector<QStringList::const_iterator> &aliasMatched, const QVector<genericField> &fields, genericFields *genFields) const
{
	QVector<QString> fieldValues;
	fieldValues.resize(aliasMatched.size());
	for (genericField f : fields) {
		bool matched = false;
		int i = 0;
		for (QStringList::const_iterator iter : aliasMatched) {
			if (iter == aliasedFields.at(i).cend()) {
				i++;
				continue;
			}
			const QString &aliasName = *iter;
			if (!f.name.compare(aliasName, Qt::CaseInsensitive)) {
				fieldValues[i] = f.value;
				matched = true;
			}
			i++;
		}
		if (!matched && genFields) {
			genFields->addField(f);
		}
	}
	return fieldValues;
}
