#include "cameraCmvCxp.h"

#include <QFuture>
#include <QtConcurrent/qtconcurrentrun.h>

#include "HwTool.h"
#include "cameraCmvCxpHw.h"

#include "imageConfiguration.h"

#define PRINT

#define INVALID_FRAME_INDEX -1

#define LOG_DEBUG(message) _LogMessage(QString("CmvCamera - %1").arg(message))

CameraCmvCxp::CameraCmvCxp(QObject *parent) : Camera(parent)
{
    mGentl = NULL;
    mGrabber = NULL;

    eState = CameraState_NotConnected;

    mCaptureState = Camera::Status::NotInitialised;
    mCurrentFrameIndex = INVALID_FRAME_INDEX;

    // following may not change
    mFwType.insert("FPGA", CoaXpressGrabber::eFirmwareType::FPGA);
    mFwType.insert("NIOS", CoaXpressGrabber::eFirmwareType::NIOS);
}

CameraCmvCxp::~CameraCmvCxp()
{
    if(mGrabber != NULL)
    {
        delete(mGrabber);
        mGrabber = NULL;
    }

    if(mGentl != NULL)
    {
        delete(mGentl);
        mGentl = NULL;
    }
}

bool CameraCmvCxp::IsConnected()
{
    return(eState == CameraState_Connected) ? true : false;
}

bool CameraCmvCxp::IsFileTransferSupported()
{
    bool res = false;
    if(eState == CameraState_Connected)
    {
        res = mGrabber->IsFileTransferSupported();
    }

    return res;
}

void CameraCmvCxp::FileTransferOpen()
{
    mGrabber->FileTransferOpen();
}

void CameraCmvCxp::FileTransferClose()
{
    mGrabber->FileTransferClose();
}

void CameraCmvCxp::FileTransferWrite()
{
    mGrabber->FileTransferWrite();
}

void CameraCmvCxp::FileTransferRead()
{
    mGrabber->FileTransferRead();
}

void CameraCmvCxp::FileTransferWrite(const void *data, size_t size)
{
    mGrabber->FileTransferWrite(data, size);
}

void CameraCmvCxp::FileTransferRead(void *data, size_t size)
{
    mGrabber->FileTransferRead(data, size);
}

int CameraCmvCxp::FwUpdateConfigure(size_t size)
{
    connect(mGrabber, &CoaXpressGrabber::ProgressUpdate,
            this, &CameraCmvCxp::ProgressUpdate);

    return mGrabber->FwUpdateConfigure(size);
}

ClassCommon::Error CameraCmvCxp::FwUpdate(QString type, char* firmware, size_t firmwareSize)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if(mFwType.contains(type) == true)
    {
        eError = mGrabber->FwUpdate(mFwType[type], firmware, firmwareSize);
    }
    else
    {
        eError = ClassCommon::Error::InvalidParameter;
    }

    return eError;
}

ClassCommon::Error CameraCmvCxp::Connect()
{
    return _Connect(false);
}

ClassCommon::Error CameraCmvCxp::ConnectPowerCycle()
{
    return _Connect(true);
}

ClassCommon::Error CameraCmvCxp::_Connect(bool bPowerCycle)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    _Disconnect();

    if(mGentl == NULL)
    {
        // LOG_DEBUG(QString("mGentl"));
        mGentl = new Euresys::EGenTL;
    }
/*
    GenTL::TL_HANDLE tl = mGentl->tlOpen();
    uint32_t numCards = mGentl->tlGetNumInterfaces(tl);

    PRINT("Initialise", QString("numCards %1").arg(numCards));

    for (uint32_t n = 0; n < numCards; ++n)
    {
        std::string id = mGentl->tlGetInterfaceID(tl, n);

        QString test = QString::fromUtf8(id.c_str());

        PRINT("Initialise", QString("%1").arg(test));
    }

    PRINT("GenTL", "Created");
*/

    if(bPowerCycle == true)
    {
        GenTL::TL_HANDLE tl = mGentl->tlOpen();

        std::string id = mGentl->tlGetInterfaceID(tl, 0);
        gc::IF_HANDLE interfaceH = mGentl->tlOpenInterface(tl, id);

        std::string regValue = "";

        // std::string regValue =  mGentl->genapiGetString(interfaceH, "CxpPoCxpStatus");

        mGentl->genapiExecuteCommand(interfaceH, "CxpPoCxpTurnOff");
        QThread::msleep(500);
        mGentl->genapiExecuteCommand(interfaceH, "CxpPoCxpAuto");

        int timeout = 0;
        bool bCameraStarted = false;

        #define MAX_TIMEOUT 100

        do
        {
            QThread::msleep(1000);
            regValue =  mGentl->genapiGetString(interfaceH, "CxpDeviceConnectionID");

            //connectionId = getString<Euresys::InterfaceModule>("CxpDeviceConnectionID");

            if(regValue.compare("NotReady") != 0)
            {
                bCameraStarted = true;
            }
            else
            {
                timeout ++;
            }

        } while((bCameraStarted == false) && (timeout < MAX_TIMEOUT));

        bPowerCycle = false;

        mGentl->tlClose(tl);
    }

    try
    {
        // LOG_DEBUG(QString("new mGrabber (PowerCycle = %1)").arg(bPowerCycle));
        mGrabber = new CoaXpressGrabber(*mGentl, bPowerCycle, this);

        connect(mGrabber, &CoaXpressGrabber::LogInFile,
                this, &Camera::_LogInFile);

        // LOG_DEBUG(QString("new mGrabber DONE"));

        mGrabber->getInterfaceInfo();

        // LOG_DEBUG(QString("DetectCamera"));
        mGrabber->DetectCamera(this->mConnectionConfig.toUtf8().constData());

        // configure image size
        // LOG_DEBUG(QString("GetCameraInfo"));
        CoaXpressGrabber_CameraInfo cameraInfo;
        mGrabber->GetCameraInfo(cameraInfo);
        mModel = GetCameraModel(cameraInfo);

        // imageConfiguration (default value matches cmv8000)
        ImageConfiguration* imageConfiguration = ImageConfiguration::Get();

        if(mModel == CameraModel_CmvCxp_50k)
        {
            IMAGE_HORIZONTAL_OFFSET                = 0; //IMAGE_HORIZONTAL_OFFSET;
            IMAGE_VERTICAL_OFFSET                  = 0; // IMAGE_VERTICAL_OFFSET;

            IMAGE_WIDTH                            = 7920;
            IMAGE_HEIGHT                           = 6004;

            ACTIVE_HORIZONTAL_OFFSET  = 958;
            ACTIVE_VERTICAL_OFFSET    = 0;

            ACTIVE_WIDTH  = 6004;
            ACTIVE_HEIGHT = 6004;

            imageConfiguration->UpdateConfiguration();
        }
        else if(mModel == CameraModel_CmvCxp_8k)
        {
            IMAGE_HORIZONTAL_OFFSET                = 0; // IMAGE_HORIZONTAL_OFFSET;
            IMAGE_VERTICAL_OFFSET                  = 0; // IMAGE_VERTICAL_OFFSET;

            IMAGE_WIDTH                            = 3360;
            IMAGE_HEIGHT                           = 2496;

            ACTIVE_HORIZONTAL_OFFSET  = 0;
            ACTIVE_VERTICAL_OFFSET    = 0;

            ACTIVE_WIDTH  = 3360;
            ACTIVE_HEIGHT = 2496;

            imageConfiguration->UpdateConfiguration();
        }

        // initialise file transfer
/*
        if(mGrabber->IsFileTransferSupported())
        {
            mGrabber->FileTransferClose();
        }
*/

        // connect the signal on the signal
#ifdef FRAME_CAPTURED_SIGNAL
        connect(mGrabber, &CoaXpressGrabber::FrameCaptured,
                this, &CameraCmvCxp::onFrameCaptured);
#endif

        PRINT("Grabber", "Created");

        eState = CameraState_Connected;
        mCaptureState = Camera::Status::Ready;

        NotifyEvent(Event::Connect);
    }
    catch(gentl_error gentlException)
    {
        LOG_DEBUG(QString("new mGrabber Exception"));
        // delete (mGrabber);
        mGrabber = NULL;

        delete(mGentl);
        mGentl = NULL;

        mCaptureState = Camera::Status::NotInitialised;

        eError = ClassCommon::Error::Failed;
    }

    return eError;
}

