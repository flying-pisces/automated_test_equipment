#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// log activated only in release mode
#ifdef QT_NO_DEBUG
#define LOGGING 1
#else
#define SKIP_FOR_DEBUG
#endif

#define APPLICATION_NAME   "CONOSCOPE_DEMO"

#define VERSION_MAJOR      0
#define VERSION_MINOR      14
#define VERSION_REV        37
#define RELEASE_DATE       "2020/11/18"
#define VERSION_STR        QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_REV)
#define APPLICATION_TITLE  QString("%1 - %2 (%3)").arg(APPLICATION_NAME).arg(VERSION_STR).arg(RELEASE_DATE)

#endif // CONFIGURATION_H
