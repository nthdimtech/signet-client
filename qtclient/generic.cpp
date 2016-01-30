#include "generic.h"

#include "esdb.h"

void generic::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->typeName);
	blk->readString(this->name);
	u8 numFields;
	numFields = blk->readU8();
	for (int i = 0; i < numFields; i++) {
		genericField fld;
		blk->readString(fld.name);
		blk->readString(fld.value);
		fields.push_back(fld);
	}
}

void generic::toBlock(block *blk)
{
	esdbEntry::toBlock(blk);
	blk->writeString(this->typeName, false);
	blk->writeString(this->name, false);
	blk->writeU8(fields.count());
	for (auto fld : fields) {
		blk->writeString(fld.name, true);
		blk->writeString(fld.value, true);
	}
}

int generic::matchQuality(const QString &search)
{
	int quality = esdbEntry::matchQuality(search);
	if (quality && hasIcon()) {
		quality++;
	}
	return quality;
}
