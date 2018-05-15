#ifndef KEYBOARDLAYOUTTESTER_H
#define KEYBOARDLAYOUTTESTER_H

#include "signetapplication.h"

extern "C" {
#include "signetdev/host/signetdev.h"
}

#include <QObject>
#include <QDialog>
#include <QTimer>
#include <QGridLayout>
class QLabel;
#include <QInputMethod>

#include <QInputMethodEvent>

class KeyCapture;

class KeyboardLayoutTester : public QDialog
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
	void charactersTyped(QString t);
	int m_timeoutCount;
	QVector<signetdev_phy_key> m_keysEmitted;
	QVector<signetdev_phy_key> m_deadKeys;
	bool m_skipGeneratingRAlt;
	bool m_keysTyped;
	bool m_keyRecieved;
	bool m_applyOnClose;
	void focusInEvent(QFocusEvent* e);
	void closeEvent(QCloseEvent *event);
	void testInterrupted();
	QLabel *m_configurationWarning;
public:
	explicit KeyboardLayoutTester(const QVector<struct signetdev_key> &currentLayout, QWidget *parent = 0);
	const QVector<struct signetdev_key> &getLayout() {
		return m_prevLayout;
	}
	void startTest();
	void stopTest();
signals:
	void closing(bool applyChanges);
	void applyChanges();
public slots:
	void signetdevCmdResp(signetdevCmdRespInfo info);
	void pressTimeout();
	void configure();
	void prepareToTest();
	void apply();
	void reset();
	void applicationStateChanged(Qt::ApplicationState state);
	void focusWindowChanged(QWindow *);
private slots:
	void canelPressed();
};

#endif // KEYBOARDLAYOUTTESTER_H
