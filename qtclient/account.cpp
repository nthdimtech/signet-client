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

void account::fromBlock(block *blk)
{
	esdbEntry::fromBlock(blk);
	blk->readString(this->acctName);
	blk->readString(this->userName);
	blk->readString(this->password);
	blk->readString(this->url);
	blk->readString(this->email);
}

void account::toBlock(block *blk) const
{
	esdbEntry::toBlock(blk);
	blk->writeString(this->acctName, false);
	blk->writeString(this->userName, false);
	blk->writeString(this->password, true);
	blk->writeString(this->url, false);
	blk->writeString(this->email, false);
}


QString account::getTitle() const
{
	return acctName;
}

QString account::getUrl() const
{
	return url;
}

int account::matchQuality(const QString &search) const
{
	int quality = esdbEntry::matchQuality(search);
	if (quality && hasIcon()) {
		quality++;
	}
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

bool is_email(const QString &s)
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
