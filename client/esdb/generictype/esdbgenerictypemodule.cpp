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

esdbEntry *esdbGenericTypeModule::decodeEntry(const QVector<genericField> &fields, bool aliasMatch) const
{
	Q_UNUSED(fields);
	Q_UNUSED(aliasMatch);
	genericTypeDesc *desc = new genericTypeDesc(-1);
	//TODO
	return desc;
}
