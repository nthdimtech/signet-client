#include "passimporter.h"

#ifdef Q_OS_UNIX

#include "signetapplication.h"
#include "account.h"
#include "passimportunlockdialog.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QProcess>
#include <QWidget>
#include <QStringList>

PassImporter::PassImporter(QWidget *parent) :
	DatabaseImporter(parent),
	m_parent(parent)
{

}

void PassImporter::start()
{
	m_gpgId = getGPGId();

	if (!m_gpgId.size()) {
		QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical,
						   databaseTypeName() + " Import",
						   "Couldn't find valid password store",
						   m_parent);
		box->exec();
		done(false);
		return;
	}

	PassImportUnlockDialog *unlockDialog = new PassImportUnlockDialog(this, m_parent);
	unlockDialog->setWindowTitle(databaseTypeName()+ " Import");
	unlockDialog->setWindowModality(Qt::WindowModal);
	if (unlockDialog->exec()) {
		unlockDialog->deleteLater();
		done(false);
		return;
	}
	m_passphrase = unlockDialog->passphrase();
	m_db = new database();
	QDir root(passwordStorePath());
	m_accountType = new databaseType();
	m_db->insert("Accounts", m_accountType);
	traverse(QString(), root);
	done(true);
}

void PassImporter::traverse(QString path, QDir &dir)
{
	QFileInfoList fileInfoList;
	QStringList nameFilters;
	nameFilters.append(QString("*.gpg"));
	fileInfoList = dir.entryInfoList(nameFilters, QDir::Files);
	for (auto info : fileInfoList) {
		QString accountName = info.completeBaseName();
		QString cmd;
		cmd = "gpg -d --batch -r " + m_gpgId;
		if (m_passphrase.size()) {
			cmd += " --passphrase " + m_passphrase;
		}
		cmd += " \"" + info.absoluteFilePath() + "\"";
		QProcess p;
		p.start(cmd);
		p.waitForFinished(-1);
		int rc = p.exitCode();
		if (rc != 0) {
			QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical,
							   databaseTypeName() + " Import",
							   "Failed to decrypt " + accountName,
							   m_parent);
			box->exec();
		} else {
			QString accountPassword = QString::fromUtf8(p.readAllStandardOutput());

			if (accountPassword.size()) {
				accountPassword[accountPassword.size()-1] == '\n';
				accountPassword.truncate(accountPassword.size()-1);
			}
			account *a = new account(-1);
			a->acctName = accountName;
			a->userName = accountName;
			a->password = accountPassword;
			a->setPath(path);
			m_accountType->append(a);
		}
	}
	fileInfoList = dir.entryInfoList(QStringList(), QDir::Dirs);
	for (auto info : fileInfoList) {
		if (info.fileName() == "." || info.fileName() == "..")
			continue;
		QString subPath = path + "/" + info.fileName();
		QDir subDir(info.absoluteFilePath());
		traverse(subPath, subDir);
	}
}

QString PassImporter::passwordStorePath()
{
	QString home = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
	return home + QString("/.password-store");
}

QString PassImporter::gpgIdPath()
{
	return passwordStorePath() + QString("/.gpg-id");
}

bool PassImporter::passphraseCheck(QString passphrase)
{
	QString cmd;
	QFile f(gpgIdPath() + ".gpg");
	f.remove();
	cmd = "gpg --batch -r " + m_gpgId + " -e " + gpgIdPath();
	if (QProcess::execute(cmd)) {
		return false; //TODO
	}
	cmd = "gpg --batch -n -r " + m_gpgId;
	if (passphrase.size()) {
		cmd += " --passphrase " + passphrase;
	}
	cmd += " -d " + gpgIdPath() + ".gpg";
	bool pass = QProcess::execute(cmd) == 0;
	f.remove();
	return pass;
}

QString PassImporter::getGPGId()
{
	QString gpgIdFilePath = passwordStorePath() + QString("/.gpg-id");
	if (!QFile::exists(gpgIdFilePath))
		return QString();
	QFile gpgIdFile(gpgIdFilePath);
	if (!gpgIdFile.open(QFile::ReadOnly)) {
		return QString();
	}
	QByteArray gpgIdArray = gpgIdFile.readLine();
	QString gpgId = QString::fromLatin1(gpgIdArray);
	if (gpgId.length() != (8+1)) {
		return QString();
	}
	gpgId.truncate(8);
	bool ok;
	gpgId.toUInt(&ok, 16);
	if (!ok) {
		return QString();
	}
	QString cmd = "gpg --batch -n -r " + gpgId + " -e " + gpgIdPath();

	int rc = QProcess::execute(cmd);
	if (rc != 0) {
		return QString();
	}
	return gpgId;
}

#endif
