#include "cleartextpasswordselector.h"
#include "cleartextpasswordeditor.h"
#include "signetapplication.h"
#include "buttonwaitdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QRadioButton>
#include <QPushButton>
#include <QLabel>

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include "signetapplication.h"

cleartextPasswordSelector::cleartextPasswordSelector(QVector<int> formats, QStringList names, QWidget *parent) :
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

	if (formats.size() == names.size()) {
		for (int i = 0; i < formats.size(); i++) {
			QString n;
			if (formats.at(i) != 1) {
				n = "<Unused>";
			} else {
				n = names.at(i);
			}
			QHBoxLayout *slot = new QHBoxLayout();
			QRadioButton *b = new QRadioButton(n);
			connect(b, SIGNAL(pressed()), this, SLOT(selectionPressed()));
			slot->addWidget(new QLabel("Slot #" + QString::number(i+1)));
			slot->addWidget(b);

			m_slotButtons.append(b);
			l->addLayout(slot);
		}
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
		e->setWindowTitle("Password slot " + QString::number(m_index + 1));
		e->setMinimumWidth(300);
		e->exec();
		e->deleteLater();
	} else {
		QMessageBox *box = SignetApplication::messageBoxError(QMessageBox::Critical,
								QString("Read password slot"),
								QString("Failed to read password slot ") +
									QString::number(m_index),
								this);
		box->exec();
	}
}

void cleartextPasswordSelector::openPressed()
{
	int i = 0;
	for (auto b : m_slotButtons) {
		if (b->isChecked()) {
			m_index = i;
			m_buttonWaitDialog = new ButtonWaitDialog("Open password slot", "open password slot", this, false);
			connect(m_buttonWaitDialog, SIGNAL(finished(int)), this, SLOT(buttonWaitFinished(int)));
			m_buttonWaitDialog->show();
			::signetdev_read_cleartext_password(NULL, &m_signetdevCmdToken, m_index);
			break;
		}
		i++;
	}
}
