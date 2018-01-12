#include "genericfields.h"

#include "esdb.h"

genericFields_1::genericFields_1()
{

}

void genericFields_1::fromBlock(block *blk)
{
	u8 numFields;
	numFields = blk->readU8();
	m_fields.clear();
	for (int i = 0; i < numFields; i++) {
		genericField fld;
		blk->readString(fld.name);
		blk->readString(fld.value);
		m_fields.push_back(fld);
	}
}

void genericFields_1::toBlock(block *blk) const
{
	blk->writeU8(m_fields.count());
	for (auto fld : m_fields) {
		blk->writeString(fld.name, true);
		blk->writeString(fld.type, true);
		blk->writeString(fld.value, true);
	}
}

genericFields::genericFields()
{

}

void genericFields::fromBlock(block *blk)
{
	u8 numFields;
	numFields = blk->readU8();
	m_fields.clear();
	for (int i = 0; i < numFields; i++) {
		genericField fld;
		blk->readString(fld.name);
		blk->readString(fld.type);
		blk->readString(fld.value);
		m_fields.push_back(fld);
	}
}

void genericFields::toBlock(block *blk) const
{
	blk->writeU8(m_fields.count());
	for (auto fld : m_fields) {
		blk->writeString(fld.name, true);
		blk->writeString(fld.type, true);
		blk->writeString(fld.value, true);
	}
}

void genericFields::upgrade(const genericFields_1 &f)
{
	m_fields = f.m_fields;
}

void genericFields::getFields(QVector<genericField> &fields) const
{
	for (auto x : m_fields) {
		fields.push_back(x);
	}
}
