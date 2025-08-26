#include "ConoscopeAppWorker.h"
#include "ConoscopeAppProcess.h"
#include "ConoscopeResource.h"

#include "ConoscopeProcess.h"

#include "imageConfiguration.h"

#ifdef APP_HELPER
#include "ConoscopeAppHelper.h"
#endif

#include <QJsonObject>
#include <QJsonDocument>

#include <QDir>

#ifdef MULTITHREAD_CAPTURE_SEQUENCE
#include "CaptureSequenceThread.h"
#include <QDebug>
#endif

#include "toolString.h"

#define LOG_CW(x) Log("              Worker", x)

#define FILE_NAME "%1_nd_%2_iris_%3"
#define FLOAT_FILE_NAME "%1_%2%3_float"

#define CAPTURE_EXTENSION "bin"

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

MeasureConfig_t ConoscopeAppWorker::mMeasureConfig;
MeasureStatus_t ConoscopeAppWorker::mMeasureStatus;

CaptureSequenceConfig_t ConoscopeAppWorker::mCaptureSequenceConfig;
CaptureSequenceStatus_t ConoscopeAppWorker::mCaptureSequenceStatus;
ProcessingConfig_t      ConoscopeAppWorker::mCaptureSequenceExportConfig;

ConoscopeDebugSettings_t ConoscopeAppWorker::mDebugSettings;
ConoscopeSettings_t      ConoscopeAppWorker::mSettings;

ConoscopeAppWorker::ConoscopeAppWorker(QObject *parent) : ConoscopeAppHelper(parent)
{
    ConoscopeAppWorker::mCaptureSequenceStatus.nbSteps = 0;
    ConoscopeAppWorker::mCaptureSequenceStatus.currentSteps = 0;
    ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_NotStarted;
    ConoscopeAppWorker::mCaptureSequenceStatus.eFilter = Filter_Invalid;

    ConoscopeAppWorker::mMeasureStatus.state = MeasureStatus_t::State_t::State_NotStarted;

    mLogHeader    = QString("[conoscopeAppWorker]");
    mLogAppHeader = QString("[APP]");
}

ConoscopeAppWorker::~ConoscopeAppWorker()
{
}

ClassCommon::Error ConoscopeAppWorker::CaptureSequenceCancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if((mCaptureSequenceStatus.state == CaptureSequenceStatus_t::State_t::State_Done) ||
       (mCaptureSequenceStatus.state == CaptureSequenceStatus_t::State_t::State_Error) ||
       (mCaptureSequenceStatus.state == CaptureSequenceStatus_t::State_t::State_Cancel) ||
       (mCaptureSequenceStatus.state == CaptureSequenceStatus_t::State_t::State_NotStarted))
    {
        eError = ClassCommon::Error::InvalidState;
    }

    mCancelRequest = true;

#ifdef MULTITHREAD_CAPTURE_SEQUENCE
    CaptureSequenceThread::Cancel();
#endif

    return eError;
}

ClassCommon::Error ConoscopeAppWorker::MeasureAECancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if((mMeasureStatus.state == MeasureStatus_t::State_t::State_Done) ||
       (mMeasureStatus.state == MeasureStatus_t::State_t::State_Error) ||
       (mMeasureStatus.state == MeasureStatus_t::State_t::State_Cancel) ||
       (mMeasureStatus.state == MeasureStatus_t::State_t::State_NotStarted))
    {
        eError = ClassCommon::Error::InvalidState;
    }

    mCancelRequest = true;

    return eError;
}

void ConoscopeAppWorker::OnWorkRequest(int value)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    Request eRequest = static_cast<Request>(value);

    switch(eRequest)
    {
    case Request::CmdCaptureSequence:
        if(ConoscopeAppWorker::mDebugSettings.emulateCamera == false)
        {
            eError = _CmdCapturingSequence();
        }
        else
        {
            eError = _CmdCapturingSequenceEmulate();
        }
        break;

    case Request::CmdMeasureAE:
        eError = _CmdMeasureAE();
        break;

    default:
        break;
    }

    LOG_CW(QString("done     request [%1] %2")
        .arg(EnumToString("Request", (int)eRequest))
        .arg((eError == ClassCommon::Error::Ok) ? "Ok" : "Failed"));

    emit WorkDone(value, (int)eError);
}

void ConoscopeAppWorker::_CapturingSequenceFileName(CaptureSequenceConfig_t& config, SomeInfo_t& info, QString& fileName, QString& appendPart)
{
#define FILE_NAME_FORMAT
#ifdef FILE_NAME_FORMAT
        QMap<FileFormatKey_t, QString> fileNameFormatParams;

        fileNameFormatParams[FileFormatKey_t::TimeStamp] = info.timeStampString;
        //fileNameFormatParams[FileFormatKey_t::Filter]    = RESOURCE->ToString(_setupConfig.eFilter);
        fileNameFormatParams[FileFormatKey_t::Nd]        = RESOURCE->ToString(config.eNd);
        fileNameFormatParams[FileFormatKey_t::Iris]      = RESOURCE->ToString(config.eIris, true);
        //fileNameFormatParams[FileFormatKey_t::ExpoTime]  = QString("%1").arg(mInfo.exposureTimeUs);
        //fileNameFormatParams[FileFormatKey_t::NbAcq]     = QString("%1").arg(mInfo.nbAcquisition);
        fileNameFormatParams[FileFormatKey_t::Height]    = QString("%1").arg(ConoscopeAppProcess::cmdExportProcessedOutput.height);
        fileNameFormatParams[FileFormatKey_t::Width]     = QString("%1").arg(ConoscopeAppProcess::cmdExportProcessedOutput.width);
        //fileNameFormatParams[FileFormatKey_t::SatLevel]  = "";
        //fileNameFormatParams[FileFormatKey_t::SatFlag]   = "";

        if(ConoscopeAppWorker::mSettings.bUseRoi == true)
        {
            // crop image if required
            int cropHeight = ConoscopeProcess::mSettings.RoiYBottom - ConoscopeProcess::mSettings.RoiYTop;
            int cropWidth  = ConoscopeProcess::mSettings.RoiXRight - ConoscopeProcess::mSettings.RoiXLeft;

            fileNameFormatParams[FileFormatKey_t::Height]    = QString("%1").arg(cropHeight);
            fileNameFormatParams[FileFormatKey_t::Width]     = QString("%1").arg(cropWidth);
        }

        if(config.bAutoExposure == true)
        {
            fileNameFormatParams[FileFormatKey_t::AeExpoGran] = QString("%1").arg(ConoscopeProcess::mSettings.AEExpoTimeGranularityUs);
        }

        fileName = ConoscopeProcess::FormatFileName(fileNameFormatParams);

        if(fileName == "")
        {
            fileName = QString(FILE_NAME).arg(info.timeStampString)
                                         .arg(RESOURCE->ToString(config.eNd))
                                         .arg(RESOURCE->ToString(config.eIris, true));
        }
        else
        {
            ConoscopeProcess::_CleanFileName(fileName);
        }

#else

        QString fileName = QString(FILE_NAME).arg(info.timeStampString)
                                             .arg(RESOURCE->ToString(config.eNd))
                                             .arg(RESOURCE->ToString(config.eIris, true));
#endif

#define COMPOSE_FILE_NAME
#ifdef COMPOSE_FILE_NAME
        // add prepend part
        // prepend and append part

        fileName = QString("%1%2").arg(CONVERT_TO_QSTRING(ConoscopeAppWorker::mSettings.fileNamePrepend))
                                         .arg(fileName);
#endif

        fileName = QDir::cleanPath(info.capturePath + QDir::separator() + fileName);

        appendPart = CONVERT_TO_QSTRING(ConoscopeAppWorker::mSettings.fileNameAppend);

        if(appendPart != "")
        {
            appendPart.append("_");
        }
}

