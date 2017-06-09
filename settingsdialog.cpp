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
	QMetaEnum metaEnum =
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
		INFO << tr("it=%1: ").arg(idx) << metaEnum.key(idx);

		(*it)->setObjectName(tr("rb%1").arg(metaEnum.key(idx)));
		(*it)->setText(metaEnum.key(idx));
		btnGrp->addButton(*it);
	}

	/**
	 * @brief	Connect the common signal "QButtonGroup::buttonPressed(..sender..)"
	 * to a lambda expression to invoke MainWindow::setAppearance
	connect(btnGrp, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonPressed),
			  this, [=](QAbstractButton *btn) {
		mwin->setProperty("appearance", metaEnum.keyToValue(toCstr(btn->text())));
	});


	connect(ui->serialPortInfoListBox,
			  static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged),
			  this, &SettingsDialog::showPortInfo);
	connect(ui->baudRateBox,
			  static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged),
			  this, &SettingsDialog::checkCustomBaudRatePolicy);
	connect(ui->serialPortInfoListBox,
			  static_cast<void (QComboBox::*)(const int)>(&QComboBox::currentIndexChanged),
			  this, &SettingsDialog::checkCustomDevicePathPolicy);

	connect(ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::apply);
	connect(ui->sldOpacity, &QSlider::valueChanged, this, &SettingsDialog::updateOptions);
//	connect(btnGrp, &QButtonGroup::, this, &SettingsDialog::updateOptions);

//	connect(btnGrp, static_cast<void(QButtonGroup::*)(QAbstractButton *, bool)>(
//				  &QButtonGroup::buttonToggled),[=](QAbstractButton *button, bool checked){ /* ... */ });

//	connect(btnGrp, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),
//					this, &SettingsDialog::close);


	intValidator = new QIntValidator(0, 5e6, this);
	ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

	fillPortsParameters();
	fillPortsInfo();

	ui->sldOpacity->setObjectName(tr("sldOpacity"));
	ui->valOpacity->setBuddy( ui->sldOpacity );

	updateSettings();
}
SettingsDialog::~SettingsDialog() {
	delete ui;
}
void SettingsDialog::updateOptions(const int value) {
	if (! qobject_cast<QWidget *>(sender()))
		return;

	if (qobject_cast<QWidget *>(sender()) == ui->valOpacity->buddy()) {
		qreal newOpacity = qreal(value)/100;
		ui->valOpacity->setNum(newOpacity);
		mwin->setWindowOpacity(newOpacity);
	}
}

void SettingsDialog::apply() {
	updateSettings();
	hide();
}
void SettingsDialog::showPortInfo(int idx) {
	if (idx == -1)
		return;

	QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
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
	bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
	ui->baudRateBox->setEditable(isCustomBaudRate);

	if (isCustomBaudRate) {
		ui->baudRateBox->clearEditText();
		QLineEdit *edit = ui->baudRateBox->lineEdit();
		edit->setValidator(intValidator);
	}
}
void SettingsDialog::checkCustomDevicePathPolicy(int idx) {
	bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
	ui->serialPortInfoListBox->setEditable(isCustomPath);

	if (isCustomPath)
		ui->serialPortInfoListBox->clearEditText();
}
void SettingsDialog::fillPortsParameters() {
	ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
	ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
	ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
	ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
	ui->baudRateBox->addItem(tr("Custom"));

	ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
	ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
	ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
	ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
	ui->dataBitsBox->setCurrentIndex(3);

	ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
	ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
	ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
	ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
	ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

	ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
	ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
	ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

	ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
	ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
	ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}
void SettingsDialog::fillPortsInfo() {
	ui->serialPortInfoListBox->clear();
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

		ui->serialPortInfoListBox->addItem(list.first(), list);
	}

	ui->serialPortInfoListBox->addItem(tr("Custom"));
}
void SettingsDialog::updateSettings() {
	currentSettings.name = ui->serialPortInfoListBox->currentText();

	if (ui->baudRateBox->currentIndex() == 4) {
		currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
	} else {
		currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
					ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
	}

	currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

	currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
				ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
	currentSettings.stringDataBits = ui->dataBitsBox->currentText();

	currentSettings.parity = static_cast<QSerialPort::Parity>(
				ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
	currentSettings.stringParity = ui->parityBox->currentText();

	currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
				ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
	currentSettings.stringStopBits = ui->stopBitsBox->currentText();

	currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
				ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
	currentSettings.stringFlowControl = ui->flowControlBox->currentText();

	currentSettings.localEchoEnabled = ui->localEchoCheckBox->isChecked();

}
/* ======================================================================== */
/*                                 Getters                                  */
/* ======================================================================== */
SettingsDialog::Settings SettingsDialog::settings() const
{ return currentSettings; }
QComboBox *SettingsDialog::getSerialPortInfoListBox()
{ return ui->serialPortInfoListBox; }
QComboBox *SettingsDialog::getBaudrateBox()
{ return ui->baudRateBox; }

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
