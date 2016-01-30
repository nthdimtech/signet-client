#include "passwordedit.h"

#include "databasefield.h"
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <random>

PasswordEdit::PasswordEdit(QWidget *parent) : QWidget(parent)
{

	m_generatePassword = new QPushButton("Generate");
	connect(m_generatePassword, SIGNAL(pressed()), this, SLOT(generate_password()));
	m_passwordField = new DatabaseField("password", 140, m_generatePassword);

	m_numGenChars = new QSpinBox();
	m_numGenChars->setValue(16);

	m_genSymbols = new QCheckBox("Include symbols");
	m_genSymbols->setChecked(true);
	m_genSymbolSet = new QLineEdit("!@#$%^&*()");
	m_genSymbolSet->setMinimumWidth(150);

	QHBoxLayout *num_chars_layout = new QHBoxLayout();
	num_chars_layout->addWidget(new QLabel("Number of characters"));
	num_chars_layout->addWidget(m_numGenChars);

	QHBoxLayout *gen_symbols_layout = new QHBoxLayout();
	gen_symbols_layout->addWidget(m_genSymbols);
	gen_symbols_layout->addWidget(m_genSymbolSet);

	QVBoxLayout *generate_options_layout = new QVBoxLayout();
	generate_options_layout->addLayout(num_chars_layout);
	generate_options_layout->addLayout(gen_symbols_layout);
	generate_options_layout->setContentsMargins(25,0,0,12);

	m_generateOptions = new QWidget();
	m_generateOptions->setLayout(generate_options_layout);
	m_generateOptions->hide();

	setContentsMargins(0,0,0,0);
	setLayout(new QVBoxLayout());
	layout()->setContentsMargins(0,0,0,0);
	layout()->addWidget(m_passwordField);
	layout()->addWidget(m_generateOptions);

	connect(m_numGenChars, SIGNAL(valueChanged(int)), this, SLOT(generate_password()));
	connect(m_genSymbols, SIGNAL(stateChanged(int)), this, SLOT(generate_password()));
	connect(m_genSymbolSet, SIGNAL(textChanged(QString)), this, SLOT(generate_password()));
	connect(m_passwordField, SIGNAL(textEdited(QString)), this, SLOT(passwordTextEdited(QString)));
}

QString PasswordEdit::password() const
{
	return m_passwordField->text();
}

void PasswordEdit::setPassword(const QString &pass)
{
	m_passwordField->setText(pass);
	m_generateOptions->hide();
}

void PasswordEdit::generate_password()
{
	m_generateOptions->show();
	int num_chars = m_numGenChars->value();
	QString result;
	QString charset;
	QString lowers("abcdefghijklmnopqrstuvwxyz");
	QString uppers("BCDEFGHIJKLMNOPQRSTUVWXYZ");
	QString numbers("0123456789");
	QString symbols = m_genSymbolSet->text();

	struct pwgen_charset {
		QString chars;
		bool use;
		int min_count;
		int current_count;
	};

	pwgen_charset sets[4];

	sets[0].chars = "abcdefghijklmnopqrstuvwxyz";
	sets[0].use = true;
	sets[0].min_count = 1;

	sets[1].chars = "BCDEFGHIJKLMNOPQRSTUVWXYZ";
	sets[1].use = true;
	sets[1].min_count = 1;

	sets[2].chars = "0123456789";
	sets[2].use = true;
	sets[2].min_count = 1;

	sets[3].chars = m_genSymbolSet->text();
	sets[3].use = m_genSymbols->isChecked();
	sets[3].min_count = 1;

	std::random_device dev;

	for (int i = 0; i < 4; i++) {
		if (sets[i].chars.length() == 0) {
			sets[i].use = false;
		}
		if (sets[i].use) {
			charset.append(sets[i].chars);
		}
	}

	while (1) {
		int total = 0;
		for (int i = 0; i < 4; i++) {
			if (!sets[i].use)
				continue;
			total += sets[i].min_count;
		}
		if (total <= num_chars) {
			break;
		} else {
			for (int i = 0; i < 4; i++) {
				if (!sets[i].use)
					continue;
				int j;
				for (j = 0; j < 4; j++) {
					if (!sets[j].use)
						continue;
					if (sets[i].min_count < sets[j].min_count)
						break;
				}
				if (j == 4) {
					sets[i].min_count--;
				}
			}
		}
	}

	do {
		result.resize(0);
		for (int i = 0; i < 4; i++) {
			if (!sets[i].use)
				continue;
			sets[i].current_count = 0;
		}
		while (result.size() < num_chars) {
			unsigned int k = dev();
			k = k % charset.size();
			QChar c = charset[k];
			for (int i = 0; i < 4; i++) {
				if (!sets[i].use)
					continue;
				if (sets[i].chars.contains(c)) {
					sets[i].current_count++;
				}
			}
			result.append(c);
		}
		int i;
		for (i = 0; i < 4; i++) {
			if (!sets[i].use)
				continue;
			if (sets[i].current_count < sets[i].min_count) {
				break;
			}
		}
		if (i == 4) {
			break;
		}
	} while(1);
	m_passwordField->setText(result);
	emit textEdited(result);
}


void PasswordEdit::passwordTextEdited(QString str)
{
	Q_UNUSED(str);
	emit textEdited(str);
	m_generateOptions->hide();
}