ClassCommon::Error ConoscopeAppWorker::_CmdCapturingSequence()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mCancelRequest = false;

    CaptureSequenceConfig_t config = ConoscopeAppWorker::mCaptureSequenceConfig;
    CaptureSequenceOption_t captureSequenceOption;

    // save configuration
    ConoscopeAppProcess::SetConfig(config);

    ConoscopeBehavior_t behaviorConfig;

#ifndef CAPTURE_SEQUENCE_ORDER
    QList<Filter_t> filterList = {Filter_X,
                                  Filter_Xz,
                                  Filter_Ya,
                                  Filter_Yb,
                                  Filter_Z};
#else
    // set the filters in the order of the filters
    QList<Filter_t> filterList = {Filter_Z,
                                  Filter_Xz,
                                  Filter_Yb,
                                  Filter_Ya,
                                  Filter_X};
#endif

    ConoscopeAppWorker::mCaptureSequenceStatus.nbSteps = filterList.count();

    QMap<Filter_t, CaptureSequenceBuffer_t> bufferList = {
        {Filter_X,  {&mBuffer1, 0, 0, 0}},
        {Filter_Xz, {&mBuffer2, 0, 0, 0}},
        {Filter_Ya, {&mBuffer3, 0, 0, 0}},
        {Filter_Yb, {&mBuffer4, 0, 0, 0}},
        {Filter_Z,  {&mBuffer5, 0, 0, 0}}
    };

#ifndef MULTITHREAD_CAPTURE_SEQUENCE
    int index = 0;
#endif

    // set behavior configuration so time stamp is not updated
    behaviorConfig.saveParamOnCmd = false;
    behaviorConfig.updateCaptureDate = true;
    ConoscopeAppProcess::SetBehaviorConfig(behaviorConfig);

    QMap<Filter_t, int> exposureTimeList = {
        {Filter_X,  ConoscopeAppWorker::mCaptureSequenceConfig.exposureTimeUs_FilterX},
        {Filter_Xz, ConoscopeAppWorker::mCaptureSequenceConfig.exposureTimeUs_FilterXz},
        {Filter_Ya, ConoscopeAppWorker::mCaptureSequenceConfig.exposureTimeUs_FilterYa},
        {Filter_Yb, ConoscopeAppWorker::mCaptureSequenceConfig.exposureTimeUs_FilterYb},
        {Filter_Z,  ConoscopeAppWorker::mCaptureSequenceConfig.exposureTimeUs_FilterZ}
    };

    // retrieve exposure time list
    if(config.bAutoExposure == false)
    {
        if(config.bUseExpoFile == true)
        {
            _ReadExposureTimeFile(exposureTimeList);
        }
    }

    // fill config
    config.exposureTimeUs_FilterX  = exposureTimeList[Filter_X];
    config.exposureTimeUs_FilterXz = exposureTimeList[Filter_Xz];
    config.exposureTimeUs_FilterYa = exposureTimeList[Filter_Ya];
    config.exposureTimeUs_FilterYb = exposureTimeList[Filter_Yb];
    config.exposureTimeUs_FilterZ  = exposureTimeList[Filter_Z];

    // read the export configuration (if any file present)
    _ReadExposureExportOption(mCaptureSequenceExportConfig, captureSequenceOption);

#ifdef MULTITHREAD_CAPTURE_SEQUENCE
    // create the thread instances
    CaptureSequenceThread thread1(this);
    CaptureSequenceThread thread2(this);

    CaptureSequenceThread::Initialise(&filterList, &exposureTimeList, &bufferList);

    thread1.start();
    // add a delay to be sure the thread 1 is started when thread 2 is started
    QThread::msleep(20);
    thread2.start();

    thread1.wait();
    thread2.wait();

    ClassCommon::Error threadError = ClassCommon::Error::Ok;
    int threadIndex;

    thread1.GetError(threadError, threadIndex);

    if(threadError != ClassCommon::Error::Ok)
    {
        LogInFile(QString(" Capture | CaptureSequence ERROR in thread [%1]").arg(threadIndex));
        LogInApp(QString(" Capture | CaptureSequence ERROR in thread [%1]").arg(threadIndex));

        eError = ClassCommon::Error::Timeout;
    }

    thread2.GetError(threadError, threadIndex);

    if(threadError != ClassCommon::Error::Ok)
    {
        LogInFile(QString(" Capture | CaptureSequence ERROR in thread [%1]").arg(threadIndex));
        LogInApp(QString(" Capture | CaptureSequence ERROR in thread [%1]").arg(threadIndex));

        eError = ClassCommon::Error::Timeout;
    }

