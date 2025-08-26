#ifndef APPTYPES_H
#define APPTYPES_H

#include <QJsonArray>
#include <QList>
#include <map>

typedef enum
{
    LogMask_Any,               // can not be disable
    LogMask_State,
    LogMask_StateMachine,
    LogMask_Worker,
    LogMask_Error,             // can not be disable
} LogMask_t;

class LogMask
{
public:
    LogMask();

    LogMask(QList<LogMask_t> input);

    LogMask(QJsonArray input);

    QJsonArray GetJson();

    bool IsPresent(LogMask_t mask);

private:
    QList<LogMask_t> mLogMasks;

    static std::map<LogMask_t, QString> mLogMaskToStringMap;    // LogMask_t
    static std::map<QString, LogMask_t> mLogMaskToStringRevMap; // LogMask_t
};

typedef struct
{
    bool    bAdmin;
    bool    bStreamProcessedData; // display stream done with processed data
    bool    autoExposure;         // enable auto exposure (or not)
    LogMask mLogMasks;            // store all the masks
    bool    enableWarningMessage; // display the popup with warning message
} AppConfig_t;

#endif // APPTYPES_H
