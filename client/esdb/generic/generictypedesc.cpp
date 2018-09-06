#include "generictypedesc.h"

void genericTypeDesc::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->group);
	blk->readString(this->name);
	int count = blk->readU8();
	for (int i = 0; i < count; i++) {
		QString fieldName;
		QString fieldType;
		blk->readString(fieldName);
		blk->readString(fieldType);
		fieldSpec fs(fieldName, fieldType);
		fields.push_back(fs);
	}
}

void genericTypeDesc::toBlock(block *blk) const
{
	esdbEntry::toBlock(blk);
	blk->writeString(this->group, false);
	blk->writeString(this->name, false);
	blk->writeU8(fields.size());
	for (auto f : fields) {
		blk->writeString(f.name, true);
		blk->writeString(f.type, true);
	}
}

void genericTypeDesc::getFields(QVector<genericField> &fields_) const
{
	fields_.push_back(genericField("name", "", name));
	fields_.push_back(genericField("path", "", group));
	for (auto f : fields) {
		fields_.push_back(genericField(f.name, "", f.type));
	}
}
