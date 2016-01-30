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
		acct->fromBlock(blk);
		break;
	}

	switch(revision) {
	case 0:
		rev1.upgrade(rev0);
	case 1:
		rev2.upgrade(rev1);
	case 2:
		acct->upgrade(rev2);
		break;
	case 3:
		break;
	default:
		delete acct;
		acct = NULL;
		break;
	}
	entry = acct;
	return entry;
}
