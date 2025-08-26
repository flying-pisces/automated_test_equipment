#include <QVector>
#include <QMap>
#include <QMetaEnum>

#include "appControllerWorker.h"
#include "appResource.h"

#define LOG_CW(x)    if(RESOURCES->IsMaskEnable(LogMask_Worker) == true) Log("              Worker", x)
#define LOG_ERROR(x) if(RESOURCES->IsMaskEnable(LogMask_Error) == true)  Log("              Worker", x)

#define PICOAMMETER_INTERVAL 10

AppControllerWorker::AppControllerWorker(QObject *parent) : ClassCommon(parent)
{
}

AppControllerWorker::~AppControllerWorker()
{
}

void AppControllerWorker::OnWorkRequest(int value)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    Request eRequest = static_cast<Request>(value);

    bCancelCommand = false;

    switch(eRequest)
    {
    case Request::CmdOpen:
        eError = _CmdOpen();
        break;

    case Request::CmdSetup:
        eError = _CmdSetup();
        break;

    case Request::CmdMeasure:
        eError = _CmdMeasure();
        break;

    case Request::CmdMeasureAE:
        eError = _CmdMeasureAE();
        break;

    case Request::CmdMeasureAECancel:
        eError = _CmdMeasureAECancel();
        break;

    case Request::CmdExportRaw:
        // eError = _CmdExportRaw();
        _CmdExportRaw();
        break;

    case Request::CmdExportProcessed:
        // eError = _CmdExportProcessed();
        _CmdExportProcessed();
        break;

    case Request::CmdClose:
        eError = _CmdClose();
        break;

    case Request::CmdReset:
        eError = _CmdReset();
        break;

    case Request::CmdCfgFileWrite:
        eError = _CmdCfgFileWrite();
        break;

    case Request::CmdCfgFileRead:
        eError = _CmdCfgFileRead();
        break;

    case Request::CmdStream:
        eError = _CmdStream();
        break;

    case Request::CmdExportRawBuffer:
        eError = _CmdExportRawBuffer();
        break;

    case Request::CmdExportProcessedBuffer:
        eError = _CmdExportProcessedBuffer();
        break;

    case Request::CmdCaptureSequence:
        eError = _CmdCaptureSequence();
        break;

    case Request::CmdCaptureSequenceCancel:
        eError = _CmdCaptureSequenceCancel();
        break;

    case Request::EventCaptureSequenceDone:
        break;

    case Request::EventCaptureSequenceCancel:
        break;

    case Request::CmdConvertRaw:
        eError = _CmdConvertRaw();
        break;

    default:
        break;
    }

    LOG_CW(QString("done     request [%1] %2%3")
        .arg(EnumToString("Request", (int)eRequest))
        .arg((eError == ClassCommon::Error::Ok) ? "" : "ERROR : ")
        .arg(ClassCommon::ErrorToString(eError)));

    emit WorkDone(value, (int)eError);
}

void AppControllerWorker::CancelCmd()
{
    // some commands (streaming) are loops that required to be stopped
    bCancelCommand = true;
}

ClassCommon::Error AppControllerWorker::_CmdOpen()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdOpen();
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdSetup()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdSetup(params.cmdSetupConfig);
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdMeasure()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdMeasure(params.cmdMeasureConfig);
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdMeasureAE()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdMeasureAE(params.cmdMeasureConfig);
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdMeasureAECancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdMeasureAECancel();
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdExportRaw()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdExportRaw();
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdExportProcessed()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    ProcessingResult_t result;

    eError = RESOURCES->mConoscopeApi->CmdExportProcessed(params.cmdProcessingConfig, result);

    if(result.saturationFlag == true)
    {
        SendWarning(QString("Capture is saturated\n(level is %1)").arg(result.SaturationLevel));
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdClose()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdClose();
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdReset()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdReset();
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdCfgFileWrite()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdCfgFileWrite();
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdCfgFileRead()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = RESOURCES->mConoscopeApi->CmdCfgFileRead();
    return eError;
}

#define SATURATION_VALUE 4090
#define MIN_EXPOSURE_TIME_US 10
#define MAX_EXPOSURE_TIME_US 950000

