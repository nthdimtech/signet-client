#include "generic.h"

#include "esdb.h"

void generic_1::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->typeName);
	blk->readString(this->name);
	fields.fromBlock(blk);
}

void generic_2::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->typeName);
	blk->readString(this->name);
	fields.fromBlock(blk);
}

void generic_3::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->typeName);
	blk->readString(this->name);
	fields.fromBlock(blk);
}

void generic::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	this->typeId = blk->readU16();
	blk->readString(this->name);
	fields.fromBlock(blk);
}

void generic::toBlock(block *blk) const
{
	esdbEntry::toBlock(blk);
	blk->writeU16(this->typeId);
	blk->writeString(this->name, false);
	fields.toBlock(blk);
}

int generic::matchQuality(const QString &search) const
{
	return esdbEntry::matchQuality(search);
}

void generic::getFields(QVector<genericField> &fields_) const
{
	fields_.push_back(genericField("name", "", name));
	fields.getFields(fields_);
}
