#ifndef GENERICFIELDS_H
#define GENERICFIELDS_H

#include <QString>
#include <QList>

struct block;

struct genericField {
	QString name;
	QString value;
};

class genericFields
{
	QList<genericField> m_fields;
public:
	genericFields();
	void fromBlock(block *blk);
	void toBlock(block *blk) const;
	int fieldCount() const {
		return m_fields.count();
	}
	genericField getField(int i) const {
		return m_fields.at(i);
	}
	void replaceField(int i, const genericField &f) {
		m_fields.replace(i, f);
	}
	void removeField(int i) {
		m_fields.removeAt(i);
	}
	void addField(const genericField &f) {
		m_fields.push_back(f);
	}
};

#endif // GENERICFIELDS_H