#else
    while((index < filterList.count()) &&
          (eError == ClassCommon::Error::Ok) &&
          (ConoscopeAppWorker::mCancelRequest == false))
    {
        ConoscopeAppWorker::mCaptureSequenceStatus.currentSteps = index + 1;
        ConoscopeAppWorker::mCaptureSequenceStatus.eFilter = filterList[index];

        eError = _Capturing(filterList[index], config, bufferList[filterList[index]]);

        behaviorConfig.saveParamOnCmd = false;
        behaviorConfig.updateCaptureDate = false;
        ConoscopeAppProcess::SetBehaviorConfig(behaviorConfig);

        index ++;
    }
#endif

    if((eError == ClassCommon::Error::Ok) &&
       (mCancelRequest == false))
    {
        if(captureSequenceOption.bGenerateXYZ == true)
        {
            SomeInfo_t info;
            ConoscopeAppProcess::GetSomeInfo(info);

            QString fileName;
            QString appendPart;

            _CapturingSequenceFileName(config, info, fileName, appendPart);

            _ComposeComponents(fileName, bufferList, appendPart);

            _WriteCaptureSequenceInfo(fileName, exposureTimeList, info);
        }

        // Send a message to indicate saturation happened
        CaptureSequenceResult_t sequenceThreadResult = CaptureSequenceThread::GetResult();
        if(sequenceThreadResult.bSaturatedCapture == true)
        {
            RESOURCE->SendWarning("CaptureSequence\nPlease check capture saturation");
        }
    }

    if(mCancelRequest == true)
    {
        ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Cancel;
        LogInApp(" Capture | CaptureSequence CANCELED");
    }
    else
    {
        if(eError == ClassCommon::Error::Ok)
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Done;
            LogInApp(" Capture | CaptureSequence DONE");
        }
        else
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Error;
            LogInApp(" Capture | CaptureSequence FAILED");
        }
    }

    // set behavior configuration back
    behaviorConfig.saveParamOnCmd = true;
    behaviorConfig.updateCaptureDate = true;
    ConoscopeAppProcess::SetBehaviorConfig(behaviorConfig);

    return eError;
}

ClassCommon::Error ConoscopeAppWorker::_CmdCapturingSequenceEmulate()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mCancelRequest = false;

    CaptureSequenceConfig_t config = ConoscopeAppWorker::mCaptureSequenceConfig;

    QList<Filter_t> filterList = {Filter_X,
                                  Filter_Xz,
                                  Filter_Ya,
                                  Filter_Yb,
                                  Filter_Z};

    ConoscopeAppWorker::mCaptureSequenceStatus.nbSteps = filterList.count();

    QMap<Filter_t, CaptureSequenceBuffer_t> bufferList = {
        {Filter_X,  {&mBuffer1, 0, 0, 0}},
        {Filter_Xz, {&mBuffer2, 0, 0, 0}},
        {Filter_Ya, {&mBuffer3, 0, 0, 0}},
        {Filter_Yb, {&mBuffer4, 0, 0, 0}},
        {Filter_Z,  {&mBuffer5, 0, 0, 0}}
    };

    int index = 0;

    QMap<Filter_t, QString> fileList;

    _ReadCapturesFile(fileList);

    QString fileName;
    QString capturePath;

    while((index < filterList.count()) &&
          (eError == ClassCommon::Error::Ok) &&
          (mCancelRequest == false))
    {
        ConoscopeAppWorker::mCaptureSequenceStatus.currentSteps = index + 1;
        ConoscopeAppWorker::mCaptureSequenceStatus.eFilter = filterList[index];

        // read file and associated meta data
        Filter_t eFilter = filterList[index];

        if(fileList.contains(eFilter))
        {
            fileName = fileList[eFilter];

            eError = _ReadCapture(fileName, bufferList[eFilter], config, capturePath);
        }
        else
        {
            eError = ClassCommon::Error::Failed;
        }

        index ++;
    }

    // crop the image if necessary (and if possible)
    if(eError == ClassCommon::Error::Ok)
    {
        int RoiXLeft   = ConoscopeProcess::mSettings.RoiXLeft;
        int RoiXRight  = ConoscopeProcess::mSettings.RoiXRight;
        int RoiYTop    = ConoscopeProcess::mSettings.RoiYTop;
        int RoiYBottom = ConoscopeProcess::mSettings.RoiYBottom;

        if((RoiXLeft   >= config.RoiXLeft) && (RoiXLeft   <= config.RoiXRight) &&
           (RoiXRight  >= config.RoiXLeft) && (RoiXRight  <= config.RoiXRight) &&
           (RoiYTop    >= config.RoiYTop)  && (RoiYTop    <= config.RoiYBottom) &&
           (RoiYBottom >= config.RoiYTop)  && (RoiYBottom <= config.RoiYBottom))
        {
            // this is a temporary buffer
            std::vector<int16_t> tmpBuffer;

            int _resizeWidth  = RoiXRight  - RoiXLeft;
            int _resizeHeight = RoiYBottom - RoiYTop;

            int index = 0;

            while(index < filterList.count())
            {
                Filter_t filterType = filterList[index];

                int bufferSize = bufferList[filterType].data->size();

                // copy into temporary buffer
                tmpBuffer.resize(bufferSize);
                memcpy(tmpBuffer.data(), bufferList[filterType].data->data(), bufferSize * sizeof(int16_t));

                // clean initial buffer
                bufferList[filterType].data->clear();
                bufferList[filterType].data->resize(_resizeWidth * _resizeHeight);

                memcpy(bufferList[filterType].data->data(), tmpBuffer.data(), bufferSize * sizeof(int16_t));

                int16_t* klibData  = (int16_t*) tmpBuffer.data();
                int16_t* pCropData = (int16_t*) bufferList[filterType].data->data();

                int cropOffsetY = ConoscopeProcess::mSettings.RoiYTop - config.RoiYTop;
                int cropOffsetX = ConoscopeProcess::mSettings.RoiXLeft - config.RoiXLeft;

                int cropWidth  = RoiXRight  - RoiXLeft;
                int cropHeight = RoiYBottom - RoiYTop;

                int _srcWidth  = config.RoiXRight  - config.RoiXLeft;

                for(int lineIndex = 0; lineIndex < cropHeight; lineIndex ++)
                {
                    for(int rowIndex = 0; rowIndex < cropWidth; rowIndex ++)
                    {
                        int destIndex = (lineIndex * cropWidth + rowIndex);
                        int srcIndex = (cropOffsetY + lineIndex) * _srcWidth + cropOffsetX + rowIndex;

                        pCropData[destIndex] = klibData[srcIndex];
                    }
                }

               index ++;
            }
        }
        else
        {
            eError = ClassCommon::Error::InvalidParameter;
        }
    }

    // in anycase disable the ROI because the images are already cropped
    ConoscopeProcess::mSettings.bUseRoi = false;
    ConoscopeAppWorker::mSettings.bUseRoi = false;

    if(eError == ClassCommon::Error::Ok)
    {
        SomeInfo_t info;

        QDateTime timeStamp = QDateTime::currentDateTime();
        info.timeStampString = timeStamp.toString("yyyyMMdd_hhmmss");
        info.capturePath = capturePath;

        QString fileName;
        QString appendPart;

        _CapturingSequenceFileName(config, info, fileName, appendPart);

        _ComposeComponents(fileName, bufferList, appendPart);
    }

    if(mCancelRequest == true)
    {
        ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Cancel;
        LogInApp("CaptureSequence CANCELED");
    }
    else
    {
        if(eError == ClassCommon::Error::Ok)
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Done;
            LogInApp("CaptureSequence DONE");
        }
        else
        {
            ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Error;
            LogInApp("CaptureSequence FAILED");
        }
    }

    return eError;
}

