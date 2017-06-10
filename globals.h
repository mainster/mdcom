#ifndef GLOBALS_H
#define GLOBALS_H

#define	PLATFORM_CONFIG_PATH	QString( qgetenv("HOME") + "/.config/" )

#ifndef INFO
#define INFO	qDebug().noquote()
#endif

#include "types.h"

#define PONAM(o) (o->setObjectName(#o),o)		//!< Set pointed objects name
#define ONAM(o) (o.setObjectName(#o),o)		//!< Set objects name

/* ======================================================================== */
/*                       Paths / vendor informations                        */
/* ======================================================================== */

#ifndef ORGANISATION
#define ORGANISATION		QString("delBassoDEV")
#endif
#ifndef CONFIG_PATH
#define CONFIG_PATH	QString( PLATFORM_CONFIG_PATH + ORGANISATION )
#endif

#ifndef CONFIG_FILE
#define CONFIG_FILE	QString( QDir(CONFIG_PATH).absoluteFilePath("config") )
#endif

#define CUSTOM_QUERYS_PATH QString(CONFIG_PATH + tr("/") + qApp->applicationName() + "_customQuerys")

#define QSETTINGS  QSettings config;
#define QSETTINGS_QUERYS QSettings configQ(CUSTOM_QUERYS_PATH, QSettings::IniFormat);
#define QSETTINGS_INIT { \
	QCoreApplication::setOrganizationName(ORGANISATION); \
	QCoreApplication::setApplicationName(qApp->applicationName()); \
	QSettings::setDefaultFormat(QSettings::IniFormat); \
	QSettings cfg; \
	if (! cfg.allKeys().contains(QString("OrganisationName"))) \
	cfg.setValue(QString("OrganisationName"), QCoreApplication::organizationName()); \
	if (! cfg.allKeys().contains(QString("ApplicationName"))) \
	cfg.setValue(QString("ApplicationName"), QCoreApplication::applicationName()); \
	qDebug() << cfg.fileName(); \
	}

class Globals {
public:
	Globals();
};

#endif // GLOBALS_H
