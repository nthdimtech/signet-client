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
