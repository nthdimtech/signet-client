#ifndef ESDB_H
#define ESDB_H

#include <vector>
#include <QByteArray>
#include <QString>
#include <QIcon>
#include <QVector>

extern "C" {
#include "common.h"
}

struct genericField {
	QString name;
	QString type;
	QString value;
	genericField(QString _name, QString _type, QString _value) :
		name(_name),
		type(_type),
		value(_value) {
	}
	genericField() {}
};

struct block {
	int index;
	void setMask(int index, int len, bool val);
	void output_bytes_needed(int n);
	void beginRead();
	void beginWrite();
	u8 readU8();
	u16 readU16();
	void writeU8(u8 v);
	void writeU16(u16 v);
	void readString(QString &str);
	void writeString(const QString &str, bool masked);
	QByteArray data;
	QByteArray mask;
};

struct esdbEntry_1 {
	int id;
	u16 type;
	u16 revision;
	virtual void fromBlock(block *blk);
	virtual ~esdbEntry_1();
	esdbEntry_1(int id_, int type_, int revision_);
	esdbEntry_1(int id_) : id(id_) {}
	virtual QString getTitle()
	{
		return QString();
	}
	virtual QString getUrl()
	{
		return QString();
	}
};

struct esdbEntry {
	int id;
	u16 type;
	u16 revision;
	u16 uid;
	u16 version;
	bool iconSet;
	QIcon icon;
	virtual void fromBlock(block *blk);
	virtual void toBlock(block *blk) const;
	virtual ~esdbEntry();
	esdbEntry(int id_, int type_, int revision_, int uid_, int version_);
	esdbEntry(int id_);
	virtual QString getTitle() const
	{
		return QString();
	}
	virtual QString getUrl() const
	{
		return QString();
	}

	bool hasIcon() const
	{
		return iconSet;
	}

	QIcon getIcon()
	{
		return icon;
	}

	void setIcon(QIcon newIcon)
	{
		icon = newIcon;
		iconSet = true;
	}

	void clearIcon()
	{
		icon = QIcon();
		iconSet = false;
	}

	virtual void getFields(QVector<genericField> &fields) const {
		Q_UNUSED(fields);
	}

	virtual int matchQuality(const QString &search) const;
};

enum esdbTypes {
	ESDB_TYPE_ACCOUNT,
	ESDB_TYPE_BOOKMARK,
	ESDB_TYPE_GENERIC,
	ESDB_TYPE_GENERIC_TYPE_DESC,
	EDDB_NUM_TYPES
};

#endif // ESDB_H
