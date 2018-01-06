#include "keyboardlayouttester.h"
#include "signetapplication.h"
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>

KeyboardLayoutTester::scancodeInfo KeyboardLayoutTester::s_scancodeSequence[] = {

	{4 /*a*/, 2, 3},
	{5 /*b*/, 6, 4},
	{6 /*c*/, 4, 4},
	{7 /*d*/, 4, 3},
	{8 /*e*/, 4, 2},

	{9 /*f*/, 5, 3},
	{10 /*g*/, 6, 3},
	{11 /*h*/, 7, 3},
	{12 /*i*/, 9, 2},
	{13 /*j*/, 8, 3},

	{14 /*k*/, 9, 3},
	{15 /*l*/, 10, 3},
	{16 /*m*/, 8, 4},
	{17 /*n*/, 7, 4},
	{18 /*o*/, 10, 2},

	{19 /*p*/, 11, 2},
	{20 /*q*/, 2, 2},
	{21 /*r*/, 5, 2},
	{22 /*s*/, 3, 3},
	{23 /*t*/, 6, 2},

	{24 /*u*/, 8, 2},
	{25 /*v*/, 5, 4},
	{26 /*w*/, 3, 2},
	{27 /*x*/, 3, 4},
	{28 /*y*/, 7, 2},

	{29 /*z*/, 2, 4},
	{30 /*1*/, 2, 1},
	{31 /*2*/, 3, 1},
	{32 /*3*/, 4, 1},
	{33 /*4*/, 5, 1},

	{34 /*5*/, 6, 1},
	{35 /*6*/, 7, 1},
	{36 /*7*/, 8, 1},
	{37 /*8*/, 9, 1},
	{38 /*9*/,10, 1},

	{39 /*0*/,11, 1},
	#if 0
	{40 /*enter*/, 14, 3},
	{43 /*tab*/, 1, 2},
	#endif
	{44 /*space*/, 3, 5},
	{45 /* -_ */, 12, 1},
	{46 /* =+ */, 13, 1},
	{47 /* [{ */, 12, 2},
	{48 /* ]} */, 13, 2},

	{49 /* \| */, 14, 2},
	{50 /* unknown */, 13, 3},
	{51 /* ;: */, 11, 3},
	{52 /* '" */, 12, 3},
	{53 /* `~ */, 1, 1},
	{54 /* ,< */, 9, 4},

	{55 /* .> */, 10, 4},
	{56 /* /? */, 11, 4},

	{0, 0, 0} //This zero value marks the end of the sequence
};

void KeyboardLayoutTester::configure()
{
	setFocus();
	startTest();
}

void KeyboardLayoutTester::apply()
{
	m_prevLayout = m_layout;
	m_applyButton->setEnabled(false);
	m_resetButton->setEnabled(false);
	m_applyOnClose = true;
	close();
}

void KeyboardLayoutTester::reset()
{
	m_applyButton->setEnabled(false);
	m_resetButton->setEnabled(false);
	showCurrentLayout();
}

KeyboardLayoutTester::KeyboardLayoutTester(const QVector<struct signetdev_key> &currentLayout, QWidget *parent) :
	QMainWindow(parent),
	m_scancodeNumChecking(-1),
	m_signetdevToken(-1),
	m_canStartTest(false),
	m_testing(false),
	m_applyOnClose(false)
{
	setFocusPolicy(Qt::ClickFocus);
	setWindowTitle("Keyboard layout configuration");
	setAttribute(Qt::WA_InputMethodEnabled, true);
	connect(SignetApplication::get(),
		SIGNAL(signetdevCmdResp(signetdevCmdRespInfo)),
		this,
		SLOT(signetdevCmdResp(signetdevCmdRespInfo)));
	connect(&m_keyTimer,
		SIGNAL(timeout()),
		this,
		SLOT(pressTimeout()));

	QWidget *centralWidget = new QWidget();
	QLayout *buttonsLayout = new QHBoxLayout();

	m_configureButton = new QPushButton("Configure");
	m_resetButton = new QPushButton("Reset");
	m_applyButton = new QPushButton("Apply");
	QPushButton *closeButton = new QPushButton("Close");

	m_resetButton->setEnabled(false);
	m_applyButton->setEnabled(false);

	connect(closeButton, SIGNAL(pressed()), this, SLOT(close()));
	connect(this, SIGNAL(close()), this, SLOT(deleteLater()));
	connect(m_configureButton, SIGNAL(pressed()), this, SLOT(configure()));
	connect(m_applyButton, SIGNAL(pressed()), this, SLOT(apply()));
	connect(m_resetButton, SIGNAL(pressed()), this, SLOT(reset()));

	buttonsLayout->addWidget(m_configureButton);
	buttonsLayout->addWidget(m_resetButton);
	buttonsLayout->addWidget(m_applyButton);
	buttonsLayout->addWidget(closeButton);

	QVBoxLayout *mainLayout = new QVBoxLayout();

	centralWidget->setLayout(mainLayout);

	m_gridLayout = new QGridLayout();

	m_grid = new QWidget();
	m_grid->setLayout(m_gridLayout);

	mainLayout->addWidget(m_grid);
	mainLayout->addLayout(buttonsLayout);

	m_layout = currentLayout;
	m_prevLayout = currentLayout;

	showCurrentLayout();
	setCentralWidget(centralWidget);
}

