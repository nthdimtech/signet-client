#ifndef ESDBGENERICMODULE_H
#define ESDBGENERICMODULE_H

#include <QObject>
#include <QList>

#include "esdbtypemodule.h"

class LoggedInWidget;

struct generic;
struct genericTypeDesc;

class esdbGenericModule : public esdbTypeModule
{
	LoggedInWidget *m_parent;
	genericTypeDesc *m_typeDesc;
	EsdbActionBar *newActionBar();
	esdbEntry *decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const;
public:
	esdbGenericModule(genericTypeDesc *typeDesc, LoggedInWidget *parent, bool userDefined, bool plural);
};

#endif // ESDBGENERICMODULE_H
