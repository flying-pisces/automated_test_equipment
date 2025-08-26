#ifndef CONOSCOPESTATICTYPES_H
#define CONOSCOPESTATICTYPES_H

#include <QString>

typedef struct
{
    QString timeStampString;
    QString capturePath;

    QString cameraCfgFileName;
    QString opticalColumnCfgFileName;
    QString flatFieldFileName;
    QString opticalColumnCfgDate;
    QString opticalColumnCfgTime;
    QString opticalColumnCfgComment;
} SomeInfo_t;

#endif // CONOSCOPESTATICTYPES_H
