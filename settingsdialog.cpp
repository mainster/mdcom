#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QWidget>
#include <QComboBox>
#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QVariant>
#include <QtGlobal>

QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");


/* ======================================================================== */
/*                      SettingsDialog::SettingsDialog                      */
/* ======================================================================== */
SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::SettingsDialog) {
	ui->setupUi(this);
	QSETTINGS;

	/**
	 * @brief	Register Meta Type <MainWindow *>
	 */
	qRegisterMetaType<SettingsDialog*>("SettingsDialog");

	try {
		mwin = qobject_cast<QMainWindow *>(
					listFindByName<QWidget *>(qApp->topLevelWidgets(),
													  tr("MainWindow")).first());
	}
	catch (...) { qApp->quit(); }

	/**
	 * @brief	After a valid cast of QMainWindow:: instance, create QMetaEnum::
	 * object from the prior registered MainWindow:: meta type
	 * (qRegisterMetaType<MainWindow*>(...)).
	 *
	 * QMetaEnum:: is used to derive MainWindow::Appearance enumerators via
	 * string representation (and vice versa),
	 */
	metaEnumAppear =
			QMetaType::metaObjectForType(
				QMetaType::type("MainWindow"))->enumerator(
				QMetaType::metaObjectForType(
					QMetaType::type("MainWindow"))->indexOfEnumerator("Appearance"));

	btnGrp = new QButtonGroup();
	QList<QRadioButton *> childs = ui->grBoxWinOpts->findChildren<QRadioButton *>();
	QList<QRadioButton *>::iterator it;

	/**
	 * @brief	Iterate over QRadioButton:: childs as also "through" the list of
	 * enumerators name strings (MainWindow::Apearence) and set the radio button
	 * object names and texts to the corresponding enum string representation.
	 * Also fill up the QButtonGroup:: container.
	 */
	for (it = childs.begin(); it != childs.end(); ++it) {
		int idx = it - childs.begin();
		INFO << tr("it=%1: ").arg(idx) << metaEnumAppear.key(idx);

		(*it)->setObjectName(tr("rb%1").arg(metaEnumAppear.key(idx)));
		(*it)->setText(metaEnumAppear.key(idx));
		btnGrp->addButton(*it);
	}

	/**
	 * @brief	Connect the common signal "QButtonGroup::buttonPressed(..sender..)"
	 * to lambda expression which invokes MainWindow::setAppearance write accessor
	 * wherein accessor payload could be derived via metaEnum and QObject::
	 * objectName() of the actual toggled QRadioButton.
	 */
	connect(btnGrp, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonPressed),
			  this, [=](QAbstractButton *btn) {
		mwin->setProperty("appearance", metaEnumAppear.keyToValue(toCstr(btn->text())));
	});

	connect(ui->sldOpacity, &QSlider::valueChanged, this, [=](int value) {
		mwin->setWindowOpacity(static_cast<qreal>(value)/100.f); });

	connect(ui->cbxPortInfo,
			  static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged),
			  this, &SettingsDialog::showPortInfo);
	connect(ui->cbxBauds,
			  static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged),
			  this, &SettingsDialog::checkCustomBaudRatePolicy);
	connect(ui->cbxPortInfo,
			  static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged),
			  this, &SettingsDialog::checkCustomDevicePathPolicy);

	connect(ui->btnApply, &QPushButton::clicked,		this, &SettingsDialog::accept);

	ui->sldOpacity->setValue(static_cast<int>(mwin->windowOpacity()*100.f+.5f));
	ui->cbxBauds->setInsertPolicy(QComboBox::NoInsert);
	intValidator = new QIntValidator(0, 5e6, this);

	initialPortParams();
	fillPortsInfo();

	updateSettingsStruct(m_activeCfg);
	m_backupCfg = m_activeCfg;
}
SettingsDialog::~SettingsDialog() {
	delete ui;
}
void SettingsDialog::showPortInfo(int idx) {
	if (idx == -1)
		return;

	QStringList list = ui->cbxPortInfo->itemData(idx).toStringList();
	ui->descriptionLabel->setText(tr("Description: %1").arg(
												list.count() > 1 ? list.at(1) : tr(blankString)));
	ui->manufacturerLabel->setText(tr("Manufacturer: %1").arg(
												 list.count() > 2 ? list.at(2) : tr(blankString)));
	ui->serialNumberLabel->setText(tr("Serial number: %1").arg(
												 list.count() > 3 ? list.at(3) : tr(blankString)));
	ui->locationLabel->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(
																									4) : tr(blankString)));
	ui->vidLabel->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ?
																				list.at(5) : tr(blankString)));
	ui->pidLabel->setText(tr("Product Identifier: %1").arg(list.count() > 6 ?
																				 list.at(6) : tr(blankString)));
}
void SettingsDialog::checkCustomBaudRatePolicy(int idx) {
	bool isCustomBaudRate = !ui->cbxBauds->itemData(idx).isValid();
	ui->cbxBauds->setEditable(isCustomBaudRate);

	if (isCustomBaudRate) {
		ui->cbxBauds->clearEditText();
		QLineEdit *edit = ui->cbxBauds->lineEdit();
		edit->setValidator(intValidator);
	}
}
void SettingsDialog::checkCustomDevicePathPolicy(int idx) {
	bool isCustomPath = !ui->cbxPortInfo->itemData(idx).isValid();
	ui->cbxPortInfo->setEditable(isCustomPath);

	if (isCustomPath)
		ui->cbxPortInfo->clearEditText();
}
void SettingsDialog::initialPortParams() {
	bool ok = false;

	metaEnumBauds =
			QMetaType::metaObjectForType(
				QMetaType::type("SettingsDialog"))->enumerator(
				QMetaType::metaObjectForType(
					QMetaType::type("SettingsDialog"))->indexOfEnumerator("BaudRateFast"));

	for (int k=0; k < metaEnumBauds.keyCount(); k++) {
		const char *sKey = metaEnumBauds.key(k);

		if (QString(sKey).contains(tr("UnknownBaud"), Qt::CaseInsensitive))
			continue;
		ui->cbxBauds->addItem(QString(sKey).remove("Baud"),
									 metaEnumBauds.keyToValue(sKey, &ok));

		if (! ok) {
			INFO << tr("metaEnumBauds.keyToValue(sKey, &ok): ok is false!");
			qApp->quit();
		}
	}

	ui->cbxDataBits->addItem(QStringLiteral("5"), QSerialPort::Data5);
	ui->cbxDataBits->addItem(QStringLiteral("6"), QSerialPort::Data6);
	ui->cbxDataBits->addItem(QStringLiteral("7"), QSerialPort::Data7);
	ui->cbxDataBits->addItem(QStringLiteral("8"), QSerialPort::Data8);
	ui->cbxDataBits->setCurrentIndex(3);

	ui->cbxParity->addItem(tr("None"), QSerialPort::NoParity);
	ui->cbxParity->addItem(tr("Even"), QSerialPort::EvenParity);
	ui->cbxParity->addItem(tr("Odd"), QSerialPort::OddParity);
	ui->cbxParity->addItem(tr("Mark"), QSerialPort::MarkParity);
	ui->cbxParity->addItem(tr("Space"), QSerialPort::SpaceParity);

	ui->cbxStopBits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
	ui->cbxStopBits->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
	ui->cbxStopBits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

	ui->cbxFlowCtrl->addItem(tr("None"), QSerialPort::NoFlowControl);
	ui->cbxFlowCtrl->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
	ui->cbxFlowCtrl->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}