#ifndef MULTITHREAD_CAPTURE_SEQUENCE
ClassCommon::Error ConoscopeAppWorker::_Capturing(Filter_t eFilter, CaptureSequenceConfig_t& config, CaptureSequenceBuffer_t& buffer)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // setup the conoscope
    LogInFile(QString(" Capture | filter %1").arg(RESOURCE->ToString(eFilter), -10));
    LogInApp(QString(" Capture | filter %1").arg(RESOURCE->ToString(eFilter), -10));

    SetupConfig_t setupConfig;
    MeasureConfigWithCropFactor_t measureConfig;

    setupConfig.sensorTemperature = config.sensorTemperature;
    setupConfig.eFilter = eFilter;
    setupConfig.eNd = config.eNd;
    setupConfig.eIris = config.eIris;

    ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_Setup;

    eError = ConoscopeAppProcess::CmdSetup(setupConfig);

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
        LogInFile(" Capture | CmdSetup Done");
        LogInApp(" Capture | CmdSetup Done");
    }
    else
    {
        LogInFile(" Capture | CmdSetup Error");
        LogInApp(" Capture | CmdSetup Error");
    }

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
        }
    }

    ProcessingConfig_t processingConfig;

    processingConfig.bBiasCompensation       = true;
    processingConfig.bSensorDefectCorrection = true;
    processingConfig.bSensorPrnuCorrection   = true;
    processingConfig.bLinearisation          = true;
    processingConfig.bFlatField              = true;
    processingConfig.bAbsolute               = true;

    if(eError == ClassCommon::Error::Ok)
    {
        LogInFile(QString(" Capture | CmdExportProcessed [%1]").arg(config.bSaveCapture == true ? "save capture": "don't save capture"));
        LogInApp(QString(" Capture | CmdExportProcessed [%1]").arg(config.bSaveCapture == true ? "save capture": "don't save capture"));

        // eError = ConoscopeAppProcess::CmdExportProcessed(processingConfig, *(buffer.data), SAVE_PROCESSED_DATA);
        eError = ConoscopeAppProcess::CmdExportProcessed(processingConfig, *(buffer.data), config.bSaveCapture);

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

        if(eError != ClassCommon::Error::Ok)
        {
            LogInFile(QString(" Capture | CmdExportProcessed ERROR (%1)").arg(DisplayCmdExportProcessedError()));
            LogInApp(QString(" Capture | CmdExportProcessed ERROR (%1)").arg(DisplayCmdExportProcessedError()));
        }
    }

    return eError;
}

ClassCommon::Error ConoscopeAppWorker::_Capture(MeasureConfigWithCropFactor_t config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    MeasureConfigWithCropFactor_t measureConfig;

    if(eError == ClassCommon::Error::Ok)
    {
        measureConfig.exposureTimeUs = config.exposureTimeUs;
        measureConfig.nbAcquisition = config.nbAcquisition;
        measureConfig.binningFactor = 1;
        measureConfig.bTestPattern = false;

        LogInFile(QString(" Capture | CmdMeasure %1 us (%2)").arg(measureConfig.exposureTimeUs).arg(measureConfig.nbAcquisition));
        LogInApp(QString(" Capture | CmdMeasure %1 us (%2)").arg(measureConfig.exposureTimeUs).arg(measureConfig.nbAcquisition));
        eError = ConoscopeAppProcess::CmdMeasure(measureConfig);

        if(eError != ClassCommon::Error::Ok)
        {
            LogInFile(" Capture | CmdMeasure ERROR");
            LogInApp(" Capture | CmdMeasure ERROR");
        }
    }

    return eError;
}
#endif /* MULTITHREAD_CAPTURE_SEQUENCE */

#define SATURATION_VALUE 4090

#define MIN_EXPOSURE_TIME_US 10
#define MAX_EXPOSURE_TIME_US 985000
#define MAX_AE_LOOP_COUNT    20

#define ALIGN_VALUE(a, b) a - (a % b)

#include <QFileInfo>

#define IMAGE_INFO_EXTENSION ".json"

