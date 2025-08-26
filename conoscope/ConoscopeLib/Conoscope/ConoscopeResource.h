#ifndef CONOSCOPERESOURCE_H
#define CONOSCOPERESOURCE_H

#include <map>

#include "conoscopeTypes.h"
#include "logger.h"

#include <QElapsedTimer>

#define RESOURCE ConoscopeResource::Instance()

class ConoscopeResource
{
public:

    typedef enum
    {
        ResourceType_Filter,
        ResourceType_Nd,
        ResourceType_Iris
    } ResourceType_t;

private:
    ConoscopeResource();

    ~ConoscopeResource() {}

    static ConoscopeResource* mInstance;

    static ConoscopeResource* GetInstance();

public:
    static ConoscopeResource* Instance();

public:
    QString ToString(Filter_t value);

    QString ToString(Nd_t value, bool bReplaceDot = false);

    QString ToString(IrisIndex_t value, bool bReplaceDot = false);

    QString ToString(WheelState_t value);

    QString ToString(TemperatureMonitoringState_t value);

    int Convert(ResourceType_t eType, QString value);

    void SetLogger(Logger* logger);

    void AppendLog(QString msg);

    void AppendLog(QString header, QString msg);

    void RegisterLogCallback(void (*callback)(char*));

    void RegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString));

    void RegisterWarningCallback(void (*callback)(QString));

    void Log(QString message);

    void SendEvent(ConoscopeEvent_t event);

    void SendWarning(QString description = "");

    void TaktTimeStart();

    qint64 TaktTimeMs();

private:
    std::map<int, QString> mFilterToStringMap;           // Filter_t
    std::map<int, QString> mNdToStringMap;               // Nd_t
    std::map<int, QString> mIrisIndexToStringMap;        // IrisIndex_t
    std::map<int, QString> mWheelStateIndexToStringMap;
    std::map<int, QString> mTemperatureMonitoringToStringMap;

    std::map<QString, int> mFilterToStringMapRevert;     // Filter_t
    std::map<QString, int> mNdToStringMapRevert;         // Nd_t
    std::map<QString, int> mIrisIndexToStringMapRevert;  // IrisIndex_t

    QString _ToString(std::map<int, QString>&map, int value, bool bReplaceDot = false);
    int _ToInt(std::map<QString, int>&map, QString value);

    Logger* mLogger;

    void (*mLogCallback)(char*);
    void (*mEventCallback)(ConoscopeEvent_t, QString);
    void (*mWarningCallback)(QString);

    QElapsedTimer taktTimer;
};


#endif /* CONOSCOPERESOURCE_H */
