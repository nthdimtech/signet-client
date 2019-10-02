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

size_t block::dataRemaining() const
{
	return data.size() - index;
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

void block::readLongString(QString &str)
{
	int sz = readU16();
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
	((u8 *)data.data())[index] = v;
	index++;
}

void block::writeU16(u16 v)
{
	output_bytes_needed(index + 1 + 2);
	((u8 *)data.data())[index] = v & 0xff;
	((u8 *)data.data())[index + 1] = v >> 8;
	index += 2;
}

void block::writeString(const QString &str, bool masked)
{
	QByteArray x = str.toUtf8();
	u8 sz = x.size() > 255 ? 255 : x.size();
	writeU8(sz);
	output_bytes_needed(index + 1 + sz);
	memcpy(data.data() + index, x.data(), sz);
	setMask(index, sz, masked);
	index += sz;
}

void block::writeLongString(const QString &str, bool masked)
{
	QByteArray x = str.toUtf8();
	u16 sz = x.size() > ((1<<16) - 1) ? ((1<<16) - 1) : x.size();
	writeU16(sz);
	output_bytes_needed(index + 1 + sz);
	memcpy(data.data() + index, x.data(), sz);
	setMask(index, sz, masked);
	index += sz;
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


int esdbEntry::matchLocation(const QString &search, bool &wordStart, int &wordLocation) const
{
	QString title = getTitle();
	if (!search.size()) {
		return 0;
	}
	int wordStartMatch = -1;
	int wordEndMatch = -1;
	int wordCount = 1;
	int index = -1;
	for (int i = 0; i < title.size(); i++) {
		int j = 0;
		for (j = 0; j < search.size() && (j + i) < title.size(); j++) {
			if (title.at(i+j).toLower() != search.at(j).toLower()) {
				break;
			}
		}
		if (j == search.size()) {
			int wordEndMatchPrev = wordEndMatch;
			if (i == 0) {
				wordStartMatch = wordCount;
			} else if (title.at(i).isUpper()) {
				if (!title.at(i-1).isUpper())
					wordStartMatch = wordCount;
			} else if (title.at(i).isLower()) {
				if (!title.at(i-1).isLetter())
					wordStartMatch = wordCount;
			} else if (title.at(i).isNumber()) {
				if (!title.at(i-1).isNumber())
					wordStartMatch = wordCount;
			} else if (!title.at(i).isSpace()) {
				if (title.at(i-1).isLetterOrNumber() || title.at(i-1).isSpace())
					wordStartMatch = wordCount;
			}

			int endMark = i + search.size() - 1;
			if (endMark == title.size() - 1) {
				wordEndMatch = wordCount;
			} else if (title.at(endMark + 1).isSpace()) {
				wordEndMatch = wordCount;
			}
			if (wordStartMatch > 0) {
				index = i;
				break;
			} else if (wordEndMatchPrev < 0 && wordEndMatch > 0) {
				index = i;
			}
		}
		if (i) {
			if (!title.at(i).isUpper()) {
				if (!title.at(i-1).isUpper())
					wordCount++;
			} else if (title.at(i).isLower()) {
				if (!title.at(i-1).isLetter())
					wordCount++;
			} else if (title.at(i).isNumber()) {
				if (!title.at(i-1).isNumber())
					wordCount++;
			}
		}
	}
	if (wordStartMatch > 0) {
		wordStart = true;
		wordLocation = wordStartMatch;
	} else if (wordEndMatch > 0) {
		wordStart = false;
		wordLocation = wordStartMatch;
	}
	return index;
}

int esdbEntry::matchQuality(const QString &search) const
{
	QString title = getTitle();
	int quality = 0;
	if (!search.size()) {
		quality = 20;
		return quality;
	}
	if (!title.compare(search, Qt::CaseInsensitive)) {
		quality = 20;
		return quality;
	}

	bool wordStart;
	int wordLocation;
	int index = matchLocation(search, wordStart, wordLocation);

	if (wordLocation > 9) {
		wordLocation = 9;
	}

	if (index >= 0) {
		if (wordStart) {
			quality = 20 - wordLocation;
		} else {
			quality = 10 - wordLocation;
		}
	}
	return quality;
}
