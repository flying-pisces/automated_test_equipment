#include "CaptureSequenceThread.h"

#include <QDir>

#include "imageConfiguration.h"

#include "toolString.h"

#include "ConoscopeAppProcess.h"
#include "ConoscopeResource.h"

#include "ConoscopeAppHelper.h"

#define RESOURCE ConoscopeResource::Instance()

#define FILE_NAME "%1_nd_%2_iris_%3"

int CaptureSequenceThread::instanceCounter = 0;

QSemaphore     CaptureSequenceThread::mSemaSetup;
QSemaphore     CaptureSequenceThread::mSemaProcessing;

QList<Filter_t>     *CaptureSequenceThread::mFilterList = nullptr;
QMap<Filter_t, int> *CaptureSequenceThread::mExposureTimeList = nullptr;
QMap<Filter_t, CaptureSequenceBuffer_t> *CaptureSequenceThread::mBufferList = nullptr;
CaptureSequenceResult_t CaptureSequenceThread::mResult;

bool CaptureSequenceThread::mErrorOccurs = false;

CaptureSequenceThread::CaptureSequenceThread(QObject *parent)
    : QThread(parent), ConoscopeAppHelper(parent)
{
    mErrorOccurs = false;

    instanceIndex = CaptureSequenceThread::instanceCounter;
    CaptureSequenceThread::instanceCounter ++;

    mLogHeader = QString("[CaptureSeqThread %1]").arg(instanceIndex);
    mLogAppHeader = QString("CaptureSeqThread[%1]").arg(instanceIndex);

    meError = ClassCommon::Error::Ok;
}

CaptureSequenceThread::~CaptureSequenceThread()
{
    CaptureSequenceThread::instanceCounter --;
}

#define SEMA_ACQUIRE_TIMEOUT_MS 30000

void CaptureSequenceThread::CheckStaticError()
{
    if(meError != ClassCommon::Error::Ok)
    {
        mErrorOccurs = true;

        LogInFile(QString("indicate general error"));
        LogInApp(QString("indicate general error"));
    }
}