void KeyboardLayoutTester::initGridLayout()
{
	m_gridLayout->addWidget(new QLabel("Regular keys"), 0, 0, 1, 10);
	m_gridLayout->addWidget(new QLabel("Shift keys"), 6, 0, 1, 10);
	m_gridLayout->addWidget(new QLabel("R-Alt keys"), 0, 16, 1, 10);
	m_gridLayout->addWidget(new QLabel("Shift + R-Alt keys"), 6, 16, 1, 10);
	for (int i = 1; i <= 14 + 15; i++) {
		QLabel *l = new QLabel("");
		l->setMinimumSize(15,15);
		m_gridLayout->addWidget(l, 1, i, 1, 1);
	}
	for (int j = 2; j <= 5; j++) {
		QLabel *l = new QLabel("");
		l->setMinimumSize(15,15);
		m_gridLayout->addWidget(l, j, 1, 1, 1);
	}
	for (int j = 6; j <= 6+5; j++) {
		QLabel *l = new QLabel("");
		l->setMinimumSize(15,15);
		m_gridLayout->addWidget(l, j, 1, 1, 1);
	}
}

void KeyboardLayoutTester::showCurrentLayout()
{
	if (m_gridLayout) {
		QLayoutItem* item;
		while ( ( item = m_gridLayout->takeAt( 0 ) ) != NULL ) {
			delete item->widget();
			delete item;
		}
		delete m_gridLayout;
	}
	m_gridLayout = new QGridLayout();
	initGridLayout();

	for (int i = 0; i < m_prevLayout.size(); i++) {
		const struct signetdev_key &k = m_prevLayout.at(i);
		int j = 0;
		while (s_scancodeSequence[j].code != k.phy_key[0].scancode
				&&
			s_scancodeSequence[j].code) {
			j++;
		}
		if (s_scancodeSequence[j].code == k.phy_key[0].scancode) {
			int row = s_scancodeSequence[j].row;
			int col = s_scancodeSequence[j].column;
			if (row && col) {
				QLabel *l = new QLabel(QChar((ushort)k.key));
				l->setMinimumSize(15,15);
				if (k.phy_key[0].modifier & 0x40) {
					col += 15;
				}
				if (k.phy_key[0].modifier & 2) {
					row += 6;
				}
				auto item = m_gridLayout->itemAtPosition(row, col);
				if (item && item->widget()) {
					item->widget()->deleteLater();
				}
				m_gridLayout->addWidget(l, row, col, 1, 1);
			}
		}
	}
	m_grid->setLayout(m_gridLayout);
}

void KeyboardLayoutTester::closeEvent(QCloseEvent *event) {
	closing(m_applyOnClose);
	event->accept();
}

void KeyboardLayoutTester::typeKey()
{
	signetdev_phy_key key;
	m_keysTyped = false;
	m_keyRecieved = false;
	key.modifier = m_modifierChecking;
	key.scancode = s_scancodeSequence[m_scancodeNumChecking].code;
	u8 codes[4] = {
		key.modifier,
		key.scancode,
		0,
		0
	};
	m_keyTimer.start(100);
	m_keysEmitted.append(key);
	::signetdev_type_raw(NULL, &m_signetdevToken, codes, 2);
}

void KeyboardLayoutTester::startTest()
{
	m_canStartTest = true;
	m_configureButton->setEnabled(false);
	if (QApplication::activeWindow() == this) {
		doStartTest();
	}
}

void KeyboardLayoutTester::doStartTest()
{
	m_scancodeNumChecking = 0;
	m_modifierChecking = 0;
	m_testing = true;
	m_skipGeneratingRAlt = false;

	m_layout.clear();
	signetdev_key k;
	k.key = '\t';
	k.phy_key[0].modifier = 0;
	k.phy_key[0].scancode = 43;
	k.phy_key[1].modifier = 0;
	k.phy_key[1].scancode = 0;
	m_layout.append(k);
	k.key = '\n';
	k.phy_key[0].modifier = 0;
	k.phy_key[0].scancode = 40;
	k.phy_key[1].modifier = 0;
	k.phy_key[1].scancode = 0;
	m_layout.append(k);

	QLayoutItem* item;
	while ( ( item = m_gridLayout->takeAt( 0 ) ) != NULL )
	{
		delete item->widget();
		delete item;
	}
	delete m_gridLayout;
	m_gridLayout = new QGridLayout();
	initGridLayout();
	m_grid->setLayout(m_gridLayout);
	typeKey();
}

