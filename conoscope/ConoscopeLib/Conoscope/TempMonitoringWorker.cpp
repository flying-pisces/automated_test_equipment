#include "TempMonitoringWorker.h"

#include <QApplication>

#include "ConoscopeResource.h"

#define LOG_CW(x) Log("              Worker", x)

#define MAX_ATTEMPTS    2000
#define SLEEP_TIME_MS   100
#define TIME_QUANTA_MS  10

#define LOG_HEADER "[tempWorker]"
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))

TempMonitoringWorker::TempMonitoringWorker(Camera *camera, QObject *parent) : ClassCommon(parent)
{
    isRunning = false;
    mCamera = camera;
}

TempMonitoringWorker::~TempMonitoringWorker()
{
}

void TempMonitoringWorker::OnWorkRequest(int value, void *parameter)
{
    isRunning = true;

    LogInFile(QString("OnWorkRequest [%1]").arg(EnumToString("Request", value)));

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    Request eRequest = static_cast<Request>(value);

    CmdSetTemperatureParam_t* pCmdSetTemperatureParam;

    switch(eRequest)
    {
    case Request::CmdSetTemperature:
        // retrieve parameter
        pCmdSetTemperatureParam = (CmdSetTemperatureParam_t*)parameter;

        // execute command
        eError = _CmdSetTemperature(pCmdSetTemperatureParam->temperature, pCmdSetTemperatureParam->timeoutMs);
        break;

    default:
        break;
    }

/*
    LOG_CW(QString("done     request [%1] %2")
        .arg(EnumToString("Request", (int)eRequest))
        .arg((eError == ClassCommon::Error::Ok) ? "Ok" : "Failed"));
*/

    emit WorkDone(value, (int)eError);

    isRunning = false;
}

ClassCommon::Error TempMonitoringWorker::CancelCmd()
{
    LogInFile("CancelCmd");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    bCancelRequest = true;

    return eError;
}

ClassCommon::Error TempMonitoringWorker::_CmdSetTemperature(float temperature, int timeoutMs)
{
    LogInFile("_CmdSetTemperature");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    bCancelRequest = false;
    eError = _SetupTemperature(temperature, timeoutMs);

    return eError;
}

#define PID_P 0.2
#define PID_I 0.0015
#define PID_D 0

#define PID_TARGET_MIN 10
#define PID_TARGET_MAX 50

#define FAN_INDEX     1
#define FAN_SPEED     80.0f
#define FAN_SPEED_LOW 40.0f

// #define QUIET_MODE

void TempMonitoringWorker::_CheckCmdIsAborted(ClassCommon::Error& eError)
{
    if(bCancelRequest == true)
    {
        eError = ClassCommon::Error::Aborted;
        bCancelRequest = false;
    }
}