void CaptureSequenceThread::run()
{
    if(mFilterList == nullptr)
    {
        // the request list is not defined
        return;
    }

    LogInFile(QString("START"));
    LogInApp(QString("START"));

    ConoscopeBehavior_t behaviorConfig;
    CaptureSequenceConfig_t config = ConoscopeAppWorker::mCaptureSequenceConfig;

    int loopIndex = 0;

    int listIndex = loopIndex * instanceCounter + instanceIndex;

    meError = ClassCommon::Error::Ok;

    while((listIndex < mFilterList->count() &&
          (meError == ClassCommon::Error::Ok) &&
          (mErrorOccurs == false)))
    {
        if(mSemaSetup.tryAcquire(1, SEMA_ACQUIRE_TIMEOUT_MS) == false)
        {
            LogInFile(QString("ERROR semaSetup"));
            LogInApp(QString("ERROR semaSetup"));

            meError = ClassCommon::Error::Timeout;
            CheckStaticError();
            break;
        }

        config.exposureTimeUs_FilterX  = mExposureTimeList->value(Filter_X );
        config.exposureTimeUs_FilterXz = mExposureTimeList->value(Filter_Xz);
        config.exposureTimeUs_FilterYa = mExposureTimeList->value(Filter_Ya);
        config.exposureTimeUs_FilterYb = mExposureTimeList->value(Filter_Yb);
        config.exposureTimeUs_FilterZ  = mExposureTimeList->value(Filter_Z );

        Filter_t eFilter = mFilterList->at(listIndex);

        CaptureSequenceBuffer_t buffer = mBufferList->value(eFilter);

        ConoscopeAppWorker::mCaptureSequenceStatus.currentSteps = listIndex + 1;
        ConoscopeAppWorker::mCaptureSequenceStatus.eFilter = eFilter;

        meError = _ProcessSetup(eFilter, config);

        CheckStaticError();

        if(mCancelRequest == true)
        {
            mSemaSetup.release();
            break;
        }

        if(mSemaProcessing.tryAcquire(1, SEMA_ACQUIRE_TIMEOUT_MS) == false)
        {
            LogInFile(QString("ERROR semaProcessing"));
            LogInApp(QString("ERROR semaProcessing"));

            meError = ClassCommon::Error::Timeout;
            CheckStaticError();
            break;
        }

        if(meError == ClassCommon::Error::Ok)
        {
            meError = _ProcessMeasure(eFilter, config, buffer);
            CheckStaticError();

            if(mCancelRequest == true)
            {
                mSemaSetup.release();
                mSemaProcessing.release();
                break;
            }
        }

        mSemaSetup.release();

        if(meError == ClassCommon::Error::Ok)
        {
            meError = _ProcessExport(eFilter, config, buffer);
            CheckStaticError();

            // update map with processed info (conversion factor)
            mBufferList->insert(eFilter, buffer);

            if(mCancelRequest == true)
            {
                mSemaProcessing.release();
                break;
            }
        }

        behaviorConfig.saveParamOnCmd = false;
        behaviorConfig.updateCaptureDate = false;
        ConoscopeAppProcess::SetBehaviorConfig(behaviorConfig);

        mSemaProcessing.release();

        // next loop
        loopIndex ++;
        listIndex = loopIndex * instanceCounter + instanceIndex;
    }

    // the first thread to end moves the wheel in the initial position
    if((meError == ClassCommon::Error::Ok) &&
       (mErrorOccurs == false) &&
       (listIndex == mFilterList->count()))
    {
        if(mSemaSetup.tryAcquire(1, SEMA_ACQUIRE_TIMEOUT_MS) == false)
        {
            LogInFile(QString("ERROR semaSetup last"));
            LogInApp(QString("ERROR semaSetup last"));

            meError = ClassCommon::Error::Timeout;
            CheckStaticError();
        }

        Filter_t eFilter = mFilterList->at(0);

        config.exposureTimeUs_FilterX  = mExposureTimeList->value(Filter_X );
        config.exposureTimeUs_FilterXz = mExposureTimeList->value(Filter_Xz);
        config.exposureTimeUs_FilterYa = mExposureTimeList->value(Filter_Ya);
        config.exposureTimeUs_FilterYb = mExposureTimeList->value(Filter_Yb);
        config.exposureTimeUs_FilterZ  = mExposureTimeList->value(Filter_Z );

        meError = _ProcessSetup(eFilter, config);
        CheckStaticError();

        mSemaSetup.release();
    }

    if(meError == ClassCommon::Error::Ok)
    {
        LogInFile(QString("END"));
        LogInApp(QString("END"));
    }
    else
    {
        LogInFile(QString("END with error (%1)").arg((int)meError));
        LogInApp(QString("END with error (%1)").arg((int)meError));
    }
}

#define MAX_STATUS_RETRY 5

ClassCommon::Error CaptureSequenceThread::_SetupStatus(bool &canRetry)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    SetupStatus_t setupStatus;

    setupStatus.eTemperatureMonitoringState = TemperatureMonitoringState_Error;
    setupStatus.sensorTemperature           = 0;
    setupStatus.eWheelStatus                = WheelState_Idle;
    setupStatus.eFilter                     = Filter_Invalid;
    setupStatus.eNd                         = Nd_Invalid;
    setupStatus.eIris                       = IrisIndex_Invalid;

    canRetry = false;

    // check setup status
    int statusRetry = 0;
    bool bStatusDone = false;

    do
    {
        eError = ConoscopeAppProcess::CmdSetupStatus(setupStatus);

        if(eError == ClassCommon::Error::Ok)
        {
            if(setupStatus.eWheelStatus != WheelState_Success)
            {
                QString wheelStateString = "unknown";

                switch(setupStatus.eWheelStatus)
                {
                    case WheelState_Idle:
                        wheelStateString = "Idle";
                        break;
                    case WheelState_Success:
                        wheelStateString = "Success";
                        break;
                    case WheelState_Operating:
                        wheelStateString = "Operating";
                        break;
                    case WheelState_Error:
                        wheelStateString = "Error";
                        break;
                    default:
                        wheelStateString = "unknown...";
                        break;
                }

                LogInFile(QString("ProcessSetup | WheelState %1").arg(wheelStateString));
                LogInApp(QString("ProcessSetup      WheelState %1").arg(wheelStateString));
            }

            if((setupStatus.eWheelStatus == WheelState_Success) ||
               (setupStatus.eWheelStatus == WheelState_Idle))
            {
                // Status can be idle if the wheel has nothing to do.
                // Requested position is the same has the current one just after power on.
                bStatusDone = true;
            }
            else if(setupStatus.eWheelStatus == WheelState_Operating)
            {
                // wait for the end of operation

                if(statusRetry ++ < MAX_STATUS_RETRY)
                {
                    // check status again
                    bStatusDone = false;
                    QThread::msleep(500);
                }
                else
                {
                    // reach max nb retry
                    bStatusDone = true;
                    eError = ClassCommon::Error::FailedMaxRetry;

                    LogInFile(QString("ProcessSetup | MAX RETRY operating"));
                    LogInApp(QString("ProcessSetup      MAX RETRY operating"));
                }
            }
            else
            {
                bStatusDone = true;
                eError = ClassCommon::Error::Failed;

                // indicate upper layer that a setup can be retryed
                canRetry = true;
            }
        }
    } while(bStatusDone == false);

    return eError;
}

