#include "esdbtypemodule.h"
#include "esdbmodel.h"
#include "esdbactionbar.h"

esdbTypeModule::esdbTypeModule(const QString &name) :
	m_name(name)
{
}


EsdbActionBar *esdbTypeModule::newActionBar()
{
	return NULL;
}
