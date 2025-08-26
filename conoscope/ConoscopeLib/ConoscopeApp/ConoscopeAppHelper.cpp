#include "ConoscopeAppHelper.h"
#include "imageConfiguration.h"
#include "configuration.h"

#include "ConoscopeResource.h"

#include "ConoscopeAppProcess.h"
#include "ConoscopeAppWorker.h"

#include "toolString.h"

bool ConoscopeAppHelper::mCancelRequest = false;

#define LOG_HEADER "[conoscopeAppHelper]"
// #define LogInFile(text) RESOURCE->AppendLog(QString("%1 | ").arg(LOG_HEADER, -20), text)

#define LOG_APP_HEADER "[APPHelper]"
// #define LogInApp(text) RESOURCE->Log(QString("%1 %2").arg(LOG_APP_HEADER, -20).arg(text));

ConoscopeAppHelper::ConoscopeAppHelper(QObject *parent): ClassCommon(parent)
{
}

ConoscopeAppHelper::~ConoscopeAppHelper()
{
}

#define SATURATION_VALUE 4090

#define MIN_EXPOSURE_TIME_US 10
#define MAX_EXPOSURE_TIME_US 985000
#define MAX_AE_LOOP_COUNT    20

#define ALIGN_VALUE(a, b) a - (a % b)

#define EXPOSURE_TIME_GRANULARITY_FOR_AE
#ifdef EXPOSURE_TIME_GRANULARITY_FOR_AE
int ConoscopeAppHelper::_GetExposureTime(int exposureTimeUs, int granularity, int min, int max)
{
    int newExposureTimeUs = exposureTimeUs;

    // apply the granularity to the exposure time
    if(ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs > 1)
    {
        if(exposureTimeUs > ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs)
        {
            int mod = exposureTimeUs % granularity;

            newExposureTimeUs = exposureTimeUs - mod;

            LogInApp(QString("            [AutoExp] granularity %1 to %2").arg(exposureTimeUs, 6).arg(newExposureTimeUs, 6));

            if(mod > (granularity / 2))
            {
                newExposureTimeUs += granularity;

                LogInApp(QString("                                         to %1").arg(newExposureTimeUs, 6));
            }
        }
        else
        {
            newExposureTimeUs = granularity;
        }
    }

    if(min != 0)
    {
        while(newExposureTimeUs < min)
        {
            newExposureTimeUs += granularity;
        }
    }

    if(max != 0)
    {
        while(newExposureTimeUs > max)
        {
            newExposureTimeUs -= granularity;
        }
    }

    return newExposureTimeUs;
}
#endif

