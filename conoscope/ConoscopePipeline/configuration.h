#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// log activated only in release mode

#define APPLICATION_NAME   "PIPELINE_LIB"

#define VERSION_MAJOR      0
#define VERSION_MINOR      4
#define VERSION_REV        11
#define RELEASE_DATE       "2020/11/30"
#define VERSION_STR        QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_REV)
#define APPLICATION_TITLE  QString("%1 - %2 (%3)").arg(APPLICATION_NAME).arg(VERSION_STR).arg(RELEASE_DATE)

#endif // CONFIGURATION_H
