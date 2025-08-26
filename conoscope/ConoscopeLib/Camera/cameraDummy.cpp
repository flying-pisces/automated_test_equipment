#include "cameraDummy.h"

#include <QFuture>
#include <QtConcurrent/qtconcurrentrun.h>

#include "HwTool.h"
// #include "CameraDummyHw.h"
#include "cameraCmvCxpHw.h"

#include "imageConfiguration.h"
#include "ConoscopeResource.h"

#define PRINT

#define INVALID_FRAME_INDEX -1

CameraDummy::CameraDummy(QObject *parent) : Camera(parent)
{
    eState = CameraState_NotConnected;

    mCaptureState = Camera::Status::NotInitialised;
    mCurrentFrameIndex = INVALID_FRAME_INDEX;

    // following may not change
    mFwType.insert("FPGA", CoaXpressGrabber::eFirmwareType::FPGA);
    mFwType.insert("NIOS", CoaXpressGrabber::eFirmwareType::NIOS);
}

CameraDummy::~CameraDummy()
{
}

ClassCommon::Error CameraDummy::LoadRawImage(QString path, QMap<QString, QVariant>& settings)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mDummyRawImagePath = path;

    eError = _ReadImageFile(
                mDummyRawImagePath,
                mRawData,
                mRawDataInfo);

    if(eError == ClassCommon::Error::Ok)
    {
        mHwValueMap["HotTemperature"]  = mRawDataInfo.setupSensorTemperature;
        mHwValueMap["ColdTemperature"] = mRawDataInfo.setupSensorTemperature;
        mHwValueMap["CmosTemperature"] = mRawDataInfo.setupSensorTemperature;

        mHwValueMap["AdcITec"] = 2;

        // output the data read in the json file associated to the capture
        settings["exposureTimeUs"] = mRawDataInfo.measureExposureTimeUs;
        settings["nbAcquisition"]  = mRawDataInfo.measureNbAcquisition;
        settings["binningFactor"]  = mRawDataInfo.measureBinningFactor;
        settings["bTestPattern"]   = mRawDataInfo.measureTestPattern;

        settings["setupFilter"]            = mRawDataInfo.setupFilter;
        settings["setupIris"]              = mRawDataInfo.setupIris;
        settings["setupNd"]                = mRawDataInfo.setupNd;
        settings["setupSensorTemperature"] = mRawDataInfo.setupSensorTemperature;

        settings["timeStampString"] = mRawDataInfo.timeStampString;

        settings["timeStampDate"] = mRawDataInfo.infoDate;
        settings["timeStampTime"] = mRawDataInfo.infoTime;
    }

    return eError;
}

bool CameraDummy::IsConnected()
{
    return(eState == CameraState_Connected) ? true : false;
}

bool CameraDummy::IsFileTransferSupported()
{
    return false;
}

void CameraDummy::FileTransferOpen()
{
}

void CameraDummy::FileTransferClose()
{
}

void CameraDummy::FileTransferWrite()
{
}

void CameraDummy::FileTransferRead()
{
}

void CameraDummy::FileTransferWrite(const void*, size_t)
{
}

void CameraDummy::FileTransferRead(void*, size_t)
{
}

int CameraDummy::FwUpdateConfigure(size_t)
{
    return 0;
}

ClassCommon::Error CameraDummy::FwUpdate(QString, char*, size_t)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;
    return eError;
}

ClassCommon::Error CameraDummy::Connect()
{
    return _Connect(false);
}

ClassCommon::Error CameraDummy::ConnectPowerCycle()
{
    return _Connect(true);
}

ClassCommon::Error CameraDummy::_Connect(bool)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mModel = CameraModel_CmvCxp_50k;

    _Disconnect();

    try
    {
        // configure image size
        CoaXpressGrabber_CameraInfo cameraInfo;
        // TODO fill camera info
        mModel = CameraModel_CmvCxp_50k;

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


        eState = CameraState_Connected;
        mCaptureState = Camera::Status::Ready;

        NotifyEvent(Event::Connect);
    }
    catch(gentl_error gentlException)
    {
        // QString expString = QString("CameraDummy:  %1").arg(QString::fromStdString(gentlException.exception.what()));

        eError = ClassCommon::Error::Failed;
    }

    return eError;
}

ClassCommon::Error CameraDummy::_Disconnect()
{
    mCaptureState = Camera::Status::NotInitialised;
    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::Disconnect()
{
    ClassCommon::Error eError = _Disconnect();

    eState = CameraState_NotConnected;
    NotifyEvent(Event::Disconnect);

    return eError;
}

ClassCommon::Error CameraDummy::SetDefaultConfiguration()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    return eError;
}