ClassCommon::Error ConoscopeAppHelper::_CaptureAutoExposure(MeasureConfigWithCropFactor_t& config, std::vector<int16_t>& mBuffer, float autoExposurePixelMax)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    int width;
    int height;
    int maxValue;

    AutoExposureParam_t aeParam;

    aeParam.targetMax = autoExposurePixelMax * SATURATION_VALUE;
    aeParam.targetMax = aeParam.targetMax / 100;

    float conAETargetMargin = 0.02F;
    aeParam.thresholdUp   = aeParam.targetMax * (1 + conAETargetMargin);
    aeParam.thresholdDown = aeParam.targetMax * (1 - conAETargetMargin);

    aeParam.saturation = SATURATION_VALUE;

    aeParam.decreasingFactor = 2;
    aeParam.increasingFactor = 5;
    aeParam.noiseLevelRatio  = 3;

    aeParam.sngNoise = 150;

    bool bLocked = false;
    ProcessingConfig_t processingConfig;

    // do not save processing configuration
    ConoscopeBehavior_t behavior;
    behavior.saveParamOnCmd = false;
    behavior.updateCaptureDate = true;
    eError = ConoscopeAppProcess::SetBehaviorConfig(behavior);

    // get min and max exposure time
    aeParam.minExposureTimeUs = MIN_EXPOSURE_TIME_US;
    aeParam.maxExposureTimeUs = MAX_EXPOSURE_TIME_US;

    // retrieve configuration
    if(ConoscopeAppWorker::mSettings.AEMaxExpoTimeUs > ConoscopeAppWorker::mSettings.AEMinExpoTimeUs)
    {
        if((ConoscopeAppWorker::mSettings.AEMinExpoTimeUs >= MIN_EXPOSURE_TIME_US) &&
           (ConoscopeAppWorker::mSettings.AEMinExpoTimeUs <  MAX_EXPOSURE_TIME_US) &&
           (ConoscopeAppWorker::mSettings.AEMaxExpoTimeUs >  MIN_EXPOSURE_TIME_US) &&
           (ConoscopeAppWorker::mSettings.AEMaxExpoTimeUs <= MAX_EXPOSURE_TIME_US))
        {
            aeParam.minExposureTimeUs = ConoscopeAppWorker::mSettings.AEMinExpoTimeUs;
            aeParam.maxExposureTimeUs = ConoscopeAppWorker::mSettings.AEMaxExpoTimeUs;
        }
    }

    // be sure initial exposure time is in the range
    if(config.exposureTimeUs < aeParam.minExposureTimeUs)
    {
        config.exposureTimeUs = aeParam.minExposureTimeUs + 1;
    }
    else if(config.exposureTimeUs > aeParam.maxExposureTimeUs)
    {
        config.exposureTimeUs = aeParam.maxExposureTimeUs - 1;
    }

    int aeLoopCount = 0;

    int captureHeight = ConoscopeAppWorker::mSettings.AEMeasAreaHeight;
    int captureWidth  = ConoscopeAppWorker::mSettings.AEMeasAreaWidth;
    int captureX      = ConoscopeAppWorker::mSettings.AEMeasAreaX;
    int captureY      = ConoscopeAppWorker::mSettings.AEMeasAreaY;

    while((ConoscopeAppHelper::mCancelRequest == false) &&
          (bLocked == false) &&
          (eError == ClassCommon::Error::Ok) &&
          (aeLoopCount < MAX_AE_LOOP_COUNT))
    {
        aeLoopCount ++;

        // this information may not the necessary.
        // This is mainly for debug and monitoring purpose at application level.
        // moreover it is relevant only for MeasureAE
        ConoscopeAppWorker::mMeasureStatus.exposureTimeUs = config.exposureTimeUs;
        ConoscopeAppWorker::mMeasureStatus.nbAcquisition = config.nbAcquisition;

        ImageConfiguration* imageConfiguration = ImageConfiguration::Get();

        if((captureHeight + captureY > imageConfiguration->image_height) ||
           (captureWidth + captureX > imageConfiguration->image_width))
        {
            LogInFile(QString(" Capture | [AutoExp] Meas Area disable because invalid %1x%2 @ %3,%5")
                      .arg(captureWidth).arg(captureHeight).arg(captureX).arg(captureY));
            LogInApp(QString(" Capture     [AutoExp] Meas Area disable because invalid %1x%2 @ %3,%5")
                     .arg(captureWidth).arg(captureHeight).arg(captureX).arg(captureY));

            // disable the measure area
            captureHeight = 0;
            captureWidth  = 0;
            captureX      = 0;
            captureY      = 0;
        }

        // apply alignment
        captureHeight        = ALIGN_VALUE(captureHeight,   4);
        captureWidth         = ALIGN_VALUE(captureWidth,   16);
        captureX             = ALIGN_VALUE(captureX,       16);
        captureY             = ALIGN_VALUE(captureY,        2);

        config.cropArea.setX(captureX);
        config.cropArea.setY(captureY);
        config.cropArea.setWidth(captureWidth);
        config.cropArea.setHeight(captureHeight);

        config.AeEnable = true;
        config.AeExpoTimeGranularityUs = ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs;

        config.bAeEnable = false;
        config.AEMeasAreaHeight = captureHeight;
        config.AEMeasAreaWidth  = captureWidth;
        config.AEMeasAreaX      = captureX;
        config.AEMeasAreaY      = captureY;

#ifdef EXPOSURE_TIME_GRANULARITY_FOR_AE
        config.exposureTimeUs = _GetExposureTime(config.exposureTimeUs, ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs);
#endif

        int captureExposureTime = config.exposureTimeUs;

#ifdef EXPOSURE_TIME_GRANULARITY_FOR_AE
        // store exposure time value before modification
        int ExposureTimeStore = config.exposureTimeUs;
#endif

        // capture data
        eError = ConoscopeAppProcess::CmdMeasure(config);

        if(eError != ClassCommon::Error::Ok)
        {
            LogInFile(QString("Capture | [AutoExp] CmdMeasure ERROR"));
            LogInApp(QString("Capture     [AutoExp] CmdMeasure ERROR"));
        }

        if((mCancelRequest == false) &&
           (eError == ClassCommon::Error::Ok))
        {
            // LogInFile(QString("  [AutoExp] CmdExportProcessed"));
            // LogInApp(QString("  [AutoExp] CmdExportProcessed"));

            // retrieve the camera processed data
            processingConfig.bBiasCompensation = false;
            processingConfig.bSensorDefectCorrection = true;
            processingConfig.bSensorPrnuCorrection = false;
            processingConfig.bLinearisation = false;
            processingConfig.bFlatField = false;
            processingConfig.bAbsolute = false;

            eError = ConoscopeAppProcess::CmdExportProcessed(processingConfig, mBuffer);

            height = ConoscopeAppProcess::cmdExportProcessedOutput.height;
            width = ConoscopeAppProcess::cmdExportProcessedOutput.width;
            maxValue = ConoscopeAppProcess::cmdExportProcessedOutput.max;
        }

        if((mCancelRequest == false) &&
           (eError == ClassCommon::Error::Ok))
        {
#ifndef APP_HELPER
            bLocked = _ProcessAutoExposure(aeParam, maxValue, config.exposureTimeUs);
#else
            bLocked = ConoscopeAppHelper::_ProcessAutoExposure(aeParam, maxValue, config.exposureTimeUs);
#endif

#ifdef EXPOSURE_TIME_GRANULARITY_FOR_AE
            // check whether the new exposure time is within the range
            // [exposureTime - (granularity / 2), exposureTime + (granularity / 2)]
            if((bLocked == false) && (ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs >= 1))
            {
#ifdef REMOVED
                int exposureTimeRange = 0;

                if(config.exposureTimeUs > ExposureTimeStore)
                {
                    exposureTimeRange = config.exposureTimeUs - ExposureTimeStore;
                }
                else
                {
                    exposureTimeRange = ExposureTimeStore - config.exposureTimeUs;
                }

                // config.exposureTimeUs = ExposureTimeStore;

                // bLocked = true;

                if(exposureTimeRange < ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs)
                {
                    // this is locked
                    bLocked = true;
                }
#else

                int ExposureTime = _GetExposureTime(config.exposureTimeUs, ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs);

                if(ExposureTime == ExposureTimeStore)
                {
                    // this is locked
                    bLocked = true;
                }
#endif
            }
#endif

            QString statusString;

            if(bLocked == false)
            {
                statusString = QString("Capture | UNLOCKED -> next expTime = %1 us").arg(config.exposureTimeUs, 6);
            }
            else
            {
                statusString = QString("Capture | LOCKED");
            }

            LogInFile(QString("Capture | [AutoExp] CmdMeasure expTime = %1 us (%2) pixelMax = %3 %4")
                      .arg(captureExposureTime, 6)
                      .arg(config.nbAcquisition)
                      .arg(maxValue, 5)
                      .arg(statusString));

            LogInApp(QString("Capture     [AutoExp] CmdMeasure expTime = %1 us (%2) pixelMax = %3 %4")
                     .arg(captureExposureTime, 6)
                     .arg(config.nbAcquisition)
                     .arg(maxValue, 5)
                     .arg(statusString));
        }

        // set back the camera with the right configuration
        config.cropArea.setX(0);
        config.cropArea.setY(0);
        config.cropArea.setWidth(0);
        config.cropArea.setHeight(0);
    }

    if(bLocked == false)
    {
        LogInApp(QString("Capture     [AutoExp] ERROR not locked"));
        LogInFile(QString("Capture     [AutoExp] ERROR not locked"));

        //eError = ClassCommon::Error::Failed;

#ifdef EXPOSURE_TIME_GRANULARITY_FOR_AE
        if(eError == ClassCommon::Error::Ok)
        {
            LogInApp(QString("                      BUT KEEP GOING"));
            bLocked = true;
        }
#endif
    }