ClassCommon::Error CaptureSequenceThread::_ProcessSetup(Filter_t eFilter, CaptureSequenceConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // check general error (that means error in the other thread)
    if(mErrorOccurs == true)
    {
        return ClassCommon::Error::Failed;
    }

    // setup the conoscope
    LogInFile(QString("ProcessSetup | filter %1").arg(RESOURCE->ToString(eFilter), -10));
    LogInApp(QString("ProcessSetup      filter %1").arg(RESOURCE->ToString(eFilter), -10));

    SetupConfig_t setupConfig;

    setupConfig.sensorTemperature = config.sensorTemperature;
    setupConfig.eFilter = eFilter;
    setupConfig.eNd = config.eNd;
    setupConfig.eIris = config.eIris;

    ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Setup;

    eError = ConoscopeAppProcess::CmdSetup(setupConfig);

    if(eError != ClassCommon::Error::Ok)
    {
        LogInFile("ProcessSetup | CmdSetup Error");
        LogInApp("ProcessSetup      CmdSetup Error");
    }

    if(eError == ClassCommon::Error::Ok)
    {
        if(config.bWaitForSensorTemperature == true)
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_WaitForTemp;
            _WaitForSetup();
        }
    }

    if(eError == ClassCommon::Error::Ok)
    {
        LogInFile("ProcessSetup | Done");
        LogInApp("ProcessSetup      Done");
    }
    else
    {
        LogInFile("ProcessSetup | Error");
        LogInApp("ProcessSetup      Error");
    }

    return eError;
}

ClassCommon::Error CaptureSequenceThread::_ProcessMeasure(Filter_t eFilter, CaptureSequenceConfig_t& config, CaptureSequenceBuffer_t& buffer)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // check general error (that means error in the other thread)
    if(mErrorOccurs == true)
    {
        return ClassCommon::Error::Failed;
    }

    // setup the conoscope
    LogInFile(QString("ProcessMeasure | filter %1").arg(RESOURCE->ToString(eFilter), -10));
    LogInApp(QString("ProcessMeasure    filter %1").arg(RESOURCE->ToString(eFilter), -10));

    MeasureConfigWithCropFactor_t measureConfig;
    SetupConfig_t setupConfig;

    setupConfig.sensorTemperature = config.sensorTemperature;
    setupConfig.eFilter = eFilter;
    setupConfig.eNd = config.eNd;
    setupConfig.eIris = config.eIris;

    float autoExposurePixelMax = ConoscopeAppWorker::mSettings.AELevelPercent;

    switch(eFilter)
    {
        case Filter_X:
            measureConfig.exposureTimeUs = config.exposureTimeUs_FilterX;
            break;
        case Filter_Xz:
            measureConfig.exposureTimeUs = config.exposureTimeUs_FilterXz;
            break;
        case Filter_Ya:
            measureConfig.exposureTimeUs = config.exposureTimeUs_FilterYa;
            break;
        case Filter_Yb:
            measureConfig.exposureTimeUs = config.exposureTimeUs_FilterYb;
            break;
        case Filter_Z:
            measureConfig.exposureTimeUs = config.exposureTimeUs_FilterZ;
            break;
        default:
            eError = ClassCommon::Error::InvalidParameter;
            break;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        measureConfig.nbAcquisition = config.nbAcquisition;
        measureConfig.binningFactor = 1;
        measureConfig.bTestPattern = false;

        if(config.bAutoExposure == false)
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Measure;
            eError = _Capture(measureConfig);
        }
        else
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_AutoExpo;
            eError = _CaptureAutoExposure(measureConfig, *(buffer.data), autoExposurePixelMax);

            mExposureTimeList->insert(eFilter, measureConfig.exposureTimeUs);
        }
    }

    if(eError == ClassCommon::Error::Ok)
    {
        LogInFile(QString("ProcessMeasure | Done"));
        LogInApp(QString("ProcessMeasure    Done"));
    }
    else
    {
        LogInFile(QString("ProcessMeasure | Error"));
        LogInApp(QString("ProcessMeasure    Error"));
    }

    return eError;
}

