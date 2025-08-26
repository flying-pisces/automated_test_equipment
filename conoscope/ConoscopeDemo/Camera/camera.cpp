#include "camera.h"

Camera::Camera(QObject *parent) : ClassCommon(parent)
{
    mModel = CameraModel_Unknown;
}

Camera::~Camera()
{
}

ClassCommon::Error Camera::Connect(){return ClassCommon::Error::Ok;}

ClassCommon::Error Camera::ConnectPowerCycle(){return ClassCommon::Error::Ok;}

ClassCommon::Error Camera::Disconnect(){return ClassCommon::Error::Ok;}

ClassCommon::Error Camera::SetDefaultConfiguration(){return ClassCommon::Error::Ok;}

bool Camera::IsConnected(){return true;}

bool Camera::IsFileTransferSupported(){return false;}

void Camera::FileTransferOpen(){}

void Camera::FileTransferClose(){}

void Camera::FileTransferWrite(){}

void Camera::FileTransferRead(){}

void Camera::FileTransferWrite(const void *data, size_t size){}

void Camera::FileTransferRead(void *data, size_t size){}

ClassCommon::Error Camera::Register(QString cameraSerialNumber, QString cameraBoardSerialNumber)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // this is hard coded
    QStringList snList = cameraBoardSerialNumber.split("_");
    if(snList.count() == 0)
    {
        mInfo.cameraBoardSerialNumber = cameraBoardSerialNumber;
    }
    else if(snList.count() == 2)
    {
        mInfo.VendorName = snList[0];
        mInfo.cameraBoardSerialNumber = snList[1];
    }
    else
    {
        eError = ClassCommon::Error::InvalidParameter;
    }

    mInfo.cameraSerialNumber = cameraSerialNumber;

    return eError;
}

