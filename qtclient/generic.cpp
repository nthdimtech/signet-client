#include "generic.h"

#include "esdb.h"

void generic_1::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->typeName);
	blk->readString(this->name);
	fields.fromBlock(blk);
}

void generic::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->typeName);
	blk->readString(this->name);
	fields.fromBlock(blk);
}

void generic::toBlock(block *blk)
{
	esdbEntry::toBlock(blk);
	blk->writeString(this->typeName, false);
	blk->writeString(this->name, false);
	fields.toBlock(blk);
}

int generic::matchQuality(const QString &search)
{
	int quality = esdbEntry::matchQuality(search);
	if (quality && hasIcon()) {
		quality++;
	}
	return quality;
}