ClassCommon::Error ConoscopeAppWorker::_ReadCapture(QString fileName, CaptureSequenceBuffer_t &buffer, CaptureSequenceConfig_t& config, QString& capturePath)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // read binary file
    QFile ff(QString("%1").arg(fileName));

    QByteArray imgData;

    if(ff.exists() && ff.open(QFile::ReadOnly))
    {
        imgData = ff.readAll();
        ff.close();
    }
    else
    {
        //writeInfo(QString("Image does not exist or couldn't be open: %1").arg(acFilename));
        eError = ClassCommon::Error::Failed;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // copy data into the buffer
        buffer.data->resize(imgData.length() / 2);

        char* ptrSrc = (char*)imgData.data();
        char* ptrDst = (char*)buffer.data->data();

        memcpy(ptrDst, ptrSrc, imgData.length());
    }

    if(eError == ClassCommon::Error::Ok)
    {
        QFileInfo fileInfo(fileName);
        QString path = fileInfo.absolutePath();
        QString baseName = fileInfo.baseName();

        capturePath = path;

        QString infoFileName = path + "/" + baseName + IMAGE_INFO_EXTENSION;

#ifdef REMOVED
        ProcessedImageInfo_t info;
#endif

        eError = _ReadConversionFactor(infoFileName, buffer, config);
    }

#ifdef REMOVED
    if(eError == ClassCommon::Error::Ok)
    {
        // read associated data
        struct Camera::RawDataInfo rawDataInfo;
        QMap<QString, QVariant> optionalInfo;

        QFileInfo fileInfo(acFilename);
        QString path = fileInfo.absolutePath();
        QString baseName = fileInfo.baseName();

        // retrieve the date from file name
        QStringList fileNamePart = baseName.split("_");
        if(fileNamePart.count() >= 4)
        {
            info.timeStampString = QString("%1_%2").arg(fileNamePart[0]).arg(fileNamePart[1]);
        }
        else
        {
            info.timeStampString = "";
        }

        QString rawFileName = path + "/" + baseName + IMAGE_INFO_EXTENSION;

        eError = _ReadImageInfo(rawFileName, info);
    }
#endif

    return eError;
}

ClassCommon::Error ConoscopeAppWorker::_ReadConversionFactor(QString filePath, CaptureSequenceBuffer_t &buffer, CaptureSequenceConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // should check is file exists
    if(QFile(filePath).exists())
    {
        QString jsonData;
        QFile jsonFile(filePath);
        if(jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            jsonData = (QString)jsonFile.readAll();
            jsonFile.close();

            // retrieve data from the json file
            QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonData.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();

            QJsonObject processedDataObject = jsonObject["ProcessedData"].toObject();
            QJsonObject setupObject         = jsonObject["Setup"].toObject();

            buffer.convFactX = processedDataObject["conversionFactorCompX"].toDouble();
            buffer.convFactY = processedDataObject["conversionFactorCompY"].toDouble();
            buffer.convFactZ = processedDataObject["conversionFactorCompZ"].toDouble();

            // = RESOURCE->Convert(ConoscopeResource::ResourceType_Filter, setupObject["filter"].toString());
            config.eIris   = (IrisIndex_t)RESOURCE->Convert(ConoscopeResource::ResourceType_Iris, setupObject["iris"].toString());
            config.eNd     = (Nd_t)RESOURCE->Convert(ConoscopeResource::ResourceType_Nd, setupObject["nd"].toString());

            QJsonObject roiObject = jsonObject["ROI"].toObject();

            // bool      bUseRoi;          // ROI of the processed data
            // int       RoiXLeft;         // ROI
            // int       RoiXRight;        // ROI
            // int       RoiYTop;          // ROI
            // int       RoiYBottom;       // ROI
            config.RoiXLeft   = roiObject["XLeft"].toInt();
            config.RoiXRight  = roiObject["XRight"].toInt();
            config.RoiYTop    = roiObject["YTop"].toInt();
            config.RoiYBottom = roiObject["YBottom"].toInt();
        }
    }
    else
    {
        eError = ClassCommon::Error::Failed;
    }

    return eError;
}


QString ConoscopeAppWorker::DisplayCmdExportProcessedError()
{
    QString message;

    Conoscope::CmdExportProcessedOutput_t output;
    output = ConoscopeAppProcess::cmdExportProcessedOutput;

    if(output.cameraCfgFileName.valid == false)
    {
        message = QString("file %1 missing").arg(output.cameraCfgFileName.data);
    }
    else if (output.opticalColumnCfgFileName.valid == false)
    {
        message = QString("file %1 missing").arg(output.opticalColumnCfgFileName.data);
    }
    else if (output.flatFieldFileName.valid == false)
    {
        message = QString("file %1 missing").arg(output.opticalColumnCfgFileName.data);
    }
    else
    {
        message = "unknown";
    }

    return message;
}

#define PROCESS_ROI

ClassCommon::Error ConoscopeAppWorker::_Compose(
        std::vector<int16_t>& mBuffer,
        std::vector<float_t>& output,
        double conversionFactor,
        ComposeMode_t mode,
        QString fileName)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    std::vector<float_t> cropBuffer;

    int bufferSize = (int)mBuffer.size();

#ifdef PROCESS_ROI
    if(ConoscopeAppWorker::mSettings.bUseRoi == false)
    {
#endif
        output.resize(bufferSize);
#ifdef PROCESS_ROI
    }
    else
    {
        // resize depending on ROI
        int outputHeight = ConoscopeProcess::mSettings.RoiYBottom - ConoscopeProcess::mSettings.RoiYTop;
        int outputWidth  = ConoscopeProcess::mSettings.RoiXRight - ConoscopeProcess::mSettings.RoiXLeft;

        int outputSize = outputHeight * outputWidth;

        output.resize(outputSize);
    }
