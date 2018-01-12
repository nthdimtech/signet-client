#include "esdbbookmarkmodule.h"
#include "bookmarkactionbar.h"
#include "esdb.h"
#include "bookmark.h"

class EsdbActionBar;

EsdbActionBar *esdbBookmarkModule::newActionBar()
{
	return new BookmarkActionBar(this, m_parent);
}

esdbEntry *esdbBookmarkModule::decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const
{
	bookmark *bm = NULL;

	if (!prev) {
		bm = new bookmark(id);
	} else {
		bm = static_cast<bookmark *>(prev);
	}

	switch(revision) {
	case 0:
		bm->fromBlock(blk);
		break;
	}

	switch(revision) {
	case 0:
		break;
	default:
		delete bm;
		bm = NULL;
		break;
	}
	return bm;
}