ClassCommon::Error CameraDummy::GetInfo(CameraInfo_t& info)
{
    info.CpuBoardRev       = "DummyCamera";
    info.SensorBoardRev    = "DummyCamera";
    info.CameraRev         = "DummyCamera";
    info.SoftwareVersion   = "DummyCamera";
    info.ModelName         = "DummyCamera";
    info.VendorName        = "DummyCamera";

    info.cameraSerialNumber      = mRawDataInfo.cameraSerialNumber;
    info.cameraBoardSerialNumber = mRawDataInfo.cameraSerialNumber;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::CheckConnection()
{
    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::GetInterfaceInfo(Camera::InterfaceInfo_t& interfaceInfo)
{
    CoaXpressGrabber_InterfaceInfo info;

    // TODO
    interfaceInfo.version = info.version;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::GetSerialNumber(
        QString& cameraSerialNumber,
        QString& cameraBoardSerialNumber)
{
    cameraSerialNumber = mRawDataInfo.cameraSerialNumber;
    cameraBoardSerialNumber = mRawDataInfo.cameraSerialNumber;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::GetSettings(CameraSettings_t& settings)
{
    CoaXpressGrabber_CameraSettings cameraSettings;

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

    return ClassCommon::Error::Ok;
}

void AsyncNotifyEvent(CameraDummy* m_Camera, int eventIndex)
{
    emit m_Camera->EventOccured(eventIndex);
}

void CameraDummy::NotifyEvent(Event eEvent)
{
    // execute the action asynchronously
    // else, in some cases, a key is not properly captured
    // and is processed twice
    QtConcurrent::run(AsyncNotifyEvent, this, (int)eEvent);
}

ClassCommon::Error CameraDummy::Configure(struct CaptureConfig* pConfig)
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
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

int CameraDummy::GetCurrentCaptureTimeoutSteps(int)
{
    return 10;
}

ClassCommon::Error CameraDummy::ConfigurePipeline(struct PipelineConfig*){
    // no implemented in the current version of the camera
    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::GetStatus(enum Status& eStatus)
{
    eStatus = mCaptureState;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::StartMeasurement()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    return eError;
}

ClassCommon::Error CameraDummy::StopMeasurement()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    return eError;
}

ClassCommon::Error CameraDummy::GetPulse(struct Pulse*){
    return ClassCommon::Error::Ok;
}

ClassCommon::Error CameraDummy::GetRawData(struct RawDataInfo&, QByteArray& arr, QByteArray&)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    arr = mRawData;

    return eError;
}

#ifndef COAXPRESS_FRAME_AVERAGE
void CameraDummy::UpdateCaptureConfiguration(
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
void CameraDummy::SetPrnu(ns1__prnu *data, struct ns1__setPRNUResponse &_param_1)
{
}

void CameraDummy::SetSensorDefects(ns1__sensorDefects *defects, struct ns1__setSensorDefectsResponse &_param_1)
{
}

void CameraDummy::SetSensorSaturation(ns1__sensorSaturation *value, struct ns1__setSensorSaturationResponse &_param_1)
{
}

void CameraDummy::SetPartInformation(ns1__part *partIndex, ns1__partInformation *partInformation, struct ns1__setPartInformationResponse &_param_1)
{
}

void CameraDummy::StoreConfigurationFile(struct ns1__storeConfigurationFileResponse &_param_1)
{
}
#endif

void CameraDummy::GetCameraInfo(
        QString     &cameraInfo,
        QJsonObject &cameraInfoJson)
{
    CoaXpressGrabber::GetInfo_t infoData;

    CameraSettings settings;

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

#ifdef CAMERA_ERROR_RECOVERY
ClassCommon::Error CameraDummy::ErrorRecovery()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // perform and error recovery mechanism
    mGrabber->ErrorRecovery();

    return eError;
}
#endif

void CameraDummy::SetScriptStatement(QString fileName)
{
    CoaXpressGrabber::mScriptFile = fileName;
}

ClassCommon::Error CameraDummy::SetModelName(QString modelName, QString serialNumber)
{
    return ClassCommon::Error::Ok;
}

#define TMP10X_TEMPERATURE 0x00

float CameraDummy::HW_TMP10XReadTemperature(UCHAR, UCHAR)
{
    return 0.0f;
}

//-----------------------------------------------------------------------------
// PCA9536_WriteRegister
//-----------------------------------------------------------------------------
bool CameraDummy::HW_PCA9536_WriteRegister(UCHAR, UCHAR)
{
    return true;
}

//-----------------------------------------------------------------------------
// PCA9536_ReadRegister
//-----------------------------------------------------------------------------
UCHAR  CameraDummy::HW_PCA9536_ReadRegister(UCHAR)
{
    return 0;
}

//-----------------------------------------------------------------------------
// PCA9536_I2CSelectI2CBus
//-----------------------------------------------------------------------------
void  CameraDummy::HW_PCA9536_I2CSelectI2CBus (UCHAR ucBusNumber)
{
    CurrentI2C_BUS_PCA = ucBusNumber;
}

#define PCA9536_REG_RW_CONFIG 0x03
#define PCA9536_REG_RW_OUTPUT  0x01

//-----------------------------------------------------------------------------
// PCA9536_ConfigureAsOutput
//-----------------------------------------------------------------------------
void CameraDummy::HW_PCA9536_ConfigureAsOutput (UCHAR ubBit)
{
    UCHAR ubCurrentValue;

    ubCurrentValue = HW_PCA9536_ReadRegister (PCA9536_REG_RW_CONFIG);
    ubCurrentValue = CLEAR (ubCurrentValue,ubBit);

    HW_PCA9536_WriteRegister (PCA9536_REG_RW_CONFIG, ubCurrentValue);
}

//-----------------------------------------------------------------------------
// PCA9536_SetOutputValue
//-----------------------------------------------------------------------------
void  CameraDummy::HW_PCA9536_SetOutputValue (UCHAR ubBit, UCHAR ubValue)
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

void CameraDummy::HW_ADC128_WriteRegister (UCHAR ubRegAddress, UCHAR ubValue)
{
}

float CameraDummy::HW_ADC_TEC_Value(CameraDummy::eDAC_Channel, bool)
{
    return (-255.0f);
}

//-----------------------------------------------------------------------------
// HW_SetTriggerGain
// 3 available gains
//    0 --> x1
//    1 --> x10
//    2 --> x50
//-----------------------------------------------------------------------------
void  CameraDummy::HW_SetTriggerGain (UCHAR)
{
}

//-----------------------------------------------------------------------------
// HW_Get_TEC_Hot_Temperature
//-----------------------------------------------------------------------------
float CameraDummy::HW_Get_TEC_Hot_Temperature()
{
    return 0.0F;
}

//-----------------------------------------------------------------------------
// HW_Get_Sensor_Cold_Temperature
//-----------------------------------------------------------------------------
float CameraDummy::HW_Get_Sensor_Cold_Temperature()
{
    return 0.0F;
}

#define DAC8571_WRITE_DATA_LOAD       0x10
#define DAC8571_UPDATE        0x20
#define DAC8571_WRITE_PWDC   0x01
#define DAC8571_UPDATE        0x20

void CameraDummy::HW_DAC8571_WriteData(USHORT)
{
}

void CameraDummy::HW_DAC8571_WritePWDSettings(UCHAR)
{
}

//-----------------------------------------------------------------------------
// HW_Disable_TEC
//-----------------------------------------------------------------------------
void CameraDummy::HW_Disable_TEC()
{
}

//-----------------------------------------------------------------------------
// HW_Enable_TEC
//-----------------------------------------------------------------------------
void CameraDummy::HW_Enable_TEC()
{
}

#define MAX_DAC_VALUE 65535.0f
#define MID_DAC_VALUE 32767.0f

//-----------------------------------------------------------------------------
// HW_Set_DAC_TEC_OutputValue
// -1.0 <= fValue <= 1.0
//-----------------------------------------------------------------------------
void  CameraDummy::HW_Set_DAC_TEC_OutputValue(float)
{
}

float CameraDummy::HW_Get_CMOSTemperature()
{
    return 0.0F;
}

float CameraDummy::HW_Get_MainboardTemperature()
{
    return 0.0F;
}

float CameraDummy::HW_Get_CPUTemperature()
{
    return 0.0F;
}

#define FAN_MAX_RPM 6850.0 // Datasheet 9GA0612P6G001

// Return a value between 0 and 100.0f
float CameraDummy::HW_Get_FanSpeed (int intFanSelect)
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
void CameraDummy::HW_Set_FanSpeed (int intFanSelect, float fValueInPercent)
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

bool CameraDummy::HW_I2CIsDeviceReady(unsigned char, unsigned char)
{
    return true;
}

void CameraDummy::HW_I2CSelectDevice(unsigned char, unsigned char)
{
}

bool CameraDummy::HW_I2CSendAndReceiveData(const unsigned char*, unsigned char, unsigned char*, unsigned char)
{
    return true;
}

QString CameraDummy::GetStringValue (const std::string feature)
{
        return("Invalid!");
}

void  CameraDummy::SetStringValue(const std::string feature, QString value)
{
}

int CameraDummy::GetIntegerValue (const std::string feature)
{
    return(0);
}

void  CameraDummy::SetIntegerValue(const std::string, int)
{
}

float CameraDummy::GetFloatValue (const std::string feature)
{
    return mFloatValueMap[CONVERT_TO_QSTRING(feature)];
}

void  CameraDummy::SetFloatValue(const std::string feature, float value)
{
    mFloatValueMap[CONVERT_TO_QSTRING(feature)] = value;

    if(feature.compare("PIDTarget") == 0)
    {
        mHwValueMap["HotTemperature"]  = value;
        mHwValueMap["ColdTemperature"] = value;
        mHwValueMap["CmosTemperature"] = value;

        mFloatValueMap["PIDTemp"] = value;
    }

}

float CameraDummy::GetHw(QString hwDeviceName, QString hwFeature)
{
    float value = 0.0;

    value = mHwValueMap[hwFeature];

    return value;
}

void CameraDummy::SetHw(QString hwDeviceName, QString hwFeature, float value, int index)
{
    if(hwDeviceName == "Fan")
    {
        if(hwFeature == "Speed")
        {
            HW_Set_FanSpeed(index, value) ;
        }
    }
}

#include <QFileInfo>

#define IMAGE_INFO_EXTENSION ".json"

ClassCommon::Error CameraDummy::_ReadImageFile(
        QString acFilename,
        QByteArray& imgData,
        ImageInfoRead_t& info)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    QFile ff(QString("%1").arg(acFilename));

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

    return eError;
}

#include <QJsonObject>
#include <QJsonDocument>

ClassCommon::Error CameraDummy::_ReadImageInfo(QString filePath, ImageInfoRead_t &info)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // should check is file exists
    if(!QFile(filePath).exists())
    {
        eError = ClassCommon::Error::Failed;
    }

    if(eError == ClassCommon::Error::Ok)
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
            // QJsonArray jsonArray = jsonObject["properties"].toArray();

            QJsonObject cameraObject      = jsonObject["Camera"].toObject();
            QJsonObject infoObject        = jsonObject["Info"].toObject();
            QJsonObject measureObject     = jsonObject["Measure"].toObject();
            QJsonObject setupObject       = jsonObject["Setup"].toObject();
            QJsonObject softwareObject    = jsonObject["Software"].toObject();

            info.cameraHeight = cameraObject["Height"].toInt();
            info.cameraWidth = cameraObject["Width"].toInt();
            info.cameraSerialNumber = cameraObject["SerialNumber"].toString();

            info.infoDate = infoObject["Date"].toString();
            info.infoTime = infoObject["Time"].toString();
            info.infoFile = infoObject["File"].toString();
            info.infoTemperature = infoObject["Temperature"].toDouble();

            info.measureBinningFactor = measureObject["BinningFactor"].toInt();
            info.measureExposureTimeUs = measureObject["ExposureTimeUs"].toInt();
            info.measureNbAcquisition = measureObject["NbAcquisition"].toInt();
            info.measureTestPattern = measureObject["TestPattern"].toBool();

#ifdef STORE_INDEX
            info.setupFilter = setupObject["filter"].toInt();
            info.setupIris = setupObject["iris"].toInt();
            info.setupNd = setupObject["nd"].toInt();
#else
            info.setupFilter = RESOURCE->Convert(ConoscopeResource::ResourceType_Filter, setupObject["filter"].toString());
            info.setupIris   = RESOURCE->Convert(ConoscopeResource::ResourceType_Iris, setupObject["iris"].toString());
            info.setupNd     = RESOURCE->Convert(ConoscopeResource::ResourceType_Nd, setupObject["nd"].toString());

            if((info.setupFilter == -1) || (info.setupIris == -1) || (info.setupNd == -1))
            {
                // backward compatibility
                info.setupFilter = setupObject["filter"].toInt();
                info.setupIris   = setupObject["iris"].toInt();
                info.setupNd     = setupObject["nd"].toInt();
            }
#endif
            info.setupSensorTemperature = setupObject["sensorTemperature"].toDouble();

            info.softwareSwDate = softwareObject["SwDate"].toString();
            info.softwareSwVersion = softwareObject["SwVersion"].toString();
        }
    }

    return eError;
}