ClassCommon::Error CameraCmvCxp::_Disconnect()
{
    if(mGrabber != NULL)
    {
        delete(mGrabber);
        mGrabber = NULL;
    }

    if(mGentl != NULL)
    {
        delete(mGentl);
        mGentl = NULL;
    }

    mCaptureState = Camera::Status::NotInitialised;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::Disconnect()
{
    ClassCommon::Error eError = _Disconnect();

    eState = CameraState_NotConnected;
    NotifyEvent(Event::Disconnect);

    return eError;
}

ClassCommon::Error CameraCmvCxp::SetDefaultConfiguration()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mGrabber->SetCameraDefaultConfiguration();

    return eError;
}

CameraModel_t CameraCmvCxp::GetCameraModel(CoaXpressGrabber_CameraInfo& cameraInfo)
{
    CameraModel_t eModel;

    if(cameraInfo.cameraVendorName == "CriticalLink")
    {
        if(cameraInfo.cameraModelName == "CMV8000_CXP")
        {
            eModel = CameraModel_CmvCxp_8k;
        }
        else
        {
            eModel = CameraModel_CmvCxp_50k;
        }
    }
    else if(cameraInfo.cameraVendorName == "Adimec")
    {
        eModel = CameraModel_Adimec_50k;
    }

    return eModel;
}

ClassCommon::Error CameraCmvCxp::GetInfo(CameraInfo_t& info)
{
    CoaXpressGrabber_CameraInfo cameraInfo;

    if(mGrabber != NULL)
    {
        mGrabber->GetCameraInfo(cameraInfo);

        // mModel = GetCameraModel(cameraInfo);

        mGrabber->GetTemperature(info.settings);

        info.cameraBoardSerialNumber = QString("%1").arg(cameraInfo.cameraSerialNumber.c_str());
        info.CpuBoardRev       = "";
        info.SensorBoardRev    = "";
        info.CameraRev         = QString("%1").arg(cameraInfo.cameraVersion.c_str());
        info.SoftwareVersion   = QString("%1").arg(cameraInfo.cameraFirmwareVersion.c_str());

        info.ModelName         = QString("%1").arg(cameraInfo.deviceModelName.c_str());
        info.VendorName        = QString("%1").arg(cameraInfo.deviceVendorName.c_str());

        for(int index = 0; index < cameraInfo.cameraSpecific.length(); index ++)
        {
            info.cameraSpecific.append(
                    CameraInfoItem(cameraInfo.cameraSpecific[index].name,
                                   cameraInfo.cameraSpecific[index].value));
        }

        CameraInfo_t info_;
        Camera::GetInfo(info_);

        if(info_.cameraSerialNumber.valid == true)
        {
            info.cameraSerialNumber = info_.cameraSerialNumber;
        }
    }

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::CheckConnection()
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    if(mGrabber != NULL)
    {
        if(mGrabber->HasCameraChanged() == true)
        {
            eError = ClassCommon::Error::Ok;
        }
    }

    return eError;
}

