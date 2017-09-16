#include "genericfields.h"

#include "esdb.h"

genericFields::genericFields()
{

}

void genericFields::fromBlock(block *blk)
{
	u8 numFields;
	numFields = blk->readU8();
	for (int i = 0; i < numFields; i++) {
		genericField fld;
		blk->readString(fld.name);
		blk->readString(fld.value);
		m_fields.push_back(fld);
	}
}

void genericFields::toBlock(block *blk) const
{
	blk->writeU8(m_fields.count());
	for (auto fld : m_fields) {
		blk->writeString(fld.name, true);
		blk->writeString(fld.value, true);
	}
}