ClassCommon::Error TempMonitoringWorker::_SetupTemperature(float targetTemp, int timeoutMs)
{
    LogInFile("_SetupTemperature");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // check parameter
    if((targetTemp< PID_TARGET_MIN) || (targetTemp > PID_TARGET_MAX))
    {
        eError = ClassCommon::Error::InvalidParameter;
    }

#ifndef MONITOR_PID_TEMP
    float fCurrentTempHot;
    float fCurrentTempCold;
    float fCurrentTempCMOS;
#endif

    if(eError == ClassCommon::Error::Ok)
    {
        // configure the PID
        // modif: keep the default configuration
        // mCamera->SetFloatValue("PIDP", PID_P);
        // mCamera->SetFloatValue("PIDI", PID_I);
        // mCamera->SetFloatValue("PIDD", PID_D);

        // check if the device is correctly connected
#ifndef MONITOR_PID_TEMP
        fCurrentTempHot  = mCamera->GetHw("TEC", "HotTemperature");
        fCurrentTempCold = mCamera->GetHw("Sensor", "ColdTemperature");
        fCurrentTempCMOS = mCamera->GetHw("Sensor", "CmosTemperature");

        float ITec = mCamera->GetHw("TEC", "AdcITec");

        if((fCurrentTempHot == -255.0) ||
           (fCurrentTempCold == -255) ||
           (fCurrentTempCMOS == -255) ||
           (ITec == -255.0))
        {
            // _Log("  Error: can not read sensors");
            eError = ClassCommon::Error::Failed;
        }
#endif
    }

    _CheckCmdIsAborted(eError);

    if(eError == ClassCommon::Error::Ok)
    {
#ifndef QUIET_MODE
        mCamera->SetHw("Fan", "Speed", FAN_SPEED, FAN_INDEX);
#else
        mCamera->SetHw("Fan", "Speed", FAN_SPEED_LOW, FAN_INDEX);
#endif
        mCamera->SetFloatValue("PIDTarget", targetTemp);
    }

    _CheckCmdIsAborted(eError);

    if(eError == ClassCommon::Error::Ok)
    {
        // wait for the temperature is set

        bool processDone = false;
#ifndef MONITOR_PID_TEMP
        int intCounterDefect = 0;
#endif

        // step ends when the temperature reaches setpoint +/- <0.2> degree
        float  temperatureLockedCriteria = 0.2F;

        // during at least <10000> ms
        int    intTempLockedTimeMS       = 10000;

        // Max time to reach setPoint
        qint64 timeoutMicroSec           = timeoutMs * 1000;

#ifndef MONITOR_PID_TEMP
        // Diff max temperature entre la temperature du CMOS et Temperature du drain thermique
        // Si > fMaxDiffTemp : pb collage du capteur ou du CMOS
        float  fMaxDiffTemp              = 10;
#endif

        qint64 elapsed = 0;
        qint64 setTargetTime = 0;
        qint64 lockedStateTime = 0;

        do
        {
#ifndef MONITOR_PID_TEMP
            fCurrentTempCold = mCamera->GetHw("Sensor", "ColdTemperature");
            fCurrentTempCMOS = fGetTrueCMOSTemperature() ;
#endif

#ifdef REMOVED
            fCurrentTempPid = tcTestCase::m_oqcResource->m_camera->GetFloatValue("PIDTemp");
            fCurrentTempHot = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "HotTemperature");

            ITec = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "AdcITec");
            VTec = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "VTec");
            DacOutput = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "DacOutput");
            AdcVIn = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "AdcVin");
            Adc5V = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "Adc5V");
            Adc3V3 = tcTestCase::m_oqcResource->m_camera->GetHw("TEC", "Adc3V3");

            QString result = QString("%1").arg(QString::number(pTimerTest->elapsed(), 'f', 0));
            result.append(QString(",%1").arg(QString::number(sensorTarget, 'f', 0)));

            result.append(QString(",%1").arg(QString::number(tcTestCase::m_oqcResource->m_camera->GetFloatValue("PIDTarget"), 'f', 0)));

            result.append(QString(",%1").arg(QString::number(fCurrentTempPid, 'f', 0)));
            result.append(QString(",%1").arg(QString::number(fCurrentTempCold, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(fCurrentTempCMOS, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(fCurrentTempHot, 'f', 2)));

            result.append(QString(",%1").arg(QString::number(ITec, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(VTec, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(DacOutput, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(AdcVIn, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(Adc5V, 'f', 2)));
            result.append(QString(",%1").arg(QString::number(Adc3V3, 'f', 2)));

            ResultFileAppend(LOG_MEAS, result);
#endif

            // check target
            if(mCamera->GetFloatValue("PIDTarget") != targetTemp)
            {
                mCamera->SetFloatValue("PIDTarget", targetTemp);
            }

            // check diff between temperatures
#ifndef MONITOR_PID_TEMP
            if(fabs(fCurrentTempCold - fCurrentTempCMOS ) > fMaxDiffTemp)
            {
                 if (intCounterDefect > 5)
                 {
                    // LOG("Delta between CMOS Temp and Cold Temp is above limit");
                    // LOG(QString("    Cold = %1 CMOS = %2").arg(QString::number(fCurrentTempCold, 'f', 2)).arg(QString::number(fCurrentTempCMOS, 'f', 2)));
                    // LOG("    Please check if the CMOS is glued correctly");
                    // LOG("    Please check if the TMP101 is glued to the drain");

                    eError = ClassCommon::Error::FailedMaxRetry;

                    // RESULT_INSERT("meas",
                    //     QString("ERROR Too much difference between CMOS Temp and Cold Temp is too big! (Cold = %1 CMOS = %2)").arg(QString::number(fCurrentTempCold, 'f', 2)).arg(QString::number(fCurrentTempCMOS, 'f', 2)));
                 }

                 intCounterDefect ++;
            }
            else
            {
                intCounterDefect = 0;
            }
#endif

            // check timeout
            if(eError == ClassCommon::Error::Ok)
            {
                // qint64 time = (pTimerTest->elapsed() - setTargetTime);
                qint64 time = elapsed - setTargetTime;

                if(time > timeoutMicroSec)
                {
                    // timeout to reach temperature
                    eError = ClassCommon::Error::Timeout;
                }
            }

            // if(res == true)
            if(eError == ClassCommon::Error::Ok)
            {
#ifndef MONITOR_PID_TEMP
                float temperatureDelta = fabs(temperature - fCurrentTempCold);
#else
                float currentTemp = 0.0F;
                currentTemp = mCamera->GetFloatValue("PIDTemp");
                float temperatureDelta = fabs(targetTemp - currentTemp);
#endif

                if(temperatureDelta > temperatureLockedCriteria)
                {
                    // stabilizing
                    // lockedStateTime = pTimerTest->elapsed();
                    lockedStateTime = elapsed;
                }
                else
                {
                    // end of the Step ?
                    //qint64 lockedStateDuration = (pTimerTest->elapsed() - lockedStateTime);
                    qint64 lockedStateDuration = (elapsed - lockedStateTime);

                    if(lockedStateDuration > intTempLockedTimeMS)
                    {
                        // done
                        processDone = true;
                    }
                }
            }

/*
            LOG(QString("  %1  %2 %3 (%4) I = %5 V = %6")
                    .arg(logState)
                    .arg(QString::number(sensorTarget, 'f', 2))
                    .arg(QString::number(tcTestCase::m_oqcResource->m_camera->GetFloatValue("PIDTarget"), 'f', 2))
                    .arg(QString::number(fCurrentTempCold, 'f', 2))
                    .arg(QString::number(ITec, 'f', 2))
                    .arg(QString::number(VTec, 'f', 2)));
*/

            _CheckCmdIsAborted(eError);

            if((processDone == false) &&
               (eError == ClassCommon::Error::Ok))
            {
                // Wait(&res, TIME_QUANTA_MS);
                elapsed += TIME_QUANTA_MS;

                QThread::msleep(TIME_QUANTA_MS);
                QApplication::processEvents(QEventLoop::AllEvents, TIME_QUANTA_MS);
            }

        } while((eError == ClassCommon::Error::Ok) && (processDone == false));
    }

    return eError;
}

#ifndef MONITOR_PID_TEMP
float TempMonitoringWorker::fGetTrueCMOSTemperature()
{
    float result = 0;
    bool  blnOk  ;
    int intWatchDog = 5 ;

    static float fLastValue = mCamera->GetHw("Sensor", "ColdTemperature");

    do
    {
        blnOk = true;
        result = mCamera->GetHw("Sensor", "CmosTemperature");

        if(result > 100)
        {
            blnOk = false;
        }
        if(result < -100)
        {
            blnOk = false;
        }
        if(fabs (fLastValue - result) > 10)
        {
            blnOk = false;
        }

        intWatchDog -- ;
    }while((blnOk == false) && (intWatchDog > 0)) ;

    if(blnOk)
    {
        fLastValue = result;
    }
    else
    {
        result = fLastValue;
    }

    return result;
}
#endif

float TempMonitoringWorker::GetTemperature()
{
    // LogInFile("GetTemperature");

#ifndef MONITOR_PID_TEMP
    float fCurrentTempCold = mCamera->GetHw("Sensor", "ColdTemperature");
    return fCurrentTempCold;
#else
    return mCamera->GetFloatValue("PIDTemp");
#endif
}
