#include "account.h"

#define CROWD_SUPPLY_BIAS 1

void account_0::fromBlock(block *blk)
{
	esdbEntry_1::fromBlock(blk);
	blk->readString(this->acct_name);
	blk->readString(this->user_name);
	blk->readString(this->password);
}

void account_1::fromBlock(block *blk)
{
	esdbEntry_1::fromBlock(blk);
	blk->readString(this->acct_name);
	blk->readString(this->user_name);
	blk->readString(this->password);
	blk->readString(this->url);
}

void account_2::fromBlock(block *blk)
{
	esdbEntry_1::fromBlock(blk);
	blk->readString(this->acct_name);
	blk->readString(this->user_name);
	blk->readString(this->password);
	blk->readString(this->url);
	blk->readString(this->email);
}

void account_3::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->acctName);
	blk->readString(this->userName);
	blk->readString(this->password);
	blk->readString(this->url);
	blk->readString(this->email);
}

void account_4::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->acctName);
	blk->readString(this->userName);
	blk->readString(this->password);
	blk->readString(this->url);
	blk->readString(this->email);
	fields.fromBlock(blk);
}

void account_5::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->acctName);
	blk->readString(this->userName);
	blk->readString(this->password);
	blk->readString(this->url);
	blk->readString(this->email);
	fields.fromBlock(blk);
}

void account::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->path);
	blk->readString(this->acctName);
	blk->readString(this->userName);
	blk->readString(this->password);
	blk->readString(this->url);
	blk->readString(this->email);
	fields.fromBlock(blk);
}

void account::toBlock(block *blk) const
{
	esdbEntry::toBlock(blk);
	blk->writeString(this->path, false);
	blk->writeString(this->acctName, false);
	blk->writeString(this->userName, false);
	blk->writeString(this->password, true);
	blk->writeString(this->url, false);
	blk->writeString(this->email, false);
	fields.toBlock(blk);
}

void account::getFields(QVector<genericField> &fields_) const
{
	fields_.push_back(genericField("name", QString(), acctName));
	fields_.push_back(genericField("username", QString(), userName));
	fields_.push_back(genericField("password", QString(), password));
	fields_.push_back(genericField("url", QString(), url));
	fields_.push_back(genericField("email", QString(), email));
	fields.getFields(fields_);
}

int account::matchQuality(const QString &search) const
{
	int quality = esdbEntry::matchQuality(search);
	if (CROWD_SUPPLY_BIAS) {
		if (acctName == QString("Crowd Supply") && search.size() == 0) {
			quality += 100;
		}
	}
	return quality;
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QRegularExpression>
#endif

bool isEmail(const QString &s)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	QRegularExpression ra("^.+@.+\\..+");
	QRegularExpressionMatch match = ra.match(s);
	return match.hasMatch();
#else
	Q_UNUSED(s);
	return false; //TODO
#endif
}
