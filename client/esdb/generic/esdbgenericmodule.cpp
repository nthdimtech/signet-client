#include "esdbbookmarkmodule.h"
#include "genericactionbar.h"
#include "esdb.h"
#include "generic.h"
#include "generictypedesc.h"
#include "esdbgenericmodule.h"

class EsdbActionBar;

esdbGenericModule::esdbGenericModule(genericTypeDesc *typeDesc, bool userDefined, bool plural) :
	esdbTypeModule(plural ? typeDesc->name + "s" : typeDesc->name)
{
	Q_UNUSED(userDefined);
}

esdbEntry *esdbGenericModule::decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const
{
	generic *g = NULL;
	generic_1 rev_1(id);
	generic_2 rev_2(id);

	if (!prev) {
		g = new generic(id);
	} else {
		g = static_cast<generic *>(prev);
	}

	switch(revision) {
	case 0:
		rev_1.fromBlock(blk);
		break;
	case 1:
		rev_2.fromBlock(blk);
		break;
	case 2:
		g->fromBlock(blk);
	}

	switch(revision) {
	case 0:
		rev_2.upgrade(rev_1);
	case 1:
		g->upgrade(rev_2);
	case 2:
		break;
	default:
		delete g;
		g = NULL;
		break;
	}
	return g;
}

esdbEntry *esdbGenericModule::decodeEntry(const QVector<genericField> &fields, bool doAliasMatch) const
{
	QVector<QStringList> aliasedFields;
	QStringList nameAliases;
	nameAliases.push_back("name");
	if (doAliasMatch) {
		nameAliases.push_back("title");
		nameAliases.push_back("account");
		nameAliases.push_back("url");
		nameAliases.push_back("address");
	}
	aliasedFields.push_back(nameAliases);
	QStringList fieldNames;
	for (auto f : fields) {
		fieldNames.append(f.name);
	}
	generic *g = new generic(-1);
	QVector<QStringList::const_iterator> aliasMatched = aliasMatch(aliasedFields, fieldNames);
	QVector<QString> fieldValues = aliasMatchValues(aliasedFields, aliasMatched, fields, &g->fields);
	g->name = fieldValues[0];
	g->typeName = name();
	return g;
}
