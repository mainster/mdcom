QT += widgets serialport
CONFIG += c++11
#CONFIG += c+99

TARGET = mdcom
TEMPLATE = app

SOURCES += \
	main.cpp \
	mainwindow.cpp \
	settingsdialog.cpp \
	console.cpp \
	globals.cpp \
	types.cpp

HEADERS += \
	mainwindow.h \
	settingsdialog.h \
	console.h \
	globals.h \
	types.h

FORMS += \
	mainwindow.ui \
	settingsdialog.ui

RESOURCES += \
	mdcom.qrc

# target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
# INSTALLS += target

#prescaler
