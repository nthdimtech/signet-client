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
	account_0(int id_) : esdbEntry_1(id_, ESDB_TYPE_ACCOUNT, 0)
	{
	}
	~account_0() {}
};

struct account_1 : public esdbEntry_1 {
	QString acct_name;
	QString user_name;
	QString password;
	QString url;
	void fromBlock(block *blk);
	account_1(int id_) : esdbEntry_1(id_, ESDB_TYPE_ACCOUNT, 1)
	{
	}

	void upgrade(account_0 &prev)
	{
		acct_name = prev.acct_name;
		user_name = prev.user_name;
		password = prev.password;
	}

	~account_1() {}
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
		hasIcon(false)
	{
	}

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

	~account_2() {}
};

struct account_3 : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	void fromBlock(block *blk);
	account_3(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 3, id_, 1)
	{
	}
	void setTitle(const QString &title) {
		acctName = title;
	}

	void upgrade(account_2 &prev)
	{
		acctName = prev.acct_name;
		userName = prev.user_name;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		uid = id;
	}

	~account_3() {}
};

struct account_4 : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	genericFields_1 fields;
	void fromBlock(block *blk);
	account_4(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 4, id_, 1)
	{
	}
	void setTitle(const QString &title) {
		acctName = title;
	}
	void upgrade(account_3 &prev)
	{
		acctName = prev.acctName;
		userName = prev.userName;
		password = prev.password;
		email = prev.email;
		url = prev.url;
		uid = id;
	}

	~account_4() {}
};

struct account : public esdbEntry {
	QString acctName;
	QString userName;
	QString password;
	QString url;
	QString email;
	genericFields fields;
	void fromBlock(block *blk);
	void toBlock(block *blk) const;
	account(int id_) : esdbEntry(id_, ESDB_TYPE_ACCOUNT, 5, id_, 1)
	{
	}

	QString getTitle() const;
	QString getUrl() const;
	int matchQuality(const QString &search) const;

	void setTitle(const QString &title) {
		acctName = title;
	}

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

	void getFields(QVector<genericField> &fields) const;


	~account() {}
};

#endif // ACCOUNT_H