#ifdef EXPOSURE_TIME_GRANULARITY_FOR_AE
    if(bLocked == false)
    {
    }
#endif

    else
    {
        // other configuration might be good (i.e. exposure time)

        // apply the granularity to the exposure time
        if(ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs > 1)
        {
            config.exposureTimeUs = _GetExposureTime(config.exposureTimeUs,
                                                     ConoscopeAppWorker::mSettings.AEExpoTimeGranularityUs,
                                                     aeParam.minExposureTimeUs,
                                                     aeParam.maxExposureTimeUs);

            LogInApp(QString("Capture     [AutoExp] CmdMeasure expTime = %1 us (%2)")
                     .arg(config.exposureTimeUs, 6)
                     .arg(config.nbAcquisition));

            LogInFile(QString("Capture     [AutoExp] CmdMeasure expTime = %1 us (%2)")
                     .arg(config.exposureTimeUs, 6)
                     .arg(config.nbAcquisition));
        }

        config.bAeEnable = true;
        config.AEMeasAreaHeight = captureHeight;
        config.AEMeasAreaWidth  = captureWidth;
        config.AEMeasAreaX      = captureX;
        config.AEMeasAreaY      = captureY;

        // capture data
        eError = ConoscopeAppProcess::CmdMeasure(config);
    }

    // set back: save processing configuration
    behavior.saveParamOnCmd = true;
    behavior.updateCaptureDate = true;
    ConoscopeAppProcess::SetBehaviorConfig(behavior);

    return eError;
}