#endif

    // clean the buffer if not in append mode
    if(mode == ComposeMode_Normal)
    {
        memset(output.data(), 0, output.size() * sizeof(float_t));
    }

    // crop image according to settings
    int cropHeight  = ConoscopeProcess::mSettings.RoiYBottom - ConoscopeProcess::mSettings.RoiYTop;
    int cropWidth   = ConoscopeProcess::mSettings.RoiXRight - ConoscopeProcess::mSettings.RoiXLeft;
    int cropOffsetX = ConoscopeProcess::mSettings.RoiXLeft;
    int cropOffsetY = ConoscopeProcess::mSettings.RoiYTop;
    int lineLenght  = ConoscopeAppProcess::cmdExportProcessedOutput.width;

    // input and output data pointers
    int16* pBuffer = (int16*)mBuffer.data();
    float* pCompose = output.data();

    if(conversionFactor != 0)
    {
#ifdef PROCESS_ROI
        if(ConoscopeAppWorker::mSettings.bUseRoi == false)
        {
#endif
            // retrive the correction factor
            for(int index = 0; index < bufferSize; index ++)
            {
                 pCompose[index] += pBuffer[index] * conversionFactor;
            }
#ifdef PROCESS_ROI
        }
        else
        {
            for(int lineIndex = 0; lineIndex < cropHeight; lineIndex ++)
            {
                for(int rowIndex = 0; rowIndex < cropWidth; rowIndex ++)
                {
                    pCompose[(lineIndex * cropWidth + rowIndex)] += pBuffer[(cropOffsetY + lineIndex) * lineLenght + cropOffsetX + rowIndex] * conversionFactor;
                }
            }
        }
#endif
    }

    // save the file
    if(!fileName.isEmpty())
    {
        fileName = QString("%1.%2").arg(fileName).arg(CAPTURE_EXTENSION);
        fileName.replace("__", "_");

        QFile filePointer(fileName);

        if(!filePointer.open(QFile::WriteOnly))
        {
            LogInApp(QString("Failed to open file: %1").arg(fileName));
            eError = ClassCommon::Error::Failed;
        }
        else
        {
#ifndef PROCESS_ROI
            if(ConoscopeAppWorker::mSettings.bUseRoi == true)
            {
                // crop image according to settings
                // int cropHeight = ConoscopeProcess::mSettings.RoiYBottom - ConoscopeProcess::mSettings.RoiYTop;
                // int cropWidth  = ConoscopeProcess::mSettings.RoiXRight - ConoscopeProcess::mSettings.RoiXLeft;

                // int cropOffsetX = ConoscopeProcess::mSettings.RoiXLeft;
                // int cropOffsetY = ConoscopeProcess::mSettings.RoiYTop;

                // int lineLenght = ConoscopeAppProcess::cmdExportProcessedOutput.width;

                // allocate memory for crop buffer
                bufferSize = cropHeight * cropWidth;
                cropBuffer.resize(bufferSize);
                float* pCropData = (float*)cropBuffer.data();

                // copy into crop buffer
                for(int lineIndex = 0; lineIndex < cropHeight; lineIndex ++)
                {
                    for(int rowIndex = 0; rowIndex < cropWidth; rowIndex ++)
                    {
                        pCropData[(lineIndex * cropWidth + rowIndex)] = pCompose[(cropOffsetY + lineIndex) * lineLenght + cropOffsetX + rowIndex];
                    }
                }

                // set the data to save
                pCompose = pCropData;
            }
#endif
            // save the buffer
            filePointer.write((char*)pCompose, bufferSize * sizeof(float));
            filePointer.close();
        }
    }

    return eError;
}

ClassCommon::Error ConoscopeAppWorker::_ComposeComponents(QString fileName, QMap<Filter_t, CaptureSequenceBuffer_t>& bufferList, QString appendFileName)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    _Compose(*(bufferList[Filter_X].data),  mCompose, bufferList[Filter_X].convFactX);
    _Compose(*(bufferList[Filter_Xz].data), mCompose, bufferList[Filter_Xz].convFactX, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Ya].data), mCompose, bufferList[Filter_Ya].convFactX, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Yb].data), mCompose, bufferList[Filter_Yb].convFactX, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Z].data),  mCompose, bufferList[Filter_Z].convFactX, ComposeMode_Append,
             QString(FLOAT_FILE_NAME).arg(fileName).arg(appendFileName).arg("X"));

    _Compose(*(bufferList[Filter_X].data),  mCompose, bufferList[Filter_X].convFactY);
    _Compose(*(bufferList[Filter_Xz].data), mCompose, bufferList[Filter_Xz].convFactY, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Ya].data), mCompose, bufferList[Filter_Ya].convFactY, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Yb].data), mCompose, bufferList[Filter_Yb].convFactY, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Z].data),  mCompose, bufferList[Filter_Z].convFactY, ComposeMode_Append,
             QString(FLOAT_FILE_NAME).arg(fileName).arg(appendFileName).arg("Y"));

    _Compose(*(bufferList[Filter_X].data),  mCompose, bufferList[Filter_X].convFactZ);
    _Compose(*(bufferList[Filter_Xz].data), mCompose, bufferList[Filter_Xz].convFactZ, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Ya].data), mCompose, bufferList[Filter_Ya].convFactZ, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Yb].data), mCompose, bufferList[Filter_Yb].convFactZ, ComposeMode_Append);
    _Compose(*(bufferList[Filter_Z].data),  mCompose, bufferList[Filter_Z].convFactZ, ComposeMode_Append,
             QString(FLOAT_FILE_NAME).arg(fileName).arg(appendFileName).arg("Z"));

    return eError;
}

#define EXPOSURETIME_LABEL "ExposureTimeUs"

