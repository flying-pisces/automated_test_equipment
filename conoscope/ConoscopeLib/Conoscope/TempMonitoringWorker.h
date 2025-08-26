#ifndef TEMP_MONITORING_WORKER_H
#define TEMP_MONITORING_WORKER_H

#include "classcommon.h"
#include <string>

#include "cameraCmvCxp.h"

#define MONITOR_PID_TEMP

class TempMonitoringWorker : public ClassCommon
{
    Q_OBJECT

public:
    enum class Request
    {
        CmdSetTemperature,
    };
    Q_ENUM(Request)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = TempMonitoringWorker::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    /*!
     *  \brief  constructor
     *  \param  parent
     *  \return none
     */
    explicit TempMonitoringWorker(Camera* camera, QObject *parent = nullptr);

    /*!
     *  \brief  destructor
     *  \param  none
     *  \return none
     */
    ~TempMonitoringWorker();

    float GetTemperature();

    typedef struct
    {
        float temperature;
        int timeoutMs;
    } CmdSetTemperatureParam_t;

    ClassCommon::Error CancelCmd();

    bool IsRunning()
    {
        return isRunning;
    }

private:
    Camera* mCamera;

    ClassCommon::Error _CmdSetTemperature(float temperature, int timeoutMs);

    void _CheckCmdIsAborted(ClassCommon::Error& eError);

    ClassCommon::Error _SetupTemperature(float targetTemp, int timeoutMs);

#ifndef MONITOR_PID_TEMP
    float fGetTrueCMOSTemperature();
#endif

    bool bCancelRequest;

    bool isRunning;

public slots:
    void OnWorkRequest(int value, void* parameter);

signals:
    void WorkDone(int value, int error);
};

#endif // TEMP_MONITORING_WORKER_H
