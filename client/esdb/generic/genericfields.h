#ifndef GENERICFIELDS_H
#define GENERICFIELDS_H

#include <QString>
#include <QList>

#include "esdb.h"

class genericFields;

class genericFields_1
{
	QList<genericField> m_fields;
	friend class genericFields_2;
public:
	genericFields_1() {}
	void fromBlock(block *blk);
};

class genericFields_2
{
public:
	genericFields_2() {}
	QList<genericField> m_fields;
	void fromBlock(block *blk);
	void upgrade(const genericFields_1 &f)
	{
		m_fields = f.m_fields;
	}
};

class genericFields
{
public:
	genericFields() {}
	QList<genericField> m_fields;
	void fromBlock(block *blk);
	void clear()
	{
		m_fields.clear();
	}
	void toBlock(block *blk) const;
	int fieldCount() const
	{
		return m_fields.count();
	}
	genericField getField(int i) const
	{
		return m_fields.at(i);
	}
	void replaceField(int i, const genericField &f)
	{
		m_fields.replace(i, f);
	}
	void removeField(int i)
	{
		m_fields.removeAt(i);
	}
	void addField(const genericField &f)
	{
		m_fields.push_back(f);
	}

	const genericField *getField(const QString &name) const;

	void getFields(QVector<genericField> &fields) const;

	void upgrade(const genericFields_2 &f)
	{
		m_fields = f.m_fields;
	}
};

#endif // GENERICFIELDS_H