ClassCommon::Error Camera::ConfigureConnection(
    QString ipAddr,
    QString port,
    QString connectionConfig)
{
    mIpAddr = ipAddr;
    mPort = port;
    mConnectionConfig = connectionConfig;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::GetInfo(CameraInfo_t& info)
{
    info.cameraBoardSerialNumber = mInfo.cameraBoardSerialNumber;
    info.cameraSerialNumber = mInfo.cameraSerialNumber;

    info.VendorName = mInfo.VendorName;

    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::GetInterfaceInfo(Camera::InterfaceInfo_t& interfaceInfo)
{
    return ClassCommon::Error::Ok;
}

// this is the serial number of the camera
// it may differ from the serial number of the camera device
ClassCommon::Error Camera::GetSerialNumber(QString& cameraSerialNumber, QString &cameraBoardSerialNumber)
{
    return ClassCommon::Error::NotImplemented;
}

ClassCommon::Error Camera::GetSettings(CameraSettings_t& settings)
{
    return ClassCommon::Error::NotImplemented;
}

ClassCommon::Error Camera::Configure(struct CaptureConfig* pConfig)
{
    return ClassCommon::Error::Ok;
}

int Camera::GetCurrentCaptureTimeoutSteps(int stepMs)
{
    return -1;
}

ClassCommon::Error Camera::ConfigurePipeline(struct PipelineConfig *pConfig)
{
    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::StartMeasurement()
{
    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::StopMeasurement()
{
    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::GetPulse(struct Pulse* pPulse)
{
    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::GetStatus(enum Status& eStatus)
{
    return ClassCommon::Error::Ok;
}

ClassCommon::Error Camera::GetRawData(struct RawDataInfo &info, QByteArray& arr, QByteArray &stdDev)
{
    return ClassCommon::Error::Ok;
}

#ifdef SOAP_INTERFACE
void Camera::SetPrnu(ns1__prnu *data, struct ns1__setPRNUResponse &_param_1)
{}

void Camera::SetSensorDefects(ns1__sensorDefects *defects, struct ns1__setSensorDefectsResponse &_param_1)
{}

void Camera::SetSensorSaturation(ns1__sensorSaturation *value, struct ns1__setSensorSaturationResponse &_param_1)
{}

void Camera::SetPartInformation(ns1__part *partIndex, ns1__partInformation *partInformation, struct ns1__setPartInformationResponse &_param_1)
{}

void Camera::StoreConfigurationFile(struct ns1__storeConfigurationFileResponse &_param_1)
{}
#endif

void Camera::UpdateCaptureConfiguration(
        int& numberReads,
        int& numberCaptures)
{
    // by default not modifications of the parameters are done
}

void Camera::GetCameraInfo(
        QString     &cameraInfo,
        QJsonObject &cameraInfoJson)
{
    cameraInfo = "NA";
    cameraInfoJson.insert("CameraInfo", "NA");
}

ClassCommon::Error Camera::ErrorRecovery()
{
    return ClassCommon::Error::NotImplemented;
}

ClassCommon::Error Camera::SetModelName(QString modelName, QString serialNumber)
{
    return ClassCommon::Error::NotImplemented;
}

ClassCommon::Error Camera::FwUpdate(QString type, char* firmware, size_t firmwareSize)
{
    return ClassCommon::Error::NotImplemented;
}

int Camera::FwUpdateConfigure(size_t size)
{
    return 0;
}

// TEC specific
#ifdef TEC_INTERFACE
float HW_ADC_TEC_Value(Camera::eDAC_Channel ucChannel, bool inPercent)
{
    return 0.0;
}

float HW_TMP10XReadTemperature(UCHAR i2c_dev, UCHAR Address)
{
    return 0.0;
}

void HW_DAC8571_WriteData(USHORT usValue)
{

}

bool HW_PCA9536_WriteRegister(UCHAR RegAddress, UCHAR ubValue)
{
    return false;
}

UCHAR HW_PCA9536_ReadRegister(UCHAR RegAddress)
{
    return 0;
}

void HW_PCA9536_I2CSelectI2CBus(UCHAR ucBusNumber)
{

}

void HW_PCA9536_SetOutputValue(UCHAR ubBit, UCHAR ubValue)
{

}

void HW_SetTriggerGain(UCHAR ubGain)
{

}

float HW_Get_TEC_Hot_Temperature()
{
    return 0.0;
}

float HW_Get_Sensor_Cold_Temperature()
{
    return 0.0;
}

void HW_Disable_TEC()
{

}

void HW_Enable_TEC()
{

}

void HW_Set_DAC_TEC_OutputValue(float fValue)
{

}

float HW_Get_CMOSTemperature()
{
    return 0.0;
}

float HW_Get_MainboardTemperature()
{
    return 0.0;
}

float HW_Get_CPUTemperature()
{
    return 0.0;
}

float HW_Get_FanSpeed(int intFanSelect)
{
    return 0.0;
}

void HW_Set_FanSpeed(int intFanSelect, float fValueInPercent)
{

}
#endif

#ifdef TEC_INTERFACE
void Camera::HW_DAC8571_WritePWDSettings(UCHAR ubSettings)
{

}

void Camera::HW_ADC128_WriteRegister(UCHAR ubRegAddress, UCHAR ubValue)
{

}

void Camera::HW_PCA9536_ConfigureAsOutput(UCHAR ubBit)
{

}
#endif

QString Camera::GetStringValue(const std::string feature)
{
    return "...";
}

void Camera::SetStringValue(const std::string feature, QString value)
{
    return;
}

int Camera::GetIntegerValue(const std::string feature)
{
    return 0;
}

void Camera::SetIntegerValue(const std::string feature, int value)
{
    return;
}

float Camera::GetFloatValue(const std::string feature)
{
    float value = 0.0f;

    if(feature == "PIDTarget")
    {
        value = mDummySensorTarget;
    }

    return value;
}

void Camera::SetFloatValue(const std::string feature, float value)
{
    if(feature == "PIDTarget")
    {
        mDummySensorTarget = value;
        mDummySensorTargetStep = 10;
    }

    return;
}

float Camera::GetHw(QString hwDeviceName, QString hwFeature)
{
    float value = 0.0f;

    if(hwDeviceName == "Sensor")
    {
        if(hwFeature == "ColdTemperature")
        {
            value = mDummySensorTarget - mDummySensorTargetStep * 0.1;
            if(value != mDummySensorTarget)
            {
                mDummySensorTargetStep --;
            }
        }
        else if(hwFeature == "CmosTemperature")
        {
            value = mDummySensorTarget;
        }
    }

    return value;
}

void Camera::SetHw(QString hwDeviceName, QString hwFeature, float value, int index)
{

}