ClassCommon::Error AppControllerWorker::_CmdStream()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    int width;
    int height;

    AutoExposureParam_t aeParam;

    aeParam.targetMax = params.applicationConfig.autoExposurePixelMax * SATURATION_VALUE;
    aeParam.targetMax = aeParam.targetMax / 100;

    float conAETargetMargin = 0.02F;
    aeParam.thresholdUp   = aeParam.targetMax * (1 + conAETargetMargin);
    aeParam.thresholdDown = aeParam.targetMax * (1 - conAETargetMargin);

    aeParam.saturation = SATURATION_VALUE;

    aeParam.decreasingFactor = 2;
    aeParam.increasingFactor = 5;
    aeParam.noiseLevelRatio  = 3;

    aeParam.sngNoise = 500;

    // configure the conoscope not to save export configuration
    ConoscopeBehavior_t behaviorConfig;
    behaviorConfig.saveParamOnCmd = false;
    behaviorConfig.updateCaptureDate = true;
    eError = RESOURCES->mConoscopeApi->CmdSetBehaviorConfig(behaviorConfig);

    ProcessingConfig_t config;

    if(params.applicationConfig.bStreamProcessedData == true)
    {
        RESOURCES->GetDisplayStreamConfig(config);
    }

#define ROI_ON_DISPLAY
#ifdef ROI_ON_DISPLAY
    ConoscopeSettings_t settings;
    RESOURCES->mConoscopeApi->CmdGetConfig(settings);
#endif

    while((bCancelCommand == false) && (eError == ClassCommon::Error::Ok))
    {
        // capture data
        eError = RESOURCES->mConoscopeApi->CmdMeasure(params.cmdMeasureConfig, false);

        if((bCancelCommand == false) && (eError == ClassCommon::Error::Ok))
        {
            if(params.applicationConfig.bStreamProcessedData == false)
            {
                // retrieve the raw data buffer
                eError = RESOURCES->mConoscopeApi->CmdExportRawBuffer(mBuffer, false);
            }
            else
            {
                // retrieve the camera processed data
                eError = RESOURCES->mConoscopeApi->CmdExportProcessedBuffer(config, mBuffer, false);
            }
        }

        // retrieve configuration of the image buffer
        height = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_HEIGHT).toInt();
        width  = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_WIDTH).toInt();

        if((bCancelCommand == false) && (eError == ClassCommon::Error::Ok))
        {
#ifdef ROI_ON_DISPLAY
            if(settings.bUseRoi == true)
            {
                _CropBuffer(settings.RoiXRight, settings.RoiXLeft, settings.RoiYBottom, settings.RoiYTop,
                            mBuffer,
                            height, width);
            }
#endif

            int whiteLevel = 4095;
            pFrameBuffer->Fill(&mBuffer, whiteLevel);

            QVariant maxValueVar = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_MAX);
            int maxValue = maxValueVar.toInt();

            pFrameBuffer->mMessage = QString("PixelMax = %1 ExposureTime = %2")
                    .arg(maxValue)
                    .arg(params.cmdMeasureConfig.exposureTimeUs);

            if(params.applicationConfig.autoExposure == true)
            {
                bool locked = _ProcessAutoExposure(aeParam, maxValue, params.cmdMeasureConfig.exposureTimeUs);

                QString lockMessage = "";

                if(params.cmdMeasureConfig.exposureTimeUs < MIN_EXPOSURE_TIME_US)
                {
                    lockMessage = " (MIN)";

                    params.cmdMeasureConfig.exposureTimeUs = MIN_EXPOSURE_TIME_US;
                    locked = true;
                }
                else if(params.cmdMeasureConfig.exposureTimeUs > MAX_EXPOSURE_TIME_US)
                {
                    lockMessage = " (MAX)";

                    params.cmdMeasureConfig.exposureTimeUs = MAX_EXPOSURE_TIME_US;
                    locked = true;
                }

                pFrameBuffer->mMessage += QString(" locked = %1%2").arg(locked == true ? "true" : "false")
                        .arg(lockMessage);

                if(locked == true)
                {
                    emit PleaseUpdateExposureTime(params.cmdMeasureConfig.exposureTimeUs);
                }
            }

            // indicate a buffer is ready
            emit RawBufferReady(width, height, whiteLevel);
        }
    }

    // set back to default configuration
    behaviorConfig.saveParamOnCmd = true;
    behaviorConfig.updateCaptureDate = true;
    eError = RESOURCES->mConoscopeApi->CmdSetBehaviorConfig(behaviorConfig);

    return eError;
}

bool AppControllerWorker::_ProcessAutoExposure(AutoExposureParam_t& param, int intMaximum, int& IntegrationTime)
{
    // ouput
    bool bLocked = false;

    float sngExposureTime;
    float sngMult;

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
            // '---- Extrapolate ExposureTime
            sngMult = (intMaximum - param.sngNoise) / sngExposureTime;
            sngExposureTime = (param.targetMax - param.sngNoise) / sngMult;
          }
        }

        // output
        IntegrationTime = sngExposureTime;
    }

    return bLocked;
}

