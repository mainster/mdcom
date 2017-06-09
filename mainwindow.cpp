#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"

#include <QMessageBox>
#include <QLabel>
#include <QTime>
#include <QtSerialPort/QSerialPort>

QString MainWindow::stateStr			= QString("Connected to %1: %2, %3, %4, %5, %6");
QString MainWindow::stateStrWraped	= QString("Connected:\n%1\n%2\n%3, %4\n%5, %6");



MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow) {
	QSETTINGS_INIT;
	QSETTINGS;
	setObjectName(tr("MainWindow"));
	ui->setupUi(this);

	config.setValue(tr("buildTime"), QDateTime::currentDateTime().toString(Qt::ISODate));
	buildNo = config.value(tr("build"), 6000).toInt() + 1;
	config.setValue(tr("build"), buildNo);

	/**
	 * @brief	Register Meta Type <MainWindow *>
	 */
	qRegisterMetaType<MainWindow*>("MainWindow");

	console = new Console;
	serial = new QSerialPort(this);
	settings = new SettingsDialog();

	console->setEnabled(false);
	setCentralWidget(console);

	ui->actConnect->setEnabled(true);
	ui->actDisconnect->setEnabled(false);
	ui->actQuit->setEnabled(true);
	ui->actConfig->setEnabled(true);

	status = new QLabel();
	ui->statusBar->addWidget(status);

	ui->mainToolBar->setFloatable(false);
	ui->mainToolBar->setAllowedAreas(Qt::LeftToolBarArea | Qt::TopToolBarArea);

	initActionsConnections();
	connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
			  this, &MainWindow::handleError);
	connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
	connect(console, &Console::getData, this, &MainWindow::writeData);
	setWindowTitle(tr("%1  build: %2").arg(config.applicationName()).arg(buildNo));
	setWindowIcon(QIcon(":/images/usb-ttl232_1.png"));

	/**
	 * @brief	Restore settings from config file
	 */
	wrapStateMsgAt = config.value(tr("wrapStateMsgAt"), 50).toInt();
	restoreGeometry(config.value(tr("geometry")).toByteArray());
	restoreState(config.value(tr("state")).toByteArray());
	QWidget::setWindowOpacity(config.value(tr("windowOpacity"), 1).toReal());

	setAppearance(static_cast<MainWindow::Appearance>(
						  config.value("Appearance", Appearance::Default).toUInt()));

	connect(this, &MainWindow::appearanceChanged, this, &MainWindow::toLog);
	setMinimumSize(10,10);

	const QList<QSerialPortInfo> plist = QSerialPortInfo::availablePorts();
	QList<QString> sers = config.value(tr("portSerials")).toStringList();

	for (int k=0; k<plist.length(); k++ ) {
		if (sers.contains(plist.at(k).serialNumber())) {
			settings->getSerialPortInfoListBox()->setCurrentIndex(k);
			settings->getBaudrateBox()->setCurrentIndex(4);
			settings->getBaudrateBox()->setCurrentText(config.value(tr("baudrate")).toString());
			settings->apply();
			openSerialPort();
			break;
		}
	}
}
void MainWindow::toLog(Appearance appearance) {
	INFO << appearance;
}

