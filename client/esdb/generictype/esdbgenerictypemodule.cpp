#include "esdbgenerictypemodule.h"
#include "generic/generictypedesc.h"

esdbGenericTypeModule::esdbGenericTypeModule() : esdbTypeModule("Data types")
{

}

esdbEntry *esdbGenericTypeModule::decodeEntry(int id, int revision, esdbEntry *prev, block *blk) const
{
	esdbEntry *entry = NULL;
	genericTypeDesc *desc = NULL;
	if (!prev) {
		desc = new genericTypeDesc(id);
	} else {
		desc = static_cast<genericTypeDesc *>(prev);
	}

	switch(revision) {
	case 0:
	case 1:
		desc->fromBlock(blk);
		break;
	default:
		delete desc;
		desc = NULL;
		break;
	}
	entry = desc;
	return entry;
}

esdbEntry *esdbGenericTypeModule::decodeEntry(const QVector<genericField> &fields, bool aliasMatch) const
{
	genericTypeDesc *desc = new genericTypeDesc(-1);
	//TODO
	return desc;
}