ClassCommon::Error AppControllerWorker::_CmdExportRawBuffer()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdExportRawBuffer(mBuffer, true);
    if(eError == ClassCommon::Error::Ok)
    {
        // retrieve configuration of the image buffer
        int height = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_HEIGHT).toInt();
        int width  = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_WIDTH).toInt();

#ifdef ROI_ON_DISPLAY
        ConoscopeSettings_t settings;
        RESOURCES->mConoscopeApi->CmdGetConfig(settings);

        if(settings.bUseRoi == true)
        {
            _CropBuffer(settings.RoiXRight, settings.RoiXLeft, settings.RoiYBottom, settings.RoiYTop,
                        mBuffer,
                        height, width);
        }
#endif

        int whiteLevel = 4095;
        pFrameBuffer->Fill(&mBuffer, whiteLevel);

        QVariant maxValue = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_MAX);
        QVariant exposureTime = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_EXPOSURE_TIME_US);
        pFrameBuffer->mMessage = QString("PixelMax = %1 ExposureTime = %2")
                .arg(maxValue.toInt())
                .arg(exposureTime.toInt());


        // retrieve AE configuration
        int AEMeasAreaWidth   = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_AE_ROI_WIDTH).toInt();
        int AEMeasAreaHeight  = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_AE_ROI_HEIGHT).toInt();
        int AEMeasAreaX       = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_AE_ROI_X).toInt();
        int AEMeasAreaY       = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_AE_ROI_Y).toInt();

        // indicate a buffer is ready
        emit RawBufferAeRoi(AEMeasAreaWidth,
                            AEMeasAreaHeight,
                            AEMeasAreaX,
                            AEMeasAreaY);

        emit RawBufferReady(width, height, whiteLevel);
    }
    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdExportProcessedBuffer()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdExportProcessedBuffer(params.cmdProcessingConfig, mBuffer, true);

    if(eError == ClassCommon::Error::Ok)
    {
        // retrieve configuration of the image buffer
        int height = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_HEIGHT).toInt();
        int width  = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_WIDTH).toInt();

#ifdef ROI_ON_DISPLAY
        ConoscopeSettings_t settings;
        RESOURCES->mConoscopeApi->CmdGetConfig(settings);

        if(settings.bUseRoi == true)
        {
            _CropBuffer(settings.RoiXRight, settings.RoiXLeft, settings.RoiYBottom, settings.RoiYTop,
                        mBuffer,
                        height, width);
        }
#endif

        int whiteLevel = 4095;
        pFrameBuffer->Fill(&mBuffer, whiteLevel);

        QVariant maxValue = RESOURCES->mConoscopeApi->GetOption(RETURN_ITEM_MAX);
        pFrameBuffer->mMessage = QString("PixelMax = %1 ExposureTime = %2")
                .arg(maxValue.toInt())
                .arg(params.cmdMeasureConfig.exposureTimeUs);

        // indicate a buffer is ready
        emit RawBufferReady(width, height, whiteLevel);
    }
    return eError;
}

#ifdef ROI_ON_DISPLAY
void AppControllerWorker::_CropBuffer(int RoiXRight, int RoiXLeft, int RoiYBottom, int RoiYTop,
                                      std::vector<int16_t>& buffer, int& height, int& width)
{
    // resize mBuffer according to ROI
    int cropOffsetX = RoiXLeft;
    int cropOffsetY = RoiYTop;
    int cropWidth   = RoiXRight - RoiXLeft;
    int cropHeight  = RoiYBottom - RoiYTop;

    // crop the data to store
    int bufferCropSize = cropHeight * cropWidth * sizeof(int16_t);
    std::vector<int16_t> bufferCrop;
    bufferCrop.resize(bufferCropSize);

#pragma omp parallel for num_threads(4)
    for(int lineIndex = 0; lineIndex < cropHeight; lineIndex ++)
    {
        memcpy_s(&bufferCrop[lineIndex * cropWidth], cropWidth * sizeof(int16_t),
                &buffer[(cropOffsetY + lineIndex) * width + cropOffsetX], cropWidth * sizeof(int16_t));
    }

    memcpy_s(buffer.data(), buffer.size(), bufferCrop.data(), bufferCropSize);

    // update height and width
    height = cropHeight;
    width = cropWidth;
}
#endif

ClassCommon::Error AppControllerWorker::_CmdCaptureSequence()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdCaptureSequence(params.cmdCaptureSequence);

    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdCaptureSequenceCancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdCaptureSequenceCancel();

    return eError;
}

ClassCommon::Error AppControllerWorker::_CmdConvertRaw()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdConvertRaw(params.cmdConvertRaw);

    return eError;
}

bool AppControllerWorker::SendWarning(QString message)
{
    emit WarningMessage(message);

    return true;
}

