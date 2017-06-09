#ifndef CONSOLE_H
#define CONSOLE_H
#include <QSettings>
#include <QPlainTextEdit>
#include <QTextEdit>
#include "globals.h"
#include <QKeyEvent>
#include <QDebug>

#define PLAIN_TEXT_EDIT

#ifdef PLAIN_TEXT_EDIT
class Console : public QPlainTextEdit {
#else
class Console : public QTextEdit {
#endif
	Q_OBJECT
	Q_PROPERTY(bool m_autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)

signals:
	 void getData(const QByteArray &data);

public:
	 explicit Console(QWidget *parent = nullptr);
	 void putData(const QByteArray &data);
	 void setLocalEchoEnabled(bool set);
	 bool autoScroll() const  { return m_autoScroll; }
	 void setAutoScroll(bool autoScroll) {
		 m_autoScroll = autoScroll;
		 emit autoScrollChanged(m_autoScroll);
	 }

signals:
	 void autoScrollChanged(bool);

protected:
	 void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
	 void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	 void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
	 void contextMenuEvent(QContextMenuEvent *e) Q_DECL_OVERRIDE;

protected slots:
	 void vScrollBarValueChanged(int value);
	 void wheelEvent(QWheelEvent *e) override;

private:
	 bool localEchoEnabled;
	 int lineCtr;
	 bool m_autoScroll;
	 static QString LINE;
};

#endif // CONSOLE_H
