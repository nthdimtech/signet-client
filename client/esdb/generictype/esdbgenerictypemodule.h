#ifndef ESDBGENERICTYPEMODULE_H
#define ESDBGENERICTYPEMODULE_H

#include "esdbtypemodule.h"

#include <QObject>

class esdbGenericTypeModule : public esdbTypeModule
{
public:
	explicit esdbGenericTypeModule();
	esdbEntry *decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const;
	esdbEntry *decodeEntry(const QVector<genericField> &fields, bool aliasMatch = true) const;
};
#endif // ESDBGENERICTYPEMODULE_H
