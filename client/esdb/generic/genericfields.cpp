#include "genericfields.h"

#include "esdb.h"

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


void genericFields_2::fromBlock(block *blk)
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

void genericFields::fromBlock(block *blk)
{
    u8 numFields = blk->readU8();
    m_fields.clear();
	for (int i = 0; i < numFields; i++) {
		genericField fld;
		blk->readString(fld.name);
		blk->readString(fld.type);
		blk->readLongString(fld.value);
		m_fields.push_back(fld);
	}
}

void genericFields::toBlock(block *blk) const
{
    u8 count = 0;
    for (auto fld : m_fields) {
        if (fld.value.size()) {
            count++;
        }
    }
    blk->writeU8(count);
	for (auto fld : m_fields) {
        if (fld.value.size()) {
            blk->writeString(fld.name, true);
            blk->writeString(fld.type, true);
            blk->writeLongString(fld.value, true);
        }
	}
}

const genericField *genericFields::getField(const QString &name) const
{
	for (const genericField &f : m_fields) {
		if (f.name == name) {
			return &f;
		}
	}
	return nullptr;
}

void genericFields::getFields(QVector<genericField> &fields) const
{
	for (auto x : m_fields) {
		fields.push_back(x);
	}
}
