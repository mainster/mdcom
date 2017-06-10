#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSlider>
#include <QtCore/QtGlobal>
#include <QDebug>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>
#include <QEvent>
#include <QLayout>
#include <QComboBox>
#include <QRadioButton>

#include "globals.h"

QT_BEGIN_NAMESPACE
class QLabel;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Console;
class SettingsDialog;

class MainWindow : public QMainWindow {

	Q_OBJECT
	Q_ENUMS(Appearance)
	Q_PROPERTY(Appearance appearance READ appearance WRITE setAppearance NOTIFY appearanceChanged)

public:

	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	enum Appearance {
		Translucent,
		Transparent,
		Default,
	};
	Q_ENUM(Appearance)

	void setAppearance(Appearance appearance) {
		m_appearance = appearance;
		emit appearanceChanged(appearance);
	}
	Appearance appearance() const
	{ return m_appearance; }


signals:
	void appearanceChanged(Appearance);

private slots:
	void openSerialPort();
	void closeSerialPort();
	void about();
	void writeData(const QByteArray &data);
	void readData();
	void handleError(QSerialPort::SerialPortError error);

	void setWindowOpacity(qreal level);
	void toLog(Appearance appearance);

protected:
	void resizeEvent(QResizeEvent *) override;
	void closeEvent(QCloseEvent *e) override;

protected slots:
private:
	void initActionsConnections();
	void showStatusMessage(const QString &message);
	void refreshStateMsg();

private:
	Ui::MainWindow		*ui;
	QLabel				*status;
	Console				*console;
	SettingsDialog		*settings;
	QSerialPort			*serial;
	Appearance			m_appearance;
	QSlider				*sldrOpacity;
	QComboBox			*cbxAppearanceOpts;
	static QString		stateStr, stateStrWraped;
	int					buildNo;
	int					wrapStateMsgAt;
};

Q_DECLARE_METATYPE(MainWindow::Appearance);

#endif