bool ConoscopeAppHelper::_ProcessAutoExposure(AutoExposureParam_t& param, int intMaximum, int& IntegrationTime)
{
    // ouput
    bool bLocked = false;

    float sngExposureTime;
    float sngMult;

    if((IntegrationTime <= param.minExposureTimeUs) || (IntegrationTime >= param.maxExposureTimeUs))
    {
        bLocked = true;
    }
    else
    {
        if((intMaximum > param.thresholdDown) && (intMaximum < param.thresholdUp))
        {
            bLocked = true;
        }
        else
        {
            sngExposureTime = IntegrationTime;

            // if we are too high
            if(intMaximum > param.saturation)
            {
                sngExposureTime = sngExposureTime / param.decreasingFactor;
            }
            else
            {
                // if we are not in the noise
                // ---- Increase ExposureTime
                if(intMaximum < (param.sngNoise * param.noiseLevelRatio))
                {
                    sngExposureTime = sngExposureTime * (param.increasingFactor);
                }
                else
                {
                    // ---- Extrapolate ExposureTime
                    sngMult = (intMaximum - param.sngNoise) / sngExposureTime;
                    sngExposureTime = (param.targetMax - param.sngNoise) / sngMult;
                }
            }

            // output must be inside range
            if(sngExposureTime < param.minExposureTimeUs)
            {
                IntegrationTime = param.minExposureTimeUs;
            }
            else if(sngExposureTime > param.maxExposureTimeUs)
            {
                IntegrationTime = param.maxExposureTimeUs;
            }
            else
            {
                IntegrationTime = sngExposureTime;
            }
        }
    }
    return bLocked;
}

void ConoscopeAppHelper::LogInFile(QString message)
{
    RESOURCE->AppendLog(QString("%1 | ").arg(mLogHeader, -20), message);
}

void ConoscopeAppHelper::LogInApp(QString message)
{
    RESOURCE->Log(ToolsString::FormatText(mLogAppHeader, message));
}

#define SETUP_STATUS_WAIT_MS 1000
#define SETUP_STATUS_TIMEOUT_MS 120000

#define SAVE_PROCESSED_DATA true

ClassCommon::Error ConoscopeAppHelper::_WaitForSetup()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile(QString("  _WaitForSetup"));

    SetupStatus_t setupConfigStatus;

    bool bLocked = false;
    int timeout = 0;

    do {
        eError = ConoscopeAppProcess::CmdSetupStatus(setupConfigStatus);

        switch(setupConfigStatus.eTemperatureMonitoringState)
        {
            case TemperatureMonitoringState_Processing:
                break;

            case TemperatureMonitoringState_Locked:
                bLocked = true;
                LogInFile("  _WaitForSetup Locked");
                LogInApp("  _WaitForSetup Locked");
                break;

            case TemperatureMonitoringState_NotStarted:
            case TemperatureMonitoringState_Aborted:
            case TemperatureMonitoringState_Error:
            default:
                LogInFile("  _WaitForSetup Error");
                LogInApp("  _WaitForSetup Error");

                eError = ClassCommon::Error::Failed;
                break;
        }

        if(eError == ClassCommon::Error::Ok)
        {
            QThread::msleep(SETUP_STATUS_WAIT_MS);
            timeout += SETUP_STATUS_WAIT_MS;

            if(timeout > SETUP_STATUS_TIMEOUT_MS)
            {
                LogInFile("  _WaitForSetup Timeout");
                LogInApp("  _WaitForSetup Timeout");

                eError = ClassCommon::Error::Timeout;
            }
        }

    } while((bLocked == false) && (eError == ClassCommon::Error::Ok));

    return eError;
}