void SettingsDialog::fillPortsInfo() {
	ui->cbxPortInfo->clear();
	QString description;
	QString manufacturer;
	QString serialNumber;
	const auto infos = QSerialPortInfo::availablePorts();

	for (const QSerialPortInfo &info : infos) {
		QStringList list;
		description = info.description();
		manufacturer = info.manufacturer();
		serialNumber = info.serialNumber();
		list << info.portName()
			  << (!description.isEmpty() ? description : blankString)
			  << (!manufacturer.isEmpty() ? manufacturer : blankString)
			  << (!serialNumber.isEmpty() ? serialNumber : blankString)
			  << info.systemLocation()
			  << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(),
																			16) : blankString)
			  << (info.productIdentifier() ? QString::number(info.productIdentifier(),
																			 16) : blankString);

		ui->cbxPortInfo->addItem(list.first(), list);
	}

	ui->cbxPortInfo->addItem(tr("Custom"));
}
void SettingsDialog::updateSettingsStruct(Settings &settings) {
	settings.sBaudrate =		QString::number(settings.baudrate);
	settings.ttyName =		ui->cbxPortInfo->currentText();
	settings.sDataBits =		ui->cbxDataBits->currentText();
	settings.sStopBits =		ui->cbxStopBits->currentText();
	settings.sParity =		ui->cbxParity->currentText();
	settings.sFlowCtrl =		ui->cbxFlowCtrl->currentText();
	settings.localEchoOn =	ui->cbLocalEcho->isChecked();

	settings.dataBits = static_cast<QSerialPort::DataBits>(
				ui->cbxDataBits->itemData(ui->cbxDataBits->currentIndex()).toInt());
	settings.parity = static_cast<QSerialPort::Parity>(
				ui->cbxParity->itemData(ui->cbxParity->currentIndex()).toInt());
	settings.stopBits = static_cast<QSerialPort::StopBits>(
				ui->cbxStopBits->itemData(ui->cbxStopBits->currentIndex()).toInt());
	settings.flowCtrl = static_cast<QSerialPort::FlowControl>(
				ui->cbxFlowCtrl->itemData(ui->cbxFlowCtrl->currentIndex()).toInt());
}
void SettingsDialog::promoteSettingsStruct(Settings &settings) {
	ui->cbxPortInfo->setCurrentText(settings.ttyName);
	ui->cbxDataBits->setCurrentText(settings.sDataBits);
	ui->cbxStopBits->setCurrentText(settings.sStopBits);
	ui->cbxParity->setCurrentText(settings.sParity);
	ui->cbxFlowCtrl->setCurrentText(settings.sFlowCtrl);
	ui->cbLocalEcho->setChecked(settings.localEchoOn);

	settings.btnGrpChecked->setChecked(true);
	ui->sldOpacity->setValue(static_cast<int>(settings.sldOpacity*100.f+.5f));

}

