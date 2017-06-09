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
	Q_ENUMS(BaudRateFast)

public:
	enum BaudRateFast {
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
	Q_ENUM(BaudRateFast)

	struct Settings {
		QString  ttyName, sDataBits, sBaudrate,
		sParity, sStopBits, sFlowCtrl;

		QSerialPort::DataBits dataBits;
		QSerialPort::Parity parity;
		QSerialPort::StopBits stopBits;
		QSerialPort::FlowControl flowCtrl;

		qint32 baudrate;
		bool localEchoOn;

		qreal sldOpacity;
		QAbstractButton *btnGrpChecked;
	};

	explicit SettingsDialog(QWidget *parent = nullptr);
	~SettingsDialog();

	Settings settings() const;

	QComboBox *getSerialPortInfoListBox();
	QComboBox *getBaudrateBox();
	void apply();

public slots:
	void reject() override;
	void accept() override;
	void promoteSettingsStruct(Settings &settings);

private slots:
	void showPortInfo(int idx);
	void checkCustomBaudRatePolicy(int idx);
	void checkCustomDevicePathPolicy(int idx);

protected:
	void showEvent(QShowEvent *e) override;

private:
	void initialPortParams();
	void fillPortsInfo();
	void updateSettingsStruct(Settings &settings);

private:
	Ui::SettingsDialog	*ui;
	Settings					m_activeCfg, m_backupCfg;
	QIntValidator			*intValidator;
	QButtonGroup			*btnGrp;
	QMainWindow				*mwin;
	QMetaEnum				metaEnumAppear, metaEnumBauds;
};

Q_DECLARE_METATYPE(SettingsDialog::Settings)
Q_DECLARE_METATYPE(SettingsDialog::BaudRateFast)


#endif // SETTINGSDIALOG_H