ClassCommon::Error CaptureSequenceThread::_ProcessExport(Filter_t eFilter, CaptureSequenceConfig_t& config, CaptureSequenceBuffer_t& buffer)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // check general error (that means error in the other thread)
    if(mErrorOccurs == true)
    {
        return ClassCommon::Error::Failed;
    }

    // setup the conoscope
    LogInFile(QString("ProcessExport | filter %1").arg(RESOURCE->ToString(eFilter), -10));
    LogInApp(QString("ProcessExport     filter %1").arg(RESOURCE->ToString(eFilter), -10));

    MeasureConfigWithCropFactor_t measureConfig;
    SetupConfig_t setupConfig;

    setupConfig.sensorTemperature = config.sensorTemperature;
    setupConfig.eFilter = eFilter;
    setupConfig.eNd = config.eNd;
    setupConfig.eIris = config.eIris;

    if(eError == ClassCommon::Error::Ok)
    {
        eError = ConoscopeAppProcess::CmdExportProcessed(ConoscopeAppWorker::mCaptureSequenceExportConfig, *(buffer.data), config.bSaveCapture);

        if(eError == ClassCommon::Error::Ok)
        {
            if(ConoscopeAppProcess::cmdExportProcessedOutput.saturationFlag == true)
            {
                mResult.bSaturatedCapture = true;
            }
        }

        // ConoscopeProcess::
        SomeInfo_t info;
        ConoscopeAppProcess::GetSomeInfo(info);

        QString fileName = QString(FILE_NAME).arg(info.timeStampString)
                                             .arg(RESOURCE->ToString(setupConfig.eNd))
                                             .arg(RESOURCE->ToString(setupConfig.eIris, true));

        fileName = QDir::cleanPath(info.capturePath + QDir::separator() + fileName);

        buffer.convFactX = ConoscopeAppProcess::cmdExportProcessedOutput.conversionFactorCompX;
        buffer.convFactY = ConoscopeAppProcess::cmdExportProcessedOutput.conversionFactorCompY;
        buffer.convFactZ = ConoscopeAppProcess::cmdExportProcessedOutput.conversionFactorCompZ;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        LogInFile(QString("ProcessExport | Done"));
        LogInApp(QString("ProcessExport     Done"));
    }
    else
    {
        LogInFile(QString("ProcessExport | Error"));
        LogInApp(QString("ProcessExport     Error"));
    }

    return eError;
}

ClassCommon::Error CaptureSequenceThread::_Capture(MeasureConfigWithCropFactor_t config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    MeasureConfigWithCropFactor_t measureConfig;

    if(eError == ClassCommon::Error::Ok)
    {
        measureConfig.exposureTimeUs = config.exposureTimeUs;
        measureConfig.nbAcquisition = config.nbAcquisition;
        measureConfig.binningFactor = 1;
        measureConfig.bTestPattern = false;

        LogInFile(QString("Capture | CmdMeasure %1 us (%2)").arg(measureConfig.exposureTimeUs).arg(measureConfig.nbAcquisition));
        LogInApp(QString("Capture           CmdMeasure %1 us (%2)").arg(measureConfig.exposureTimeUs).arg(measureConfig.nbAcquisition));
        eError = ConoscopeAppProcess::CmdMeasure(measureConfig);

        if(eError != ClassCommon::Error::Ok)
        {
            LogInFile("Capture | CmdMeasure ERROR");
            LogInApp("Capture           CmdMeasure ERROR");
        }
    }

    return eError;
}


