#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QSettings>
#include <QDebug>
#include <QComboBox>
#include <QMainWindow>
#include <QButtonGroup>
#include <QtGlobal>

#include "globals.h"

QT_USE_NAMESPACE
QT_BEGIN_NAMESPACE

namespace Ui {
class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE


class SettingsDialog : public QDialog {
	Q_OBJECT

public:
	enum BaudRateBig {
		Baud1200 = 1200,
		Baud2400 = 2400,
		Baud4800 = 4800,
		Baud9600 = 9600,
		Baud19200 = 19200,
		Baud38400 = 38400,
		Baud57600 = 57600,
		Baud115200 = 115200,
		Baud230400 = 230400,
		Baud345600 = 345600,
		Baud460800 = 460800,
		Baud691200 = 691200,
		Baud921600 = 921600,
		Baud1Mega = 1000000,
		UnknownBaud = -1
	};
	Q_ENUM(BaudRateBig)

	struct Settings {
		Settings() {}
		Settings(const Settings &others) :
			name(others.name),
			baudRate(others.baudRate),
			stringBaudRate(others.stringBaudRate),
			dataBits(others.dataBits),
			stringDataBits(others.stringDataBits),
			parity(others.parity),
			stringParity(others.stringParity),
			stopBits(others.stopBits),
			stringStopBits(others.stringStopBits),
			flowControl(others.flowControl),
			stringFlowControl(others.stringFlowControl),
			localEchoEnabled(others.localEchoEnabled) {}
		~Settings() {}
		QString name;
		qint32 baudRate;
		QString stringBaudRate;
		QSerialPort::DataBits dataBits;
		QString stringDataBits;
		QSerialPort::Parity parity;
		QString stringParity;
		QSerialPort::StopBits stopBits;
		QString stringStopBits;
		QSerialPort::FlowControl flowControl;
		QString stringFlowControl;
		bool localEchoEnabled;
	};


	explicit SettingsDialog(QWidget *parent = nullptr);
	~SettingsDialog();

	Settings settings() const;

	QComboBox *getSerialPortInfoListBox();
	QComboBox *getBaudrateBox();
	void apply();

private slots:
	void showPortInfo(int idx);
	void checkCustomBaudRatePolicy(int idx);
	void checkCustomDevicePathPolicy(int idx);
	void updateOptions(const int value);

private:
	void fillPortsParameters();
	void fillPortsInfo();
	void updateSettings();

private:
	Ui::SettingsDialog	*ui;
	Settings					currentSettings;
	QIntValidator			*intValidator;
	QButtonGroup			*btnGrp;
	QMainWindow				*mwin;
};

Q_DECLARE_METATYPE(SettingsDialog::Settings)


#endif // SETTINGSDIALOG_H
