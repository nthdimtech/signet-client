#include "cleartextpasswordselector.h"
#include "cleartextpasswordeditor.h"
#include "signetapplication.h"
#include "buttonwaitdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QRadioButton>
#include <QPushButton>

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include "signetapplication.h"

cleartextPasswordSelector::cleartextPasswordSelector(QStringList names, QWidget *parent) :
	QDialog(parent),
	m_buttonWaitDialog(NULL)
{
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevReadCleartextPassword(signetdevCmdRespInfo,cleartext_pass)),
		this, SLOT(signetdevReadCleartextPassword(signetdevCmdRespInfo,cleartext_pass)));
	setWindowModality(Qt::WindowModal);
	QVBoxLayout *l = new QVBoxLayout();
	QHBoxLayout *buttons = new QHBoxLayout();
	m_openButton = new QPushButton("Open");
	QPushButton *cancel = new QPushButton("Cancel");

	for (auto x : names) {
		QString n;
		if (x.isEmpty()) {
			n = "<Unused>";
		} else {
			n = x;
		}
		QRadioButton *b = new QRadioButton(n);
		connect(b, SIGNAL(pressed()), this, SLOT(selectionPressed()));
		m_slotButtons.append(b);
		l->addWidget(b);
	}
	buttons->addWidget(m_openButton);
	buttons->addWidget(cancel);
	l->addLayout(buttons);
	setLayout(l);

	m_openButton->setDisabled(true);

	connect(cancel, SIGNAL(pressed()), this, SLOT(close()));
	connect(m_openButton, SIGNAL(pressed()), this, SLOT(openPressed()));
}

void cleartextPasswordSelector::selectionPressed()
{
	m_openButton->setDisabled(false);
}

void cleartextPasswordSelector::buttonWaitFinished(int result)
{
	m_buttonWaitDialog->deleteLater();
	if (result != QMessageBox::Ok) {
		::signetdev_cancel_button_wait();
	}
	m_buttonWaitDialog = NULL;
}

void cleartextPasswordSelector::signetdevReadCleartextPassword(signetdevCmdRespInfo info, cleartext_pass pass)
{
	if (info.token != m_signetdevCmdToken) {
		return;
	}
	m_signetdevCmdToken = -1;
	if (m_buttonWaitDialog)
		m_buttonWaitDialog->done(QMessageBox::Ok);
	if (info.resp_code == OKAY) {
		done(0);
		cleartextPasswordEditor *e = new cleartextPasswordEditor(m_index, &pass, parentWidget());
		e->exec();
		e->deleteLater();
	} else {
		//TODO
	}
}

void cleartextPasswordSelector::openPressed()
{
	int i = 0;
	for (auto b : m_slotButtons) {
		if (b->isChecked()) {
			m_index = i;
			m_buttonWaitDialog = new ButtonWaitDialog("Open password slot", "open password slot", this, true);
			connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
			m_buttonWaitDialog->show();
			::signetdev_read_cleartext_password(NULL, &m_signetdevCmdToken, m_index);
			break;
		}
		i++;
	}
}
