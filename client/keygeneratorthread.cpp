#include "keygeneratorthread.h"
#include "signetapplication.h"

KeyGeneratorThread::KeyGeneratorThread()
{

}

void KeyGeneratorThread::setParams(const QString &password, const QByteArray &hashfn, const QByteArray &salt, int keyLength)
{
	m_password = password;
	m_hashfn = hashfn;
	m_salt = salt;
	m_keyLength = keyLength;
}

void KeyGeneratorThread::run()
{
	SignetApplication::generateKey(m_password, m_key, m_hashfn, m_salt, m_keyLength);
}
