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
	genericTypeDesc *m_typeDesc;
private:
	//genericTypeDesc *m_typeDesc;
	esdbEntry *decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const override;
	//bool m_userDefined;
	esdbEntry *decodeEntry(const QVector<genericField> &fields, bool doAliasMatch) const override;
public:
	esdbGenericModule(genericTypeDesc *typeDesc);

	QString name() const override;
};

#endif // ESDBGENERICMODULE_H
