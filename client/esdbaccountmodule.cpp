#include "esdbaccountmodule.h"
#include "accountactionbar.h"

#include "esdb.h"
#include "account.h"

EsdbActionBar *esdbAccountModule::newActionBar()
{
	return new AccountActionBar(m_parent);
}

esdbEntry *esdbAccountModule::decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const
{
	esdbEntry *entry = NULL;
	account *acct = NULL;
	account_0 rev0(id);
	account_1 rev1(id);
	account_2 rev2(id);
	account_3 rev3(id);
	account_4 rev4(id);

	if (!prev) {
		acct = new account(id);
	} else {
		acct = static_cast<account *>(prev);
	}

	switch(revision) {
	case 0:
		rev0.fromBlock(blk);
		break;
	case 1:
		rev1.fromBlock(blk);
		break;
	case 2:
		rev2.fromBlock(blk);
		break;
	case 3:
		rev3.fromBlock(blk);
		break;
	case 4:
		rev4.fromBlock(blk);
		break;
	case 5:
		acct->fromBlock(blk);
		break;
	}

	switch(revision) {
	case 0:
		rev1.upgrade(rev0);
	case 1:
		rev2.upgrade(rev1);
	case 2:
		rev3.upgrade(rev2);
	case 3:
		rev4.upgrade(rev3);
	case 4:
		acct->upgrade(rev4);
	case 5:
		break;
	default:
		delete acct;
		acct = NULL;
		break;
	}
	entry = acct;
	return entry;
}

QVector<QStringList::const_iterator> esdbAccountModule::aliasMatch(const QVector<QStringList> &aliasedFields, const QStringList &fields) const
{
	QVector<QStringList::const_iterator> aliasMatched;

	aliasMatched.resize(aliasedFields.size());

	for (int i = 0; i < aliasedFields.size(); i++) {
		aliasMatched.replace(i, aliasedFields.at(i).cend());
	}

	int j = 0;
	for (const QStringList &aliasedField : aliasedFields) {
		for (int i = 0; i < fields.size(); i++) {
			for (QStringList::const_iterator iter = aliasedField.cbegin(); iter < aliasMatched.at(j); iter++) {
				if (!fields.at(i).compare(*iter, Qt::CaseInsensitive)) {
					aliasMatched.replace(j, iter);
					break;
				}
			}
		}
		j++;
	}

	return aliasMatched;
}

esdbEntry *esdbAccountModule::decodeEntry(const QVector<genericField> &fields, bool doAliasMatch) const
{
	account *acct = new account(-1);
	QVector<QStringList> aliasedFields;

	QStringList acctNameAliases;
	acctNameAliases.push_back("name");
	acctNameAliases.push_back("title");
	acctNameAliases.push_back("account");
	acctNameAliases.push_back("url");
	acctNameAliases.push_back("address");

	QStringList usernameAliases;
	usernameAliases.push_back("username");
	usernameAliases.push_back("user");
	usernameAliases.push_back("login");
	usernameAliases.push_back("email");

	QStringList passwordAliases;
	passwordAliases.push_back("password");
	passwordAliases.push_back("pass");
	passwordAliases.push_back("passphrase");

	QStringList urlAliases;
	urlAliases.push_back("url");
	urlAliases.push_back("address");

	aliasedFields.push_back(acctNameAliases);
	aliasedFields.push_back(usernameAliases);
	aliasedFields.push_back(passwordAliases);
	aliasedFields.push_back(urlAliases);

	QStringList fieldNames;

	for (auto f : fields) {
		fieldNames.append(f.name);
	}

	QVector<QString> fieldValues;
	fieldValues.resize(aliasedFields.size());

	QVector<QStringList::const_iterator> aliasMatched = aliasMatch(aliasedFields, fieldNames);

	for (genericField f : fields) {
		bool matched = false;
		int i = 0;
		for (QStringList::const_iterator iter : aliasMatched) {
			if (iter == aliasedFields.at(i).cend()) {
				i++;
				continue;
			}
			const QString &aliasName = *iter;
			if (!f.name.compare(aliasName, Qt::CaseInsensitive)) {
				fieldValues[i] = f.value;
				matched = true;
			}
			i++;
		}
		if (!matched) {
			acct->fields.addField(f);
		}
	}
	acct->acctName = fieldValues[0];
	acct->userName = fieldValues[1];
	acct->password = fieldValues[2];
	acct->url = fieldValues[3];
	return acct;
}
