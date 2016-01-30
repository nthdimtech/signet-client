#include "generictypedesc.h"

#include "esdb.h"

void genericTypeDesc::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->name);
	u8 numFields;
	numFields = blk->readU8();
	for (int i = 0; i < numFields; i++) {
		QString name;
		blk->readString(name);
		fields.push_back(name);
	}
}

void genericTypeDesc::toBlock(block *blk)
{
	esdbEntry::toBlock(blk);
	blk->writeString(this->name, false);
	blk->writeU8(fields.count());
	for (auto fld : fields) {
		blk->writeString(fld, true);
	}
}
