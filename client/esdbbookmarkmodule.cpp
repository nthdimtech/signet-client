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

esdbEntry *esdbBookmarkModule::decodeEntry(const QVector<genericField> &fields, bool doAliasMatch) const
{
	bookmark *b = new bookmark(-1);
	QVector<QStringList> aliasedFields;

	QStringList nameAliases;
	nameAliases.push_back("name");
	if (doAliasMatch) {
		nameAliases.push_back("title");
		nameAliases.push_back("account");
		nameAliases.push_back("url");
		nameAliases.push_back("address");
	}

	QStringList urlAliases;
	urlAliases.push_back("url");
	if (doAliasMatch) {
		urlAliases.push_back("address");
	}

	aliasedFields.push_back(nameAliases);
	aliasedFields.push_back(urlAliases);

	QStringList fieldNames;
	for (auto f : fields) {
		fieldNames.append(f.name);
	}

	QVector<QStringList::const_iterator> aliasMatched = aliasMatch(aliasedFields, fieldNames);
	QVector<QString> fieldValues = aliasMatchValues(aliasedFields, aliasMatched, fields, NULL);
	b->name = fieldValues[0];
	b->url = fieldValues[1];
	return b;
}
