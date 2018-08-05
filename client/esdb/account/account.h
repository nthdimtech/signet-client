#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>
#include <QIcon>
#include "esdb.h"
#include "genericfields.h"

struct block;

struct account_0 : public esdbEntry_1 {
	QString acct_name;
	QString user_name;
	QString password;
	void fromBlock(block *blk);
	account_0(int id_) : esdbEntry_1(id_, ESDB_TYPE_ACCOUNT, 0) {}
	~account_0() {}
};

struct account_1 : public esdbEntry_1 {
	QString acct_name;
	QString user_name;
	QString password;
	QString url;
	void fromBlock(block *blk);
	account_1(int id_) : esdbEntry_1(id_, ESDB_TYPE_ACCOUNT, 1) {}
	~account_1() {}
	void upgrade(account_0 &prev)
	{
		acct_name = prev.acct_name;
		user_name = prev.user_name;
		password = prev.password;
	}
};

bool isEmail(const QString &s);

struct account_2 : public esdbEntry_1 {
	QString acct_name;
	QString user_name;
	QString password;
	QString url;
	QString email;
	QIcon icon;
	bool hasIcon;
	void fromBlock(block *blk);
	account_2(int id_) : esdbEntry_1(id_, ESDB_TYPE_ACCOUNT, 2),
		hasIcon(false) {}
	~account_2() {}
	void upgrade(account_1 &prev)
	{
		acct_name = prev.acct_name;
		user_name = prev.user_name;
		password = prev.password;
		url = prev.url;
		if (isEmail(prev.user_name)) {
			email = prev.user_name;
		}
	}
};

struct account_3 : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	void fromBlock(block *blk);
	account_3(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 3, id_, 1) {}
	~account_3() {}
	void upgrade(account_2 &prev)
	{
		acctName = prev.acct_name;
		userName = prev.user_name;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		uid = id;
	}
};

struct account_4 : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	genericFields_1 fields;
	void fromBlock(block *blk);
	account_4(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 4, id_, 1) {}
	~account_4() {}
	void upgrade(account_3 &prev)
	{
		acctName = prev.acctName;
		userName = prev.userName;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		uid = id;
	}
};

struct account_5 : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	genericFields_2 fields;
	void fromBlock(block *blk);
	account_5(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 5, id_, 1) {}
	~account_5() {}
	void upgrade(account_4 &prev)
	{
		acctName = prev.acctName;
		userName = prev.userName;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		uid = id;
		fields.upgrade(prev.fields);
	}
};

struct account_6 : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	QString path;
	genericFields_2 fields;
	void fromBlock(block *blk);
	account_6(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 6, id_, 1) {}
	~account_6() {}
	void upgrade(account_5 &prev)
	{
		acctName = prev.acctName;
		userName = prev.userName;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		uid = id;
		fields = prev.fields;
		QList<genericField>::iterator iter = fields.m_fields.begin();
		while (iter != fields.m_fields.end()) {
			if ((*iter).name == "path") {
				path = (*iter).value;
				iter = fields.m_fields.erase(iter);
				continue;
			}
			iter++;
		}
	}
};

struct account : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	QString path;
	genericFields fields;
	void fromBlock(block *blk);
	void toBlock(block *blk) const;
	account(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 7, id_, 1)
	{
	}

	QString getTitle() const {
		return acctName;
	}
	QString getUrl() const {
		return url;
	}
	int matchQuality(const QString &search) const;

	QString getPath() const {
		return path;
	}

	void setTitle(const QString &title) {
		acctName = title;
	}

	void upgrade(account_6 &prev)
	{
		acctName = prev.acctName;
		userName = prev.userName;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		path = prev.path;
		uid = id;
		fields.upgrade(prev.fields);
	}

	void getFields(QVector<genericField> &fields) const;

	void setPath(QString &p) {
		path = p;
	}

	~account() {}
};

#endif // ACCOUNT_H