void SettingsDialog::accept() {
	updateSettingsStruct(m_activeCfg);
	m_activeCfg.sldOpacity = static_cast<qreal>(ui->sldOpacity->value())/100.f;
	m_activeCfg.btnGrpChecked = btnGrp->checkedButton();
	m_backupCfg = m_activeCfg;
	hide();
}
void SettingsDialog::reject() {
	m_activeCfg = m_backupCfg;
	m_activeCfg.btnGrpChecked->setChecked(true);
	ui->sldOpacity->setValue(static_cast<int>(m_activeCfg.sldOpacity*100.f+.5f));
}
/* ======================================================================== */
/*                              Event handlers                              */
/* ======================================================================== */
void SettingsDialog::showEvent(QShowEvent *e) {
	Q_UNUSED(e)
	INFO << static_cast<int>(mwin->windowOpacity() * 100.f);
	ui->sldOpacity->setValue(static_cast<int>(mwin->windowOpacity() * 100.f));
}
/* ======================================================================== */
/*                                 Getters                                  */
/* ======================================================================== */
SettingsDialog::Settings SettingsDialog::settings() const
{ return m_activeCfg; }
QComboBox *SettingsDialog::getSerialPortInfoListBox()
{ return ui->cbxPortInfo; }
QComboBox *SettingsDialog::getBaudrateBox()
{ return ui->cbxBauds; }

/*
QDataStream &operator<<(QDataStream &out, const SettingsDialog::Settings &v) {
	out << v.name << v.baudRate << v.stringBaudRate << v.dataBits
		 << v.stringDataBits << v.parity << v.stringParity << v.stopBits
		 << v.stringStopBits << v.flowControl << v.stringFlowControl
		 << v.localEchoEnabled;
	return out;
}
QDataStream &operator>>(QDataStream &in, SettingsDialog::Settings &v) {
	in >> v.name;
	in >> v.baudRate ;
	in >> v.stringBaudRate;
	in >> v.stringDataBits;
	in >> v.stringParity;
	in >> v.stringStopBits;
	in >> v.stringFlowControl;
	in >> v.localEchoEnabled;
	//	in >> QSerialPort::DataBits(v.dataBits);
	//	in >> v.parity;
	//	in >> v.stopBits;
	//	in >> v.flowControl;
	return in;
}
*/

