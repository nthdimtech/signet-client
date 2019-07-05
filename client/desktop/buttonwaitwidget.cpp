#include "buttonwaitwidget.h"
#include "signetapplication.h"

#include <QPixmap>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>

ButtonWaitWidget::ButtonWaitWidget(QString action, bool longPress, QWidget *parent) :
	QWidget(parent),
	m_timeLeft(sTimeoutPeriod),
	m_action(action),
    m_longPress(longPress),
    m_timeoutOccured(false)
{
	QPixmap pm(":/images/button_press.png");
	//TODO: Add changable text to widget
    QLabel *image = new QLabel();
    image->setPixmap(pm.scaledToHeight(64));

    QBoxLayout *topLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    QBoxLayout *topRow = new QBoxLayout(QBoxLayout::LeftToRight);
    QBoxLayout *textColumn = new QBoxLayout(QBoxLayout::TopToBottom);

    m_actionText = new QLabel();
    m_timeoutText = new QLabel();

    int pxSz = m_timeoutText->fontInfo().pixelSize();

    QString style = "font-weight: bold; font-size: " + QString::number((pxSz * 110) / 100) + "px";
    //QString style = "font-weight: bold";

    m_timeoutText->setStyleSheet(style);
    m_actionText->setStyleSheet(style);

	m_cancelButton = new QPushButton("Cancel");
    connect(m_cancelButton, SIGNAL(clicked()), this, SIGNAL(canceled()));
    topLayout->setAlignment(Qt::AlignTop);

    textColumn->addWidget(m_actionText);
    textColumn->addWidget(m_timeoutText);
    image->setAlignment(Qt::AlignLeft);
    image->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    topRow->addWidget(image);
    topRow->addLayout(textColumn);
    topLayout->addLayout(topRow);
    topLayout->addWidget(m_cancelButton);

	updateText();
    setLayout(topLayout);
	SignetApplication *app = SignetApplication::get();
	connect(app, SIGNAL(signetdevTimerEvent(int)),
		this, SLOT(signetdevTimerEvent(int)));
}

void ButtonWaitWidget::updateText()
{
    QString actionMessage = QString("Push ") + (m_longPress ? "and HOLD" : "") + " button on device to " + m_action;
    QString timeoutMessage =  "Timeout in " + QString::number(m_timeLeft) + " seconds";

    m_actionText->setText(actionMessage);
    m_timeoutText->setText(timeoutMessage);
}

void ButtonWaitWidget::resetTimeout()
{
	m_timeLeft = sTimeoutPeriod;
	updateText();
}

void ButtonWaitWidget::signetdevTimerEvent(int val)
{
	m_timeLeft = val;
    if (!m_timeLeft && !m_timeoutOccured) {
        m_timeoutOccured = true;
		timeout();
	}
	updateText();
}
