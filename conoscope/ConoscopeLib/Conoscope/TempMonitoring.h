#ifndef TEMP_MONITORING_H
#define TEMP_MONITORING_H

#include <string>

#include "camera.h"
#include "TempMonitoringWorker.h"
#include "conoscopeTypes.h"

class TempMonitoring : public ClassThreadCommon
{
    Q_OBJECT

public:
    enum class Event {
        CmdSetTemperature,
        CmdSetTemperatureDone,
        CmdSetTemperatureAborted,
        CmdSetTemperatureError,
        CmdCancel,
    };
    Q_ENUM(Event)

    enum class State {
        Undefined,                   /*< worker is not started */
        Idle,                        /*< module is started */

        CmdSetTemperatureProcessing, /*< setting the temperature */

        TemperatureSet,
        Aborted,
        Error,
    };
    Q_ENUM(State)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = TempMonitoring::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    TempMonitoring(Camera* camera, QObject *parent = nullptr);

    ~TempMonitoring();

    void Start();

    ClassCommon::Error Stop();

    void StopThread();

    ClassCommon::Error CmdSetTemperature(float temperature, int timeoutMs);

    ClassCommon::Error CmdCancel();

    ClassCommon::Error CmdReset();

    void GetTemperature(float& temp);

    void GetTemperature(TemperatureMonitoringState_t& eStatus,
                        float& temp);

    float GetTemperatureTarget();

    void GetStatus(TemperatureMonitoringState_t& eStatus);

public:
    static QString _message;

private:
    #define MESSAGE_MAX_SIZE 5000

    void _Log(QString message)
    {
        QStringList messages = message.split("\r\n");

        _Log(messages);
    }

    void _Log(QStringList messages)
    {
        for(int index = 0; index < messages.length(); index ++)
        {
            if(!messages[index].isEmpty())
            {
                _message.append(messages[index]);
                _message.append("\r\n");
            }
        }

        if(_message.size() > MESSAGE_MAX_SIZE)
        {
            _message.clear();
        }
    }

    bool SendRequest(TempMonitoringWorker::Request event, void *parameter);

    void _SetState(State eState);

    ClassCommon::Error ChangeState(State eState);

    ClassCommon::Error ChangeState(State eState, void *parameter);

    ClassCommon::Error ProcessStateMachine(Event eEvent);

    ClassCommon::Error ProcessStateMachine(Event eEvent, void *parameter);

    State mState;

    TempMonitoringWorker* mWorker;

    void on_conoscopeProcess_Log(QString message);

    TempMonitoringWorker::CmdSetTemperatureParam_t mCmdSetTemperatureParam;

    TemperatureMonitoringState_t mStatus;

public slots:
    void on_worker_jobDone(int value, int error);

protected:
    void run();

};

#endif // TEMP_MONITORING_H
