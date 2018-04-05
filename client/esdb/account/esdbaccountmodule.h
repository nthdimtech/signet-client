#ifndef ESDBACCOUNTTYPE_H
#define ESDBACCOUNTTYPE_H

#include <QObject>
#include <QVector>
#include <QStringList>


#include "esdbtypemodule.h"
#include "account.h"

class NewAccount;
class QPushButton;
class ButtonWaitDialog;

struct esdbAccountModule : public esdbTypeModule {
public:
	esdbEntry *decodeEntry(int id, int revision, esdbEntry *prev, struct block *blk) const;
	esdbEntry *decodeEntry(const QVector<genericField> &fields, bool aliasMatch = true) const;
	esdbAccountModule() :
		esdbTypeModule("Accounts")
	{ }

	virtual ~esdbAccountModule() {}
};

#endif // ESDBACCOUNTTYPE_H