ClassCommon::Error CameraCmvCxp::GetInterfaceInfo(Camera::InterfaceInfo_t& interfaceInfo)
{
    if(mGrabber != NULL)
    {
        CoaXpressGrabber_InterfaceInfo info;
        mGrabber->GetInterfaceInfo(info);

        interfaceInfo.version = info.version;
    }

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::GetSerialNumber(
        QString& cameraSerialNumber,
        QString& cameraBoardSerialNumber)
{
    if(mGrabber != NULL)
    {
        CoaXpressGrabber_CameraInfo cameraInfo;

        mGrabber->GetCameraInfo(cameraInfo);
        cameraBoardSerialNumber = QString("%1").arg(cameraInfo.cameraSerialNumber.c_str());

        CameraInfo_t info;
        Camera::GetInfo(info);

        if(info.cameraSerialNumber.valid == true)
        {
            cameraSerialNumber = info.cameraSerialNumber.data;
        }
    }

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::GetSettings(CameraSettings_t& settings)
{
    if(mGrabber != NULL)
    {
        CoaXpressGrabber_CameraSettings cameraSettings;
        mGrabber->GetCameraSettings(cameraSettings);

        settings.settingList.clear();

        settings.settingList.append(settingItem("PixelFormat",                 cameraSettings.pixelFormat));
        settings.settingList.append(settingItem("binningHorizontal",           std::to_string(cameraSettings.binningHorizontal)));
        settings.settingList.append(settingItem("binningVertical",             std::to_string(cameraSettings.binningVertical)));
        settings.settingList.append(settingItem("DefectPixelCorrectionEnable", cameraSettings.DefectPixelCorrectionEnable == true ? "true" : "false"));
        settings.settingList.append(settingItem("DF_ColumnOffsetCorrection",   cameraSettings.DF_ColumnOffsetCorrection   == true ? "true" : "false"));
        settings.settingList.append(settingItem("BF_ColumnGainCorrection",     cameraSettings.BF_ColumnGainCorrection     == true ? "true" : "false"));
        settings.settingList.append(settingItem("exposureMode",                cameraSettings.exposureMode));
        settings.settingList.append(settingItem("blackLevel",                  std::to_string(cameraSettings.blackLevel)));
        settings.settingList.append(settingItem("SensitivityMatching",         cameraSettings.SensitivityMatching          == true ? "true" : "false"));
    }

    return ClassCommon::Error::Ok;
}

void AsyncNotifyEvent(CameraCmvCxp* m_Camera, int eventIndex)
{
    emit m_Camera->EventOccured(eventIndex);
}

void CameraCmvCxp::NotifyEvent(Event eEvent)
{
    // execute the action asynchronously
    // else, in some cases, a key is not properly captured
    // and is processed twice
    QtConcurrent::run(AsyncNotifyEvent, this, (int)eEvent);
}

void CameraCmvCxp::onFrameCaptured(int frameIndex)
{
    if(mCaptureState == Camera::Status::MeasurementPending)
    {
        // change capture state
        mCaptureState = Camera::Status::MeasurementDone;
        mCurrentFrameIndex = frameIndex;

        emit FrameCaptured(frameIndex);
    }
}

ClassCommon::Error CameraCmvCxp::Configure(struct CaptureConfig* pConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    StopMeasurement();

    if(mCaptureState == Camera::Status::Ready)
    {
        // configure the camera according to the parameters
        CoaXpressGrabber_Config config;

        config.exposureUs        = pConfig->mnExposureMicros;
        config.acquisitionNumber = pConfig->mnNumImages;
        config.VBin              = pConfig->mnVBin;
        config.dimensions        = pConfig->mcDimensions;
        config.bExtTrig          = pConfig->mbExtTrig;
        config.bTestPattern      = pConfig->mbTestPattern;
        config.trigDelayMs       = pConfig->mnTrigDelayMicros;
#ifdef STD_DEV_FILE
        config.bStoreStdDev      = pConfig->bStoreStdDev;
#endif
        if(mGrabber != NULL)
        {
            mGrabber->Configure(config);
            mGrabber->GetExposureTime(pConfig->mnExposureMicros);
        }
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

int CameraCmvCxp::GetCurrentCaptureTimeoutSteps(int stepMs)
{
    if(mGrabber != NULL)
    {
        return mGrabber->GetCurrentCaptureTimeoutSteps(stepMs);
    }
    else
    {
        return 0;
    }
}

ClassCommon::Error CameraCmvCxp::ConfigurePipeline(struct PipelineConfig *pConfig){
    // no implemented in the current version of the camera
    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::GetStatus(enum Status& eStatus)
{
    eStatus = mCaptureState;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::StartMeasurement()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

#ifdef CAMERA_LOG_IN_FILE
    _LogInFile("[Camera]", "StartMeasurement");
#endif

    if((mCaptureState == Camera::Status::Ready) && (mGrabber != NULL))
    {
        mCaptureState = Camera::Status::MeasurementPending;

        // launch the capture
#ifdef FRAME_CAPTURED_SIGNAL
        mGrabber->Start();
#else
        // capture status is not received by a signal anymore
        // (because, in some cases, it does not work with python script)
        // Then after a start, the capture is done so it is possible to retrieve data
        // WARNING: This will work only in POP mode else Start is asynchronous

#ifdef CAMERA_LOG_IN_FILE
        _LogInFile("[Camera]", "mGrabber->Start");
#endif

        bool res = false;
        res = mGrabber->Start();

#ifdef CAMERA_LOG_IN_FILE
        _LogInFile("[Camera]", "frame Captured");
#endif

        if(res == true)
        {
            // capture is done, retrieve current frame index
            int frameIndex = mGrabber->GetCurrentFrameIndex();

            // somehow emulate the signal
            onFrameCaptured(frameIndex);
        }
#endif
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

ClassCommon::Error CameraCmvCxp::StopMeasurement()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if((mCaptureState != Camera::Status::Ready) && (mGrabber != NULL))
    {
        mCaptureState = Camera::Status::Ready;

        // stop the capture
        mGrabber->Stop();
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

ClassCommon::Error CameraCmvCxp::GetPulse(struct Pulse* pPulse){
    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraCmvCxp::GetRawData(struct RawDataInfo &info, QByteArray& arr, QByteArray& stdDev)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if(mCaptureState == Camera::Status::MeasurementDone)
    {
        if(mCurrentFrameIndex != INVALID_FRAME_INDEX)
        {
            // retrieve picture
            ImageFrame* pFrame = CoaXpressFrame::GetImage(mCurrentFrameIndex);

            // fill the info structure
            info.miLines = pFrame->mFeature.height;
            info.miCols  = pFrame->mFeature.width;

            // temperature should be captured at the moment of the frame capture
            if(mGrabber != NULL)
            {
                mGrabber->GetTemperature(info.settings);
            }

            // fill the raw data array
            // pFrame->mImageVector

            int arraySize = info.miLines * info.miCols * sizeof(uint16_t);
#ifdef STD_DEV_FILE
#ifndef STD_DEV_FLOAT
            int stdDevArraySize = arraySize;
#else
            int stdDevArraySize = info.miLines * info.miCols * sizeof(float);
#endif
#endif /* STD_DEV_FILE */
            arr.resize(arraySize);

#ifdef STD_DEV_FILE
            if(pFrame->bStoreStdDev == true)
            {
                stdDev.resize(stdDevArraySize);
            }
#endif

            uchar* pLine = (uchar*)arr.data();
#ifdef STD_DEV_FILE
            uchar* pStdDev = (uchar*)stdDev.data();
#endif

            if(pFrame->mFeature.eCameraManufacturer == CameraManufacturer_CriticalLink)
            {
                // resize the picture to the right dimensions

                // on the original picture stitch appear at 3960. that means that there is no lateral offset
                // vertical offset is 22 (CRITICAL_LINK_VERTICAL_OFFSET)

#ifndef CRITICAL_LINK_FULL_FRAME
                char* ptrDst = (char*)pLine;
                char* ptrSrc = (char*)&pFrame->mImageVector[0];
#ifdef STD_DEV_FILE
                char* ptrDstStdDev = (char*)&pFrame->mStdVector[0];
#endif

                int offsetY = 0;

                if(mModel == CameraModel_CmvCxp_50k)
                {
                    offsetY = CRITICAL_LINK_VERTICAL_OFFSET - pFrame->mFeature.offsetY;
                }
                else if(mModel == CameraModel_CmvCxp_8k)
                {
                    offsetY = 0;
                }

// #define TEST_MEASUREMENT
#ifdef TEST_MEASUREMENT
                // following part is used to check AE area
                // the capture is divided into 4 parts
                // where value depends from exposure time
                // so depending on the AE ROI, the result exposure time will not be the same
                // (of course those test data are in a location where there is no AE... so it should not be in this component)
                int exposureTime;
                mGrabber->GetExposureTime(exposureTime);

                float factLine = 1;
                float factCol  = 1;
                float value = 0;

                int frameIndex = CRITICAL_LINK_VERTICAL_OFFSET * pFrame->mWidth;
                for(int lineIndex = 0; lineIndex < 6004; lineIndex ++)
                {
                    if(lineIndex == 3002)
                    {
                        factLine += 0.6;
                    }

                    factCol = factLine;

                    for(int rowIndex = 0; rowIndex < pFrame->mWidth; rowIndex ++)
                    {
                        if(rowIndex == 3960)
                        {
                            factCol += 0.3;
                        }

                        value = ((float)exposureTime / 10.0);
                        value = (int)((float)value * factCol);

                        if(value > 4095) value = 4095;

                        pFrame->mImageVector[frameIndex ++] = (int)value;
                    }
                }
#endif

                int cropOffsetX = info.cropArea.x();
                int cropOffsetY = info.cropArea.y();

                int lineLenght = info.cropArea.width();
                int nbLines    = info.cropArea.height();

#ifdef AE_ROI
                if((pFrame->mFeature.width >= IMAGE_WIDTH) ||
                   (pFrame->mFeature.height >= IMAGE_HEIGHT))
                {
                    cropOffsetX = 0;
                    cropOffsetY = 0;
                    lineLenght  = IMAGE_WIDTH;
                    nbLines     = IMAGE_HEIGHT;

                    offsetY = CRITICAL_LINK_VERTICAL_OFFSET;
                }
                else
                {
                    cropOffsetX = 0;
                    cropOffsetY = 0;

                    offsetY = 0;
                }
#else
                if(lineLenght == 0 || nbLines == 0)
                {
                    cropOffsetX = 0;
                    cropOffsetY = 0;
                    lineLenght  = IMAGE_WIDTH;
                    nbLines     = IMAGE_HEIGHT;

                    offsetY = CRITICAL_LINK_VERTICAL_OFFSET;
                }
                else
                {
                    // full frame is captured, so need to add the offset to remove blanking lines
                    offsetY = CRITICAL_LINK_VERTICAL_OFFSET;
                }
#endif

#pragma omp parallel for num_threads(4)
                for(int lineIndex = 0; lineIndex < nbLines; lineIndex ++)
                {
                    memcpy(&ptrDst[lineIndex * lineLenght * sizeof(uint16_t)],
                           &ptrSrc[(lineIndex + offsetY + cropOffsetY) * pFrame->mFeature.width * sizeof(uint16_t) +
                                   (cropOffsetX * sizeof(uint16_t))],
                           lineLenght * sizeof(uint16_t));
                }

                info.miLines = nbLines;
                info.miCols  = lineLenght;

                int arraySize = info.miLines * info.miCols * sizeof(uint16_t);
                arr.resize(arraySize);
#else
                memcpy(pLine, &pFrame->mImageVector[0], arraySize);
#endif
            }
            else
            {
                if(pFrame->mFeature.eFormat == PixelFormat_Mono8)
                {
                    // can not copy the pixel directly because input is store in 8 bits
                    // and output in 16 bits
                    for(int pixelIndex = 0; pixelIndex < info.miLines * info.miCols; pixelIndex++)
                    {
                        pLine[pixelIndex] = pFrame->mImageVector[pixelIndex];
                    }
                }
                else
                {
                    memcpy(pLine, &pFrame->mImageVector[0], arraySize);
                }

#ifdef STD_DEV_FILE
                if(pFrame->bStoreStdDev == true)
                {
                    memcpy(pStdDev, &pFrame->mStdVector[0], stdDevArraySize);
                }
#endif
            }

            mCurrentFrameIndex = INVALID_FRAME_INDEX;
            mCaptureState = Camera::Status::Ready;
        }
        else
        {
            mCaptureState = Camera::Status::Fault;
            eError = ClassCommon::Error::InvalidParameter;
        }
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

#ifndef COAXPRESS_FRAME_AVERAGE
void CameraCmvCxp::UpdateCaptureConfiguration(
            int& numberReads, int& numberCaptures)
{
    // only one capture can be performed with this camera
    // so the average part is done by OQC
    if(numberReads != 1)
    {
        numberCaptures *= numberReads;
        numberReads = 1;
    }

}
#endif

#ifdef SOAP_INTERFACE
void CameraCmvCxp::SetPrnu(ns1__prnu *data, struct ns1__setPRNUResponse &_param_1)
{
}

void CameraCmvCxp::SetSensorDefects(ns1__sensorDefects *defects, struct ns1__setSensorDefectsResponse &_param_1)
{
}

void CameraCmvCxp::SetSensorSaturation(ns1__sensorSaturation *value, struct ns1__setSensorSaturationResponse &_param_1)
{
}

void CameraCmvCxp::SetPartInformation(ns1__part *partIndex, ns1__partInformation *partInformation, struct ns1__setPartInformationResponse &_param_1)
{
}

void CameraCmvCxp::StoreConfigurationFile(struct ns1__storeConfigurationFileResponse &_param_1)
{
}
#endif

void CameraCmvCxp::GetCameraInfo(
        QString     &cameraInfo,
        QJsonObject &cameraInfoJson)
{
    if(mGrabber != NULL)
    {
        CoaXpressGrabber::GetInfo_t infoData;

        CameraSettings settings;

        mGrabber->GetTemperature(settings);

        mGrabber->GetInfo(infoData);

        // capture size may not be the size extracted
        infoData.Width = IMAGE_WIDTH;
        infoData.Height = IMAGE_HEIGHT;

        // fill the string
        cameraInfo.clear();

        cameraInfo.append(QString("  serialNumber                  %1\n\n").arg(infoData.serialNumber));
        cameraInfo.append(QString("  pixelFormat                   %1\n").arg(infoData.pixelFormat));
        cameraInfo.append(QString("  DefectPixelCorrectionEnable   %1\n").arg(infoData.DefectPixelCorrectionEnable));
        cameraInfo.append(QString("  DF_ColumnOffsetCorrection     %1\n").arg(infoData.DF_ColumnOffsetCorrection));
        cameraInfo.append(QString("  BF_ColumnGainCorrection       %1\n").arg(infoData.BF_ColumnGainCorrection));
        cameraInfo.append(QString("  SensitivityMatching           %1\n").arg(infoData.SensitivityMatching));
        cameraInfo.append(QString("  exposureMode                  %1\n").arg(infoData.ExposureMode));
        cameraInfo.append(QString("  blackLevel                    %1\n").arg(infoData.BlackLevel));
        cameraInfo.append(QString("  ReverseX                      %1\n").arg(infoData.ReverseX));
        cameraInfo.append(QString("  binningHorizontal             %1\n").arg(infoData.BinningHorizontal));
        cameraInfo.append(QString("  binningVertical               %1\n\n").arg(infoData.BinningVertical));
        cameraInfo.append(QString("  size (offset x, y)            %1 x %2   (%3, %4) \n")
                                 .arg(infoData.Width).arg(infoData.Height).arg(infoData.OffsetX).arg(infoData.OffsetY));
        cameraInfo.append(QString("  AcquisitionFramePeriod        %1   (raw = %2)\n").arg(infoData.AcquisitionFramePeriod, -10).arg(infoData.AcquisitionFramePeriodRaw));
        cameraInfo.append(QString("  ExposureTime                  %1   (raw = %2)\n").arg(infoData.ExposureTime, -10).arg(infoData.ExposureTimeRaw));

        QJsonObject temperatureJson;

        for(int index = 0; index < settings.Length(); index ++)
        {
            CameraSettingItem *ptr = settings.Get(index);

            cameraInfo.append(QString("  %1 %2      %3 %4\n")
                        .arg(ptr->type)
                        .arg(ptr->name)
                        .arg(ptr->value)
                        .arg(ptr->unit));

            if(ptr->type == "Temperature")
            {
                temperatureJson.insert(ptr->name, ptr->value);
            }
        }

        // fill the json data
        cameraInfoJson.empty();

        cameraInfoJson.insert("serialNumber",                infoData.serialNumber);
        cameraInfoJson.insert("pixelFormat",                 infoData.pixelFormat);
        cameraInfoJson.insert("DefectPixelCorrectionEnable", infoData.DefectPixelCorrectionEnable);
        cameraInfoJson.insert("DF_ColumnOffsetCorrection",   infoData.DF_ColumnOffsetCorrection);
        cameraInfoJson.insert("BF_ColumnGainCorrection",     infoData.BF_ColumnGainCorrection);
        cameraInfoJson.insert("SensitivityMatching",         infoData.SensitivityMatching);
        cameraInfoJson.insert("ExposureMode",                infoData.ExposureMode);
        cameraInfoJson.insert("BlackLevel",                  infoData.BlackLevel);
        cameraInfoJson.insert("ReverseX",                    infoData.ReverseX);

        QJsonObject binningJson;
        binningJson.insert("Horizontal",           infoData.BinningHorizontal);
        binningJson.insert("Vertical",             infoData.BinningVertical);
        cameraInfoJson.insert("Binning", binningJson);

        QJsonObject ImageJson;
        ImageJson.insert("Width",                       infoData.Width);
        ImageJson.insert("Height",                      infoData.Height);
        ImageJson.insert("OffsetX",                     infoData.OffsetX);
        ImageJson.insert("OffsetY",                     infoData.OffsetY);
        cameraInfoJson.insert("Image", ImageJson);

        QJsonObject AcquisitionFramePeriodJson;
        AcquisitionFramePeriodJson.insert("Value",      infoData.AcquisitionFramePeriod);
        AcquisitionFramePeriodJson.insert("ValueRaw",   infoData.AcquisitionFramePeriodRaw);
        cameraInfoJson.insert("AcquisitionFramePeriod", AcquisitionFramePeriodJson);

        QJsonObject exposureTimeJson;
        exposureTimeJson.insert("Value",                infoData.ExposureTime);
        exposureTimeJson.insert("ValueRaw",             infoData.ExposureTimeRaw);
        cameraInfoJson.insert("ExposureTime", exposureTimeJson);

        cameraInfoJson.insert("Temperature", temperatureJson);
    }
}

#ifdef CAMERA_ERROR_RECOVERY
ClassCommon::Error CameraCmvCxp::ErrorRecovery()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // perform and error recovery mechanism
    mGrabber->ErrorRecovery();

    return eError;
}
#endif

void CameraCmvCxp::SetScriptStatement(QString fileName)
{
    CoaXpressGrabber::mScriptFile = fileName;
}

ClassCommon::Error CameraCmvCxp::SetModelName(QString modelName, QString serialNumber)
{
    if(mGrabber != NULL)
    {
        mGrabber->ChangeEepromModelName(modelName, serialNumber);
    }

    return ClassCommon::Error::Ok;
}

#define TMP10X_TEMPERATURE 0x00

float CameraCmvCxp::HW_TMP10XReadTemperature(UCHAR i2c_dev, UCHAR Address)
{
    if(mGrabber != NULL)
    {
        UCHAR  Register = TMP10X_TEMPERATURE;
        USHORT Result   = 0x00;
        USHORT Temperature;

        mGrabber->I2CSelectDevice(i2c_dev,Address) ;
        if ( mGrabber->I2CSendAndReceiveData(&Register, 1, (UCHAR *)&Result, 2))
        {
            ((UCHAR *)&Temperature)[0] = ((UCHAR *)&Result)[1];
            ((UCHAR *)&Temperature)[1] = ((UCHAR *)&Result)[0];
            Temperature = Temperature >> 4;
            if (Temperature & 0x800) { Temperature = Temperature | 0xF000 ; }
            return (Temperature*0.0625);
        }
        else
        {
            return (-255.0f);
        }
    }
    else
    {
        return (-255.0f);
    }
}

//-----------------------------------------------------------------------------
// PCA9536_WriteRegister
//-----------------------------------------------------------------------------
bool CameraCmvCxp::HW_PCA9536_WriteRegister(UCHAR RegAddress, UCHAR ubValue)
{
    if(mGrabber != NULL)
    {
        UCHAR ucSend[2];
        UCHAR ucReceive [4] ;
        ucSend[0] = RegAddress;
        ucSend[1] = ubValue;

        mGrabber->I2CSelectDevice(CurrentI2C_BUS_PCA, I2C_ADDR_PCA9536);
        return mGrabber->I2CSendAndReceiveData(ucSend, 2, ucReceive, 0);
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------
// PCA9536_ReadRegister
//-----------------------------------------------------------------------------
UCHAR  CameraCmvCxp::HW_PCA9536_ReadRegister(UCHAR RegAddress)
{
    UCHAR Register = RegAddress;
    UCHAR ubValue = 0x00;

    if(mGrabber != NULL)
    {
        mGrabber->I2CSelectDevice(CurrentI2C_BUS_PCA, I2C_ADDR_PCA9536) ;
        mGrabber->I2CSendAndReceiveData(&Register, 1, &ubValue, 1);

    }
    return (ubValue);
}

//-----------------------------------------------------------------------------
// PCA9536_I2CSelectI2CBus
//-----------------------------------------------------------------------------
void  CameraCmvCxp::HW_PCA9536_I2CSelectI2CBus (UCHAR ucBusNumber)
{
    CurrentI2C_BUS_PCA = ucBusNumber;
}

#define PCA9536_REG_RW_CONFIG 0x03
#define PCA9536_REG_RW_OUTPUT  0x01

//-----------------------------------------------------------------------------
// PCA9536_ConfigureAsOutput
//-----------------------------------------------------------------------------
void CameraCmvCxp::HW_PCA9536_ConfigureAsOutput (UCHAR ubBit)
{
    UCHAR ubCurrentValue;

    ubCurrentValue = HW_PCA9536_ReadRegister (PCA9536_REG_RW_CONFIG);
    ubCurrentValue = CLEAR (ubCurrentValue,ubBit);

    HW_PCA9536_WriteRegister (PCA9536_REG_RW_CONFIG, ubCurrentValue);
}

//-----------------------------------------------------------------------------
// PCA9536_SetOutputValue
//-----------------------------------------------------------------------------
void  CameraCmvCxp::HW_PCA9536_SetOutputValue (UCHAR ubBit, UCHAR ubValue)
{
    UCHAR ubCurrentValue;

    ubCurrentValue = HW_PCA9536_ReadRegister (PCA9536_REG_RW_OUTPUT);

    if(ubValue)
    {
        ubCurrentValue = SET(ubCurrentValue,ubBit);
    }
    else
    {
        ubCurrentValue = CLEAR(ubCurrentValue,ubBit);
    }

    HW_PCA9536_WriteRegister(PCA9536_REG_RW_OUTPUT,ubCurrentValue);
}

void CameraCmvCxp::HW_ADC128_WriteRegister (UCHAR ubRegAddress, UCHAR ubValue)
{
    UCHAR ubData[2];
    UCHAR ubResult [4];

    ubData [0] = ubRegAddress;
    ubData [1] = ubValue;

    if(mGrabber != NULL)
    {
        mGrabber->I2CSelectDevice (I2C_BUS_TEC_ADC182D818, I2C_ADDR_TEC_ADC182D818);
        mGrabber->I2CSendAndReceiveData(ubData,2,ubResult, 0);
    }

}

float CameraCmvCxp::HW_ADC_TEC_Value(CameraCmvCxp::eDAC_Channel ucChannel, bool inPercent)
{
    if(mGrabber != NULL)
    {
        // ADC128D818 IC on the TEC board

        UCHAR     Address;
        UCHAR     ubResult [4];
        float     sngResult ;

        mGrabber->I2CSelectDevice (I2C_BUS_TEC_ADC182D818, I2C_ADDR_TEC_ADC182D818) ;

        Address = (UCHAR)ucChannel + 0x20 ;
        if (mGrabber->I2CSendAndReceiveData(&Address, 1,ubResult, 2))
        {
            sngResult = ubResult[0] * 256.0f + ubResult[1];
            sngResult = sngResult / (double)16;
            sngResult = sngResult * (2.5 / 4096);

            switch (ucChannel)
            {
                case e_ADC_ZERO:
                    return (sngResult);
                    break;

                case e_DAC_OUTPUT:
                    if (inPercent)
                        return (-100.0f*(sngResult-1.25f)/1.25f) ;
                    else
                        return (sngResult);
                    break;
               case e_ADC_VTEC: // VTEC
                    if (inPercent)
                        return (100.0f*(sngResult-1.25f)/1.25f) ;
                    else
                        return (4.0f * (sngResult - 1.25f));
                   break;

               case e_ADC_ITEC: // ITEC
                    if (inPercent)
                        return (100.0f*(sngResult-1.25f)/1.25f) ;
                    else
                        return ((sngResult - 1.25f) / 0.285f); // RCS = 0.285 V/A
                   break;

               case e_ADC_VIN: // VIN
                   return (sngResult * (56.0f + 4.7f) / 4.7f);
                   break;

               case e_ADC_5V:  // 5V
                   return (sngResult * (56.0f + 30.0f) / 30.0f);
                   break;

               case e_ADC_3V3:  // 3V3
                   return (sngResult * (56.0f + 100.0f) / 100.0f);
                   break;

               case e_ADC_Temperature: // Temperature : special case
                    unsigned short sTemp = (((unsigned short)ubResult[0] * 256 + ubResult[1])) & 0x1FF ;
                    if (sTemp & 0x100) { // neg
                      return ((512- sTemp)/2.0f) ;
                    }
                    else {
                      return ((sTemp)/4.0f) ;
                    }
                    break;
           }
            return (-255.0f);
        }
    }
    return (-255.0f);
}

//-----------------------------------------------------------------------------
// HW_SetTriggerGain
// 3 available gains
//    0 --> x1
//    1 --> x10
//    2 --> x50
//-----------------------------------------------------------------------------
void  CameraCmvCxp::HW_SetTriggerGain (UCHAR ubGain)
{
    HW_PCA9536_I2CSelectI2CBus (3) ;

    if (ubGain == 0) {
        // x1
        HW_PCA9536_SetOutputValue (BIT0,1) ;
        HW_PCA9536_SetOutputValue (BIT1,0) ;
    }
    else {
        if (ubGain == 1) {
            // x10
            HW_PCA9536_SetOutputValue (BIT0,0) ;
            HW_PCA9536_SetOutputValue (BIT1,0) ;
        }
        else {
            // x50
            HW_PCA9536_SetOutputValue (BIT0,1) ;
            HW_PCA9536_SetOutputValue (BIT1,1) ;
        }
    }
}

//-----------------------------------------------------------------------------
// HW_Get_TEC_Hot_Temperature
//-----------------------------------------------------------------------------
float CameraCmvCxp::HW_Get_TEC_Hot_Temperature()
{
    float fResult = HW_TMP10XReadTemperature(I2C_BUS_TEC_TMP102, I2C_ADDR_TEC_TMP102);
    return (fResult);
}

//-----------------------------------------------------------------------------
// HW_Get_Sensor_Cold_Temperature
//-----------------------------------------------------------------------------
float CameraCmvCxp::HW_Get_Sensor_Cold_Temperature()
{
    float fResult = HW_TMP10XReadTemperature(I2C_BUS_SENSOR_TMP102, I2C_ADDR_SENSOR_TMP102);
    return (fResult);
}

#define DAC8571_WRITE_DATA_LOAD       0x10
#define DAC8571_UPDATE        0x20
#define DAC8571_WRITE_PWDC   0x01
#define DAC8571_UPDATE        0x20

void CameraCmvCxp::HW_DAC8571_WriteData(USHORT usValue)
{
    if(mGrabber != NULL)
    {
        UCHAR ucSend[4];
        UCHAR ucReceive [4] ;

        mGrabber->I2CSelectDevice(I2C_BUS_TEC_DAC8571,I2C_ADDR_TEC_DAC8571);

        // First Set Value
        ucSend[0] = DAC8571_WRITE_DATA_LOAD;
        ucSend[1] = HIGH_BYTE(usValue);
        ucSend[2] = LOW_BYTE(usValue);
        mGrabber->I2CSendAndReceiveData(ucSend, 3, ucReceive, 0);

        // In a second Time Update
        ucSend[0] = DAC8571_UPDATE;
        ucSend[1] = 0;
        ucSend[2] = 0;
        mGrabber->I2CSendAndReceiveData(ucSend, 3, ucReceive, 0);
    }
}

void CameraCmvCxp::HW_DAC8571_WritePWDSettings( UCHAR ubSettings)
{
    if(mGrabber != NULL)
    {
        UCHAR ucSend[4];
        UCHAR ucReceive [4];

        mGrabber->I2CSelectDevice(I2C_BUS_TEC_DAC8571,I2C_ADDR_TEC_DAC8571);

        // First Set Value
        ucSend[0] = DAC8571_WRITE_PWDC;
        ucSend[1] = 0;
        ucSend[2] = ubSettings;
        mGrabber->I2CSendAndReceiveData(ucSend, 3, ucReceive, 0);

        // In a second Time Update
        ucSend[0] = DAC8571_UPDATE;
        ucSend[1] = 0;
        ucSend[2] = 0;
        mGrabber->I2CSendAndReceiveData(ucSend, 3, ucReceive, 0);
    }
}

//-----------------------------------------------------------------------------
// HW_Disable_TEC
//-----------------------------------------------------------------------------
void CameraCmvCxp::HW_Disable_TEC()
{
    HW_PCA9536_I2CSelectI2CBus (I2C_BUS_TEC_PCA9536) ;
    HW_PCA9536_SetOutputValue (BIT0,0) ;
    HW_DAC8571_WriteData (32767) ; // ask 0 current to the ADN8835 IC
}

//-----------------------------------------------------------------------------
// HW_Enable_TEC
//-----------------------------------------------------------------------------
void CameraCmvCxp::HW_Enable_TEC()
{
    HW_PCA9536_I2CSelectI2CBus (I2C_BUS_TEC_PCA9536) ;
    HW_PCA9536_SetOutputValue (BIT0,1) ;
    HW_DAC8571_WriteData (32767) ; // ask 0 current to the ADN8835 IC
}

#define MAX_DAC_VALUE 65535.0f
#define MID_DAC_VALUE 32767.0f

//-----------------------------------------------------------------------------
// HW_Set_DAC_TEC_OutputValue
// -1.0 <= fValue <= 1.0
//-----------------------------------------------------------------------------
void  CameraCmvCxp::HW_Set_DAC_TEC_OutputValue (float fValue)
{
    float fDACValue;

    fDACValue = MID_DAC_VALUE - fValue * MID_DAC_VALUE ;

    if (fDACValue > MAX_DAC_VALUE)
    {
        fDACValue = MAX_DAC_VALUE;
    }
    else if(fDACValue < 0)
    {
        fDACValue = 0.0f;
    }

    // update DACValue
    HW_DAC8571_WriteData ((unsigned short)fDACValue) ;
}

float CameraCmvCxp::HW_Get_CMOSTemperature()
{
    SetStringValue("DeviceTemperatureSelector", "Sensor");
    QThread::msleep(10);
    return (GetFloatValue("DeviceTemperature"));
}

float CameraCmvCxp::HW_Get_MainboardTemperature()
{
    SetStringValue("DeviceTemperatureSelector", "Mainboard");
    QThread::msleep(10);
    return (GetFloatValue("DeviceTemperature"));
}

float CameraCmvCxp::HW_Get_CPUTemperature()
{
     SetStringValue("DeviceTemperatureSelector", "CPU");
    QThread::msleep(10);
    return (GetFloatValue("DeviceTemperature"));
}

#define FAN_MAX_RPM 6850.0 // Datasheet 9GA0612P6G001

// Return a value between 0 and 100.0f
float CameraCmvCxp::HW_Get_FanSpeed (int intFanSelect)
{
    float fSemiPeriod ;
    float fRPMValue ;

    if (intFanSelect > 1)
    {
        intFanSelect = 1;
    }
    else if (intFanSelect < 0)
    {
        intFanSelect = 0;
    }

    SetIntegerValue ("FanSelect",intFanSelect);

    fSemiPeriod = (float)GetIntegerValue ("FanTach"); // in us

    if (fSemiPeriod < 65535)
    {
        fRPMValue = 60*1000000/(fSemiPeriod*2);
    }
    else
    {
        fRPMValue = 0;
    }

    return (100*fRPMValue / FAN_MAX_RPM);
}

// Set a value between 0 and 100.0f
void CameraCmvCxp::HW_Set_FanSpeed (int intFanSelect, float fValueInPercent)
{
    if (intFanSelect > 2) intFanSelect = 2;
    if (intFanSelect < 0) intFanSelect = 0;

    if (fValueInPercent < 0) fValueInPercent = 0;
    if (fValueInPercent > 100.0f) fValueInPercent = 100.0f;

    if (intFanSelect < 2)
    {
        SetIntegerValue ("FanSelect",intFanSelect) ;
        SetIntegerValue ("FanPeriod",100) ;
        SetIntegerValue ("FanWidth",(int)fValueInPercent) ;
    }
    else
    {
        SetIntegerValue ("FanSelect",0);
        SetIntegerValue ("FanPeriod",100);
        SetIntegerValue ("FanWidth",(int)fValueInPercent);
        SetIntegerValue ("FanSelect",1);
        SetIntegerValue ("FanPeriod",100);
        SetIntegerValue ("FanWidth",(int)fValueInPercent);
    }
}

bool CameraCmvCxp::HW_I2CIsDeviceReady(unsigned char i2c_dev, unsigned char Address)
{
    unsigned char  ucRX = 0;
    unsigned char  ucTX = 0;

    if(mGrabber != NULL)
    {
        mGrabber->I2CSelectDevice(i2c_dev,Address);

        bool status = mGrabber->I2CSendAndReceiveData(&ucTX, 1, &ucRX, 1);
        return status;
    }
    return false;

}

void CameraCmvCxp::HW_I2CSelectDevice(unsigned char ucBUS, unsigned char ucAddress)
{
    if(mGrabber != NULL)
    {
        mGrabber->I2CSelectDevice(ucBUS,ucAddress);
    }
}

bool CameraCmvCxp::HW_I2CSendAndReceiveData(const unsigned char *TXBuffer, unsigned char TXCount, unsigned char *RXBuffer, unsigned char RXCount)
{
    bool returnVal = false;

    if(mGrabber != NULL)
    {
        returnVal = (mGrabber->I2CSendAndReceiveData(TXBuffer, TXCount, RXBuffer, RXCount));
    }

    return returnVal;
}

QString CameraCmvCxp::GetStringValue (const std::string feature)
{
    try
    {
        return(QString::fromStdString(mGrabber->getString<Euresys::RemoteModule> (feature)));
    }
    catch(...)
    {
        return("Invalid!");
    }
}

void  CameraCmvCxp::SetStringValue(const std::string feature, QString value)
{
    try
    {
        mGrabber->setString<Euresys::RemoteModule> (feature, value.toStdString());
    }
    catch(...)
    {
    }
}

int CameraCmvCxp::GetIntegerValue (const std::string feature)
{
    try
    {
        return ( mGrabber->getInteger<Euresys::RemoteModule> (feature));
    }
    catch(...)
    {
        return(0);
    }
}

void  CameraCmvCxp::SetIntegerValue(const std::string feature, int value)
{
    try
    {
        mGrabber->setInteger<Euresys::RemoteModule> (feature, value);
    }
    catch(...)
    {
    }
}

float CameraCmvCxp::GetFloatValue (const std::string feature)
{
    try
    {
        return ( mGrabber->getFloat<Euresys::RemoteModule> (feature));
    }
    catch(...)
    {
        return (0)   ;
    }
}

void  CameraCmvCxp::SetFloatValue(const std::string feature, float value)
{
    try
    {
        mGrabber->setFloat<Euresys::RemoteModule> (feature, value);
    }
    catch(...)
    {

    }
}

float CameraCmvCxp::GetHw(QString hwDeviceName, QString hwFeature)
{
    float value = 0.0;

    // better is following is in a map
    if(hwDeviceName == "TEC")
    {
        if(hwFeature == "HotTemperature")
        {
            value = HW_Get_TEC_Hot_Temperature();
        }
        else if(hwFeature == "AdcITec")
        {
            value = HW_ADC_TEC_Value(CameraCmvCxp::e_ADC_ITEC);
        }
        else if(hwFeature == "VTec")
        {
            value = HW_ADC_TEC_Value(CameraCmvCxp::e_ADC_VTEC);
        }
        else if(hwFeature == "DacOutput")
        {
            value = HW_ADC_TEC_Value(CameraCmvCxp::e_DAC_OUTPUT, true);
        }
        else if(hwFeature == "AdcVin")
        {
            value = HW_ADC_TEC_Value(CameraCmvCxp::e_ADC_VIN);
        }
        else if(hwFeature == "Adc5V")
        {
            value = HW_ADC_TEC_Value(CameraCmvCxp::e_ADC_5V);
        }
        else if(hwFeature == "Adc3V3")
        {
            value = HW_ADC_TEC_Value(CameraCmvCxp::e_ADC_3V3);
        }
    }
    else if(hwDeviceName == "Sensor")
    {
        if(hwFeature == "ColdTemperature")
        {
            value = HW_Get_Sensor_Cold_Temperature() ;
        }
        else if(hwFeature == "CmosTemperature")
        {
            value = HW_Get_CMOSTemperature() ;
        }
    }
    return value;
}

void CameraCmvCxp::SetHw(QString hwDeviceName, QString hwFeature, float value, int index)
{
    if(hwDeviceName == "Fan")
    {
        if(hwFeature == "Speed")
        {
            HW_Set_FanSpeed(index, value) ;
        }
    }
}
