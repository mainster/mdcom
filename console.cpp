#include "console.h"
#include <QScrollBar>
#include <QtCore/QDebug>
#include "globals.h"
#include <QString>

#ifdef PLAIN_TEXT_EDIT
QString Console::LINE = QString("%1: %2");
#else
QString Console::LINE = QString("<span style=\"color: red\">%1: </span>%2<br>");
#endif

#ifdef PLAIN_TEXT_EDIT
Console::Console(QWidget *parent) : QPlainTextEdit(parent),
	#else
Console::Console(QWidget *parent) : QTextEdit(parent),
	#endif
	localEchoEnabled(false), m_autoScroll(true) {
	QSETTINGS;

	document()->setMaximumBlockCount(
				config.value("maxBlockCount", 500).toInt());
	QPalette p = palette();
	p.setColor(QPalette::Base, Qt::black);
	p.setColor(QPalette::Text, Qt::green);
	setPalette(p);

	QScrollBar *vScrollBar = verticalScrollBar();
	vScrollBar->triggerAction(QScrollBar::SliderToMinimum);
	connect(this->verticalScrollBar(), &QScrollBar::valueChanged,
			  this, &Console::vScrollBarValueChanged);

}
void Console::vScrollBarValueChanged(int value) {
	if (value == verticalScrollBar()->maximum())
		setAutoScroll(true);
}
void Console::putData(const QByteArray &data) {
	(lineCtr >= 1000) ? lineCtr=0 : lineCtr++;

#ifdef PLAIN_TEXT_EDIT
//	insertPlainText(LINE.arg(lineCtr, 3, 10, QChar(' ')).arg(QString(data)));
	insertPlainText(QString(data));
#else
	insertHtml(LINE.arg(lineCtr).arg(QString(data)));
#endif

	if (autoScroll()) {
		QScrollBar *bar = verticalScrollBar();
		bar->setValue(bar->maximum());
	}
}
void Console::setLocalEchoEnabled(bool set) {
	localEchoEnabled = set;
}
void Console::keyPressEvent(QKeyEvent *e) {
	switch (e->key()) {
		case Qt::Key_Backspace:
		case Qt::Key_Left:
		case Qt::Key_Right:
		case Qt::Key_Up:
		case Qt::Key_Down:
			break;

		default:
#ifdef PLAIN_TEXT_EDIT
			if (localEchoEnabled) QPlainTextEdit::keyPressEvent(e);
#else
			if (localEchoEnabled) QTextEdit::keyPressEvent(e);
#endif
            if (e->text().toLocal8Bit() == "\r")
                emit getData(QString("\n").toLocal8Bit());
            else
                emit getData(e->text().toLocal8Bit());
	}
}
void Console::mousePressEvent(QMouseEvent *e) {
	Q_UNUSED(e)
	setFocus();
}
void Console::mouseDoubleClickEvent(QMouseEvent *e) {
	Q_UNUSED(e)
}
void Console::contextMenuEvent(QContextMenuEvent *e) {
	Q_UNUSED(e)
}
void Console::wheelEvent(QWheelEvent *e) {
	QScrollBar *bar = verticalScrollBar();
	QPoint numDegrees = e->angleDelta() / 8;
	int oldVal = bar->value();

	if (!numDegrees.isNull()) {
		QPoint numSteps = numDegrees / 15;
		bar->setValue(
					bar->value() -
					(numSteps.y() * ((e->modifiers() == Qt::ControlModifier) ? 3 : 1)));
	}

	setAutoScroll((oldVal == bar->value()) ? true : false);
	e->accept();
}
