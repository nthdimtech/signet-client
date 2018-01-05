#ifndef KEYBOARDLAYOUTTESTER_H
#define KEYBOARDLAYOUTTESTER_H

#include "signetapplication.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include <QObject>
#include <QDialog>
#include <QTimer>
#include <QMainWindow>
#include <QGridLayout>
class QLabel;
#include <QInputMethod>

#include <QInputMethodEvent>

class KeyCapture;

class KeyboardLayoutTester : public QMainWindow
{
	Q_OBJECT
	QTimer m_keyTimer;
	QInputMethodEvent e;

	struct scancodeInfo {
		int code;
		int column;
		int row;
	};

	static scancodeInfo s_scancodeSequence[];
	int m_scancodeNumChecking;
	int m_modifierChecking;
	QVector<struct signetdev_key> m_layout;
	QVector<struct signetdev_key> m_prevLayout;
	void typeKey();
	int m_signetdevToken;
	bool m_canStartTest;
	bool m_testing;

	QPushButton *m_configureButton;
	QPushButton *m_resetButton;
	QPushButton *m_applyButton;

	QGridLayout *m_gridLayout;
	QWidget *m_grid;
	void doStartTest();
	void showCurrentLayout();
	void typeNextKey();
	void inputMethodEvent(QInputMethodEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void initGridLayout();
	int m_timeoutCount;
public:
	explicit KeyboardLayoutTester(const QVector<struct signetdev_key> &currentLayout, QWidget *parent = 0);
	void focusInEvent(QFocusEvent* e);
	const QVector<struct signetdev_key> &getLayout() {
		return m_layout;
	}
	void startTest();
	void stopTest();
signals:
	void testFinished();
	void testKeyTimeout();
	void testLostFocus();
	void testMultiCharResult();
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void pressTimeout();
	void configure();
	void apply();
	void reset();
};

#endif // KEYBOARDLAYOUTTESTER_H