MainWindow::~MainWindow() {
	QSETTINGS;
	config.setValue(tr("geometry"), saveGeometry());
	config.setValue(tr("state"), saveState());

	delete settings;
	delete ui;
}
/* ======================================================================== */
/*                              Window visuals                              */
/* ======================================================================== */
void MainWindow::setWindowOpacity(qreal level) {
	QSETTINGS;
	QWidget::setWindowOpacity(level);
	config.setValue(tr("windowOpacity"), level);
}
void MainWindow::resizeEvent(QResizeEvent *) {
	(wrapStateMsgAt < geometry().width())
			? ui->mainToolBar->setOrientation(Qt::Horizontal)
			: ui->mainToolBar->setOrientation(Qt::Vertical);
	refreshStateMsg();

	INFO << toolBarArea(ui->mainToolBar);

}
void MainWindow::closeEvent(QCloseEvent *e) {
	Q_UNUSED(e)
}
/* ======================================================================== */
/*                             QSerial related                              */
/* ======================================================================== */
void MainWindow::openSerialPort() {
	SettingsDialog::Settings p = settings->settings();
	serial->setPortName(p.ttyName);
	serial->setBaudRate(p.baudrate);
	serial->setDataBits(p.dataBits);
	serial->setParity(p.parity);
	serial->setStopBits(p.stopBits);
	serial->setFlowControl(p.flowCtrl);

	if (serial->open(QIODevice::ReadWrite)) {
		console->setEnabled(true);
		console->setLocalEchoEnabled(p.localEchoOn);
		ui->actConnect->setEnabled(false);
		ui->actDisconnect->setEnabled(true);
		ui->actConfig->setEnabled(false);
		refreshStateMsg();
	}
	else {
		QMessageBox::critical(this, tr("Error"), serial->errorString());

		showStatusMessage(tr("Open error"));
	}
}
void MainWindow::refreshStateMsg() {
	SettingsDialog::Settings p = settings->settings();
	QString msgStr;

	(wrapStateMsgAt < geometry().width())
			? msgStr = stateStr
			: msgStr = stateStrWraped;

	showStatusMessage(msgStr
							.arg(p.ttyName).arg(p.sBaudrate).arg(p.sDataBits)
							.arg(p.sParity).arg(p.sStopBits).arg(p.sFlowCtrl));
}
void MainWindow::closeSerialPort() {
	if (serial->isOpen())
		serial->close();

//	console->setEnabled(false);
	ui->actConnect->setEnabled(true);
	ui->actDisconnect->setEnabled(false);
	ui->actConfig->setEnabled(true);
	showStatusMessage(tr("Disconnected"));

	QSETTINGS;
}
void MainWindow::writeData(const QByteArray &data) {
	serial->write(data);
}
void MainWindow::readData() {
	QByteArray data = serial->readAll();
	console->putData(data);
}
void MainWindow::handleError(QSerialPort::SerialPortError error) {
	if (error == QSerialPort::ResourceError) {
		QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
		closeSerialPort();
	}
}
/* ======================================================================== */
/*                                 Helpers                                  */
/* ======================================================================== */
void MainWindow::initActionsConnections() {
	connect(ui->actConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
	connect(ui->actDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
	connect(ui->actQuit, &QAction::triggered, this, &MainWindow::close);
	connect(ui->actConfig, &QAction::triggered, settings, &MainWindow::show);
	connect(ui->actClear, &QAction::triggered, console, &Console::clear);
	connect(ui->actAbout, &QAction::triggered, this, &MainWindow::about);
}
void MainWindow::showStatusMessage(const QString &message) {
	status->setText(message);
}
void MainWindow::about() {
	QMessageBox::about(this, "About " + qAppName(),
							 tr("The <b>%1</b> provides standard as also non-standard "
								 "serial port related functions.").arg(qAppName()));
}

/*
void MainWindow::onSetAppearanceOpts(bool checked) {
	if (checked) {
		QMap<QString, MainWindow::Appearance> *appearanceOptions =
				new QMap<QString, MainWindow::Appearance>;
		cbxAppearanceOpts = new QComboBox();
		QRect pg = geometry();

		cbxAppearanceOpts->addItems(QList<QString>(appearanceOptions->keys()));
		QMapIterator<QString, MainWindow::Appearance> it(*appearanceOptions);

		while (it.hasNext())
			if (it.next().value() == appearance())
				cbxAppearanceOpts->setCurrentText(it.key());

		connect(cbxAppearanceOpts, SIGNAL(currentIndexChanged()),
				  this, SLOT(onCbxChanged()));
		connect(cbxAppearanceOpts, &QComboBox::close, this, &MainWindow::onCbxClosed);
		cbxAppearanceOpts->setGeometry(pg.x() + pg.width(), pg.y(), 50, 250);
		cbxAppearanceOpts->show();
	}
	else {
		disconnect(cbxAppearanceOpts, SIGNAL(currentIndexChanged()),
				  this, SLOT(onCbxChanged()));
		disconnect(cbxAppearanceOpts, &QComboBox::close, this, &MainWindow::onCbxClosed);
		cbxAppearanceOpts->deleteLater();
	}
}
void MainWindow::onCbxChanged(const QString &str) {
	INFO << str << static_cast<MainWindow::Appearance>(
				  cbxAppearanceOpts->currentData().toUInt());

	setAppearance(static_cast<MainWindow::Appearance>(
						  cbxAppearanceOpts->currentData().toUInt()));
}

void MainWindow::onCbxClosed() {
	onSetAppearanceOpts(false);
}

void MainWindow::onSetOpacity(bool checked) {
	if (checked) {
		sldrOpacity = new QSlider(Qt::Vertical, 0);
		QRect pg = geometry();

		sldrOpacity->setRange(0,100);
		sldrOpacity->setValue(int(windowOpacity()*100));
		connect(sldrOpacity, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
//		connect(sldrOpacity, &QSlider::, this, &MainWindow::onSliderDestroyed);
		sldrOpacity->setGeometry(pg.x() + pg.width(), pg.y(), 50, 250);
		sldrOpacity->show();
	}
	else {
		disconnect(sldrOpacity, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
		sldrOpacity->deleteLater();
	}
}
void MainWindow::onSliderChanged(int value) {
	setWindowOpacity(qreal(value)/100);
}
*/