void KeyboardLayoutTester::stopTest()
{
	m_canStartTest = false;
	m_testing = false;
	m_keyTimer.stop();
}

void KeyboardLayoutTester::focusInEvent( QFocusEvent* e)
{
	e->accept();
	if (e->gotFocus() && !m_testing && m_canStartTest) {
		doStartTest();
	} else if (e->lostFocus() && m_testing) {
		stopTest();
	}
}

void KeyboardLayoutTester::inputMethodEvent(QInputMethodEvent *event)
{
	event->accept();
	if (event->commitString().size()) {
		if (m_keysEmitted.size()) {
			m_deadKeys.append(m_keysEmitted.at(0));
			charactersTyped(event->commitString());
		}
	}
}

void KeyboardLayoutTester::charactersTyped(QString t)
{
	if (!m_testing)
		return;

	if (t.size() == 0)
		return;

	if (t.size() != 1) {
		stopTest();
		return;
	} else {
		if (t.at(0) == ' ' && m_timeoutCount > 0)
			return;
	}

	signetdev_key k;
	k.key = t.data()[0].unicode();
	k.phy_key[0] = m_keysEmitted.at(0);
	if (m_keysEmitted.size() > 1) {
		k.phy_key[1] = m_keysEmitted[1];
	} else {
		k.phy_key[1].modifier = 0;
		k.phy_key[1].scancode = 0;
	}

	scancodeInfo &codeInfo = s_scancodeSequence[m_scancodeNumChecking];
	int row = codeInfo.row;
	int col = codeInfo.column;
	if (row && col) {
		if (m_modifierChecking & 2) {
			row += 6;
		}
		if (m_modifierChecking & 0x40) {
			col += 15;
		}
		QLabel *label = NULL;
		label = new QLabel(t);
		m_gridLayout->addWidget(label, row, col, 1, 1);
	}
	m_layout.push_back(k);
	m_keyRecieved = true;
	if (m_keysTyped && m_keyRecieved) {
		typeNextKey();
	}
}

void KeyboardLayoutTester::keyPressEvent(QKeyEvent *event)
{
	QString t = event->text();
	event->accept();
	if (!(event->modifiers() & Qt::AltModifier)) {
		charactersTyped(t);
	} else {
		m_skipGeneratingRAlt = true;
	}
}

void KeyboardLayoutTester::typeNextKey()
{
	if (m_testing) {
		m_keysEmitted.clear();
		m_timeoutCount = 0;
		switch (m_modifierChecking) {
		case 0:
			m_modifierChecking = 2;
			break;
		case 2:
			if (m_skipGeneratingRAlt) {
				m_modifierChecking = 0;
			} else {
				m_modifierChecking = 0x40;
			}
			break;
		case 0x40:
			if (m_skipGeneratingRAlt) {
				m_modifierChecking = 0;
			} else {
				m_modifierChecking = 0x42;
			}
			break;
		case 0x42:
			m_modifierChecking = 0;
			break;
		}
		if (!m_modifierChecking) {
			m_scancodeNumChecking++;
			if (!s_scancodeSequence[m_scancodeNumChecking].code) {
				stopTest();
				m_configureButton->setEnabled(true);
				m_applyButton->setEnabled(true);
				m_resetButton->setEnabled(true);
			} else {
				typeKey();
			}
		} else {
			typeKey();
		}
	}
}

void KeyboardLayoutTester::pressTimeout()
{
	signetdev_phy_key key;
	key.modifier = 0;
	key.scancode = 44; //Space
	u8 codes[] = {
		key.modifier,
		key.scancode,
		0,
		0
	};
	m_timeoutCount++;

	if (m_timeoutCount < 2) {
		m_keysTyped = false;
		m_keyRecieved = false;
		m_keyTimer.start(100);
		m_keysEmitted.append(key);
		::signetdev_type_raw(NULL, &m_signetdevToken, codes, 2);
	} else {
		typeNextKey();
	}
}

void KeyboardLayoutTester::signetdevCmdResp(signetdevCmdRespInfo info)
{
	if (info.token != m_signetdevToken)
		return;
	m_signetdevToken = -1;
	switch (info.cmd) {
	case SIGNETDEV_CMD_TYPE:
		m_keysTyped = true;
		if (m_keysTyped && m_keyRecieved) {
			typeNextKey();
		}
		break;
	}
}

