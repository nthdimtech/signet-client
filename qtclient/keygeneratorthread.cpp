#include "keygeneratorthread.h"
#include "signetapplication.h"

KeyGeneratorThread::KeyGeneratorThread()
{

}

void KeyGeneratorThread::setParams(const QString &password, const QByteArray &hashfn, const QByteArray &salt)
{
	m_password = password;
	m_hashfn = hashfn;
	m_salt = salt;
}

void KeyGeneratorThread::run()
{
	SignetApplication::generateKey(m_password, m_key, m_hashfn, m_salt);
}
