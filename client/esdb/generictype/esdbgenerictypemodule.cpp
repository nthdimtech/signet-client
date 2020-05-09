#include "esdbgenerictypemodule.h"
#include "generic/generictypedesc.h"

esdbGenericTypeModule::esdbGenericTypeModule() : esdbTypeModule("Data types")
{

}

esdbEntry *esdbGenericTypeModule::decodeEntry(int id, int revision, esdbEntry *prev, block *blk) const
{
	esdbEntry *entry = nullptr;
	genericTypeDesc *desc = nullptr;
	genericTypeDesc_1 rev_1(id);
	if (!prev) {
		desc = new genericTypeDesc(id);
	} else {
		desc = static_cast<genericTypeDesc *>(prev);
	}

	switch(revision) {
	case 0:
	case 1:
		rev_1.fromBlock(blk);
		break;
	case 2:
		desc->fromBlock(blk);
		break;
	}

	switch(revision) {
	case 0:
	case 1:
		desc->upgrade(rev_1);
		break;
	case 2:
		desc->fromBlock(blk);
		break;
	default:
		delete desc;
		desc = nullptr;
		break;
	}
	entry = desc;
	return entry;
}

esdbEntry *esdbGenericTypeModule::decodeEntry(const QVector<genericField> &fields, bool doAliasMatch) const
{
	QVector<QStringList> aliasedFields;
	QStringList nameAliases;
	QStringList pathAliases;
	nameAliases.push_back("name");
	pathAliases.push_back("path");
	aliasedFields.push_back(nameAliases);
	aliasedFields.push_back(pathAliases);
	QStringList fieldNames;
	for (auto f : fields) {
		fieldNames.append(f.name);
	}
	genericFields genFields;
	QVector<QStringList::const_iterator> aliasMatched = aliasMatch(aliasedFields, fieldNames);
	QVector<QString> fieldValues = aliasMatchValues(aliasedFields, aliasMatched, fields, &genFields);
	genericTypeDesc *desc = new genericTypeDesc(-1); //TODO: Get real ID assigned
	for (int i = 0; i < genFields.fieldCount(); i++) {
		auto f = genFields.getField(i);
		desc->fields.append(fieldSpec(f.name, f.value));
	}
	desc->typeId = generic::invalidTypeId;
	desc->name = fieldValues[0];
	desc->group = fieldValues[1];
	return desc;
}
