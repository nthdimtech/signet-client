#include "esdb.h"

#include <QString>

void block::beginRead()
{
	index = 0;
}

void block::beginWrite()
{
	index = 0;
	data.resize(0);
}

void block::output_bytes_needed(int n)
{
	int mask_bytes_needed = (n + 7) / 8;
	if (mask_bytes_needed > mask.size()) {
		int prev_size = mask.size();
		mask.resize(mask_bytes_needed);
		for (int i = prev_size; i < mask_bytes_needed; i++) {
			mask[i] = 0;
		}
	}
	if (n > data.size()) {
		int prev_size = data.size();
		data.resize(n);
		for (int i = prev_size; i < n; i++) {
			data[i] = 0;
		}
	}
}

void block::setMask(int index, int len, bool val)
{
	u8 *d = (u8 *)(mask.data());
	for (int i = index; i < (index + len); i++) {
		int sub_bit = i % 8;
		int sub_byte = i / 8;
		d[sub_byte] = (d[sub_byte] & (~(1<<sub_bit))) | (val<<sub_bit);
	}
}

void block::readString(QString &str)
{
	int sz = readU8();
	QByteArray x = data.mid(index, sz);
	index += sz;
	str = QString::fromUtf8(x);
}

u8 block::readU8()
{
	return (u8)(data.data()[index++]);
}

u16 block::readU16()
{
	u16 ret = (u16)((u8 *)data.data())[index];
	ret += ((u16)((u8 *)data.data())[index + 1]) << 8;
	index += 2;
	return ret;
}

void block::writeU8(u8 v)
{
	output_bytes_needed(index + 1 + 1);
	((u8 *)data.data())[index++] = v;
}

void block::writeU16(u16 v)
{
	output_bytes_needed(index + 1 + 2);
	((u8 *)data.data())[index++] = v & 0xff;
	((u8 *)data.data())[index++] = v >> 8;
}

void block::writeString(const QString &str, bool masked)
{
	QByteArray x = str.toUtf8();
	writeU8(x.size());
	output_bytes_needed(index + 1 + x.size());
	memcpy(data.data() + index, x.data(), x.size());
	setMask(index, x.size(), masked);
	index += x.size();
}

esdbEntry_1::~esdbEntry_1()
{

}

esdbEntry_1::esdbEntry_1(int id_, int type_, int revision_) :
	id(id_), type(type_), revision(revision_)
{

}

void esdbEntry_1::fromBlock(block *blk)
{
	blk->beginRead();
	type = blk->readU16();
	revision = blk->readU16();
}

esdbEntry::~esdbEntry()
{

}

esdbEntry::esdbEntry(int id_, int type_, int revision_, int uid_, int version_) :
	id(id_), type(type_), revision(revision_), uid(uid_),
	version(version_),
	iconSet(false)
{

}

esdbEntry::esdbEntry(int id_) :
	id(id_),
	iconSet(false)
{

}

void esdbEntry::fromBlock(block *blk)
{
	blk->beginRead();
	type = blk->readU16();
	revision = blk->readU16();
	uid = blk->readU16();
	version = blk->readU16();
}

void esdbEntry::toBlock(block *blk) const
{
	blk->beginWrite();
	blk->writeU16(type);
	blk->writeU16(revision);
	blk->writeU16(uid);
	blk->writeU16(version);
}

int esdbEntry::matchQuality(const QString &search) const
{
	QString title = getTitle();
	int quality = 0;
	if (title.startsWith(search, Qt::CaseInsensitive)) {
		quality += 2;
	}else if(title.contains(search, Qt::CaseInsensitive)) {
		quality += 1;
	}
	return quality;
}