void ConoscopeAppWorker::_ReadExposureTimeFile(QMap<Filter_t, int>& exposureTimeList)
{
    QString fileName = ".\\CaptureSequenceExposureTime.json";

    QFile jsonFile(fileName);

    // create a default file if it does not exist
    if(!jsonFile.exists())
    {
        QJsonObject objectConfig;

        objectConfig.insert("Filter_X",  exposureTimeList[Filter_X]);
        objectConfig.insert("Filter_Xz", exposureTimeList[Filter_Xz]);
        objectConfig.insert("Filter_Ya", exposureTimeList[Filter_Ya]);
        objectConfig.insert("Filter_Yb", exposureTimeList[Filter_Yb]);
        objectConfig.insert("Filter_Z",  exposureTimeList[Filter_Z]);

        QJsonObject recordObject;
        recordObject.insert(EXPOSURETIME_LABEL, objectConfig);

        QJsonDocument doc(recordObject);

        QFile jsonFile(fileName);
        if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&jsonFile);
            out.setCodec("UTF-8");
            out << doc.toJson();
            jsonFile.close();
        }
    }

    // read the json file
    if(jsonFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileArray;
        fileArray = jsonFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(fileArray));
        QJsonObject object = loadDoc.object();

        QJsonObject objectConfig = object[EXPOSURETIME_LABEL].toObject();

        exposureTimeList[Filter_X]  = objectConfig["Filter_X"].toInt();
        exposureTimeList[Filter_Xz] = objectConfig["Filter_Xz"].toInt();
        exposureTimeList[Filter_Ya] = objectConfig["Filter_Ya"].toInt();
        exposureTimeList[Filter_Yb] = objectConfig["Filter_Yb"].toInt();
        exposureTimeList[Filter_Z]  = objectConfig["Filter_Z"].toInt();

        jsonFile.close();
    }
}

#define FILEPATH_LABEL "FilePath"

void ConoscopeAppWorker::_ReadCapturesFile(QMap<Filter_t, QString> &fileList)
{
    QString fileName = ".\\CaptureSequenceCaptures.json";

    QFile jsonFile(fileName);

    // create a default file if it does not exist
    if(!jsonFile.exists())
    {
        QJsonObject objectConfig;

        objectConfig.insert("Filter_X",  "capture_X.bin");
        objectConfig.insert("Filter_Xz", "capture_Xz.bin");
        objectConfig.insert("Filter_Ya", "capture_Ya.bin");
        objectConfig.insert("Filter_Yb", "capture_Yb.bin");
        objectConfig.insert("Filter_Z",  "capture_Z.bin");

        QJsonObject recordObject;
        recordObject.insert(FILEPATH_LABEL, objectConfig);

        QJsonDocument doc(recordObject);

        QFile jsonFile(fileName);
        if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&jsonFile);
            out.setCodec("UTF-8");
            out << doc.toJson();
            jsonFile.close();
        }
    }

    // read the json file
    if(jsonFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileArray;
        fileArray = jsonFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(fileArray));
        QJsonObject object = loadDoc.object();

        QJsonObject objectConfig = object[FILEPATH_LABEL].toObject();

        fileList[Filter_X]  = objectConfig["Filter_X"].toString();
        fileList[Filter_Xz] = objectConfig["Filter_Xz"].toString();
        fileList[Filter_Ya] = objectConfig["Filter_Ya"].toString();
        fileList[Filter_Yb] = objectConfig["Filter_Yb"].toString();
        fileList[Filter_Z]  = objectConfig["Filter_Z"].toString();

        jsonFile.close();
    }
}

#define EXPORTOPTION_LABEL "ExportOption"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define JSON_INSERT(a, b) object##a.insert(TOSTRING(b), m##a.b)

#define GET_JSON_FEATURE(a, b, c) if(objectConfig.contains(TOSTRING(a)) == true) { \
    b = objectConfig[TOSTRING(a)].toBool(); } else { \
    b = c; bUpdateConfigFile = true; }

void ConoscopeAppWorker::_ReadExposureExportOption(ProcessingConfig_t &processingConfig, CaptureSequenceOption_t  &option)
{
    QString fileName = ".\\CaptureSequenceExportOption.json";

    QFile jsonFile(fileName);

    if(jsonFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileArray;
        fileArray = jsonFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(fileArray));
        QJsonObject object = loadDoc.object();

        QJsonObject objectConfig = object[EXPORTOPTION_LABEL].toObject();

        bool bUpdateConfigFile = false;

        GET_JSON_FEATURE(bAbsolute,               processingConfig.bAbsolute,               true)
        GET_JSON_FEATURE(bBiasCompensation,       processingConfig.bBiasCompensation,       true)
        GET_JSON_FEATURE(bFlatField,              processingConfig.bFlatField,              true)
        GET_JSON_FEATURE(bLinearisation,          processingConfig.bLinearisation,          true)
        GET_JSON_FEATURE(bSensorDefectCorrection, processingConfig.bSensorDefectCorrection, true)
        GET_JSON_FEATURE(bSensorPrnuCorrection,   processingConfig.bSensorPrnuCorrection,   true)
        GET_JSON_FEATURE(generateXYZ,             option.bGenerateXYZ, true)

        QString message = QString("WARNING: use custom Export configuration\n");
        message.append(QString("    bAbsolute                %1\n").arg(processingConfig.bBiasCompensation       ));
        message.append(QString("    bBiasCompensation        %1\n").arg(processingConfig.bSensorDefectCorrection ));
        message.append(QString("    bFlatField               %1\n").arg(processingConfig.bSensorPrnuCorrection   ));
        message.append(QString("    bLinearisation           %1\n").arg(processingConfig.bLinearisation          ));
        message.append(QString("    bSensorDefectCorrection  %1\n").arg(processingConfig.bFlatField              ));
        message.append(QString("    bSensorPrnuCorrection    %1\n").arg(processingConfig.bAbsolute               ));

        message.append(QString("    generateXYZ              %1\n").arg(option.bGenerateXYZ));

        LogInFile(message);
        LogInApp(message);

        jsonFile.close();

        if(bUpdateConfigFile == true)
        {
            _SaveExposureExportOption(fileName, processingConfig, option);
        }
    }
    else
    {
        processingConfig.bBiasCompensation       = true;
        processingConfig.bSensorDefectCorrection = true;
        processingConfig.bSensorPrnuCorrection   = true;
        processingConfig.bLinearisation          = true;
        processingConfig.bFlatField              = true;
        processingConfig.bAbsolute               = true;

        option.bGenerateXYZ = true;
    }
}

