#include "bookmark.h"

#include "esdb.h"

void bookmark::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->name);
	blk->readString(this->url);
}

void bookmark::toBlock(block *blk) const
{
    esdbEntry::toBlock(blk);
	blk->writeString(this->name, false);
	blk->writeString(this->url, false);
}

int bookmark::matchQuality(const QString &search) const
{
	return esdbEntry::matchQuality(search);
}
