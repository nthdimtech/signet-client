#include "esdbbookmarkmodule.h"
#include "genericactionbar.h"
#include "esdb.h"
#include "generic.h"
#include "generictypedesc.h"
#include "esdbgenericmodule.h"

class EsdbActionBar;

esdbGenericModule::esdbGenericModule(genericTypeDesc *typeDesc) :
	esdbTypeModule(),
	m_typeDesc(typeDesc)
{
}

QString esdbGenericModule::name() const
{
	return m_typeDesc->name;
}

u16 esdbGenericModule::typeId() const
{
	return m_typeDesc->typeId;
}

esdbEntry *esdbGenericModule::decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const
{
	generic *g = nullptr;

	if (!prev) {
		g = new generic(id);
	} else {
		g = static_cast<generic *>(prev);
	}

	switch(revision) {
	case 0: {
		generic_1 rev_1(id);
		generic_2 rev_2(id);
		generic_3 rev_3(id);
		generic_4 rev_4(id);
		rev_1.fromBlock(blk);
		rev_2.upgrade(rev_1);
		rev_3.upgrade(rev_2);
		rev_4.upgrade(rev_3);
		g->upgrade(rev_4);
		break;
	}
	case 1: {
		generic_2 rev_2(id);
		generic_3 rev_3(id);
		generic_4 rev_4(id);
		rev_2.fromBlock(blk);
		rev_3.upgrade(rev_2);
		rev_4.upgrade(rev_3);
		g->upgrade(rev_4);
	}
	break;
	case 2: {
		generic_3 rev_3(id);
		generic_4 rev_4(id);
		rev_3.fromBlock(blk);
		rev_4.upgrade(rev_3);
		g->upgrade(rev_4);
	}
	break;
	case 3: {
		generic_4 rev_4(id);
		rev_4.fromBlock(blk);
		g->upgrade(rev_4);
	}
	break;
	case 4:
		g->fromBlock(blk);
		break;
	default:
		delete g;
		g = nullptr;
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

	QStringList pathAliases;
	pathAliases.push_back("path");
	if (doAliasMatch) {
		pathAliases.push_back("group");
	}
	aliasedFields.push_back(pathAliases);

	QStringList fieldNames;
	for (auto f : fields) {
		fieldNames.append(f.name);
	}
	generic *g = new generic(-1);
	QVector<QStringList::const_iterator> aliasMatched = aliasMatch(aliasedFields, fieldNames);
	QVector<QString> fieldValues = aliasMatchValues(aliasedFields, aliasMatched, fields, &g->fields);
	g->name = fieldValues[0];
	g->path = fieldValues[1];
	g->typeId = m_typeDesc->typeId;
	return g;
}