void ConoscopeAppWorker::_SaveExposureExportOption(QString fileName, ProcessingConfig_t& processingConfig, CaptureSequenceOption_t &option)
{
    QJsonObject objectConfig;

    objectConfig.insert("bAbsolute",                processingConfig.bAbsolute);
    objectConfig.insert("bBiasCompensation",        processingConfig.bBiasCompensation);
    objectConfig.insert("bFlatField",               processingConfig.bFlatField);
    objectConfig.insert("bLinearisation",           processingConfig.bLinearisation);
    objectConfig.insert("bSensorDefectCorrection",  processingConfig.bSensorDefectCorrection);
    objectConfig.insert("bSensorPrnuCorrection",    processingConfig.bSensorPrnuCorrection);
    objectConfig.insert("generateXYZ",              option.bGenerateXYZ);

    // record
    QJsonObject recordObject;
    recordObject.insert(EXPORTOPTION_LABEL, objectConfig);

    QJsonDocument doc(recordObject);

    QFile jsonFile(fileName);
    if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&jsonFile);
        out.setCodec("UTF-8");
        out << doc.toJson();
        jsonFile.close();
    }
}

ClassCommon::Error ConoscopeAppWorker::_CmdMeasureAE()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    ConoscopeAppHelper::mCancelRequest = false;

    MeasureConfigWithCropFactor_t measureConfig;

    measureConfig.exposureTimeUs = ConoscopeAppWorker::mMeasureConfig.exposureTimeUs;
    measureConfig.nbAcquisition = ConoscopeAppWorker::mMeasureConfig.nbAcquisition;
    measureConfig.binningFactor = 1;
    measureConfig.bTestPattern = false;

    float autoExposurePixelMax = ConoscopeAppWorker::mSettings.AELevelPercent;

    ConoscopeAppWorker::mMeasureStatus.state = MeasureStatus_t::State_t::State_Process;
    eError = ConoscopeAppHelper::_CaptureAutoExposure(measureConfig, mBuffer1, autoExposurePixelMax);

    if(ConoscopeAppHelper::mCancelRequest == true)
    {
        ConoscopeAppWorker::mMeasureStatus.state = MeasureStatus_t::State_t::State_Cancel;
    }
    else
    {
        if(eError == ClassCommon::Error::Ok)
        {
            ConoscopeAppWorker::mMeasureStatus.state = MeasureStatus_t::State_t::State_Done;
        }
        else
        {
            ConoscopeAppWorker::mMeasureStatus.state = MeasureStatus_t::State_Error;
        }
    }

    return eError;
}

void ConoscopeAppWorker::_WriteCaptureSequenceInfo(QString fileName, QMap<Filter_t, int>& exposureTimeList, SomeInfo_t& info)
{
    LogInFile("_WriteCaptureSequenceInfo");

    // measurement part
    QJsonObject infoObject;
    infoObject.insert("File",        fileName);

    QJsonObject cfgFileObject;
    cfgFileObject.insert("camera",               info.cameraCfgFileName);
    cfgFileObject.insert("opticalColumn",        info.opticalColumnCfgFileName);
    cfgFileObject.insert("opticalColumnComment", info.opticalColumnCfgDate);
    cfgFileObject.insert("opticalColumnDate",    info.opticalColumnCfgTime);
    cfgFileObject.insert("opticalColumnTime",    info.opticalColumnCfgComment);

    QJsonObject exposureTimeUsObject;
    exposureTimeUsObject.insert("Filter_X",  exposureTimeList[Filter_X]);
    exposureTimeUsObject.insert("Filter_Xz", exposureTimeList[Filter_Xz]);
    exposureTimeUsObject.insert("Filter_Ya", exposureTimeList[Filter_Ya]);
    exposureTimeUsObject.insert("Filter_Yb", exposureTimeList[Filter_Yb]);
    exposureTimeUsObject.insert("Filter_Z",  exposureTimeList[Filter_Z]);

    QJsonObject roiObject;
    roiObject.insert("bUseRoi",    mSettings.bUseRoi);
    roiObject.insert("RoiXLeft",   mSettings.RoiXLeft);
    roiObject.insert("RoiXRight",  mSettings.RoiXRight);
    roiObject.insert("RoiYTop",    mSettings.RoiYTop);
    roiObject.insert("RoiYBottom", mSettings.RoiYBottom);

    QJsonObject aeObject;
    aeObject.insert("bAutoExposure",    mCaptureSequenceConfig.bAutoExposure);
    if(mCaptureSequenceConfig.bAutoExposure == true)
    {
        aeObject.insert("AEMeasAreaX",      mSettings.AEMeasAreaX);
        aeObject.insert("AEMeasAreaY",      mSettings.AEMeasAreaY);
        aeObject.insert("AEMeasAreaWidth",  mSettings.AEMeasAreaWidth);
        aeObject.insert("AEMeasAreaHeight", mSettings.AEMeasAreaHeight);
    }

    QJsonObject setupObject;
    setupObject.insert("iris", mCaptureSequenceConfig.eIris);
    setupObject.insert("nd", mCaptureSequenceConfig.eNd);

    QJsonObject imageObject;
    imageObject.insert("bUseExpoFile", mCaptureSequenceConfig.bUseExpoFile);

    int imageHeight = ConoscopeAppProcess::cmdExportProcessedOutput.height;
    int imageWidth = ConoscopeAppProcess::cmdExportProcessedOutput.width;

    if(mSettings.bUseRoi == true)
    {
        imageHeight = mSettings.RoiYBottom - mSettings.RoiYTop;
        imageWidth  = mSettings.RoiXRight - mSettings.RoiXLeft;
    }
    imageObject.insert("height", imageHeight);
    imageObject.insert("width", imageWidth);

    // record
    QJsonObject recordObject;

    recordObject.insert("CfgFile",        cfgFileObject);
    recordObject.insert("Info",           infoObject);
    recordObject.insert("ExposureTimeUs", exposureTimeUsObject);
    recordObject.insert("ROI",            roiObject);
    recordObject.insert("AE",             aeObject);
    recordObject.insert("Setup",          setupObject);
    recordObject.insert("Image",          imageObject);

    QJsonDocument doc(recordObject);

    // QString jsonFileName = path + "/"+ name + IMAGE_INFO_EXTENSION;
    QString jsonFileName = fileName + IMAGE_INFO_EXTENSION;

    QFile jsonFile(jsonFileName);
    if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&jsonFile);
        out.setCodec("UTF-8");
        out << doc.toJson();
        jsonFile.close();
    }
}
