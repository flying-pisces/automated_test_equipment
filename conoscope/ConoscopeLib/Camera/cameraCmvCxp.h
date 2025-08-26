#ifndef CAMERA_CMV_CXP_H
#define CAMERA_CMV_CXP_H

#include <QMap>

#include "camera.h"

#include <EGenTL.h>
#include <EGrabber.h>

#include "CoaXpressTypes.h"

#define USE_CUSTOM_GRABBER
#ifdef USE_CUSTOM_GRABBER
#include "CoaXpressGrabber.h"
#endif

class CameraCmvCxp : public Camera
{
    Q_OBJECT
public:

    enum eDAC_Channel {
        e_ADC_ZERO          = 0x0,
        e_DAC_OUTPUT        = 0x1,
        e_ADC_VTEC          = 0x2,
        e_ADC_ITEC          = 0x3,
        e_ADC_VIN           = 0x4,
        e_ADC_5V            = 0x5,
        e_ADC_3V3           = 0x6,
        e_ADC_Temperature   = 0x7
    };

    explicit CameraCmvCxp(QObject *parent = nullptr);

    ~CameraCmvCxp();

    bool IsConnected();

    bool IsFileTransferSupported();

    void FileTransferOpen();

    void FileTransferClose();

    void FileTransferWrite();

    void FileTransferRead();

    void FileTransferWrite(const void *data, size_t size);

    void FileTransferRead(void *data, size_t size);

    ClassCommon::Error FwUpdateTypeList(QList<QString>& typeList);

    int FwUpdateConfigure(size_t size);

    ClassCommon::Error FwUpdate(QString type, char* firmware, size_t firmwareSize);

    ClassCommon::Error Connect();

    ClassCommon::Error ConnectPowerCycle();

    ClassCommon::Error Disconnect();

    ClassCommon::Error SetDefaultConfiguration();

    ClassCommon::Error GetInfo(CameraInfo_t& info);

    ClassCommon::Error CheckConnection();

    ClassCommon::Error GetInterfaceInfo(InterfaceInfo_t& interfaceInfo);

    ClassCommon::Error GetSerialNumber(QString& cameraSerialNumber, QString &cameraBoardSerialNumber);

    ClassCommon::Error GetSettings(CameraSettings_t& settings);

    ClassCommon::Error Configure(struct CaptureConfig* pConfig);

    int GetCurrentCaptureTimeoutSteps(int stepMs = 1);

    ClassCommon::Error ConfigurePipeline(struct PipelineConfig *pConfig);

    ClassCommon::Error StartMeasurement();

    ClassCommon::Error StopMeasurement();

    ClassCommon::Error GetPulse(struct Pulse* pPulse);

    ClassCommon::Error GetStatus(enum Status& eStatus);

    ClassCommon::Error GetRawData(struct RawDataInfo &info, QByteArray& arr, QByteArray &stdDev);

#ifndef COAXPRESS_FRAME_AVERAGE
    void UpdateCaptureConfiguration(
                int& numberReads, int& numberCaptures);
#endif

#ifdef SOAP_INTERFACE
    void SetPrnu(ns1__prnu *data, struct ns1__setPRNUResponse &_param_1);

    void SetSensorDefects(ns1__sensorDefects *defects, struct ns1__setSensorDefectsResponse &_param_1);

    void SetSensorSaturation(ns1__sensorSaturation *value, struct ns1__setSensorSaturationResponse &_param_1);

    void SetPartInformation(ns1__part *partIndex, ns1__partInformation *partInformation, struct ns1__setPartInformationResponse &_param_1);

    void StoreConfigurationFile(struct ns1__storeConfigurationFileResponse &_param_1);
#endif

    virtual void GetCameraInfo(
            QString     &cameraInfo,
            QJsonObject &cameraInfoJson);

#ifdef CAMERA_ERROR_RECOVERY
    virtual ClassCommon::Error ErrorRecovery();
#endif

    void SetScriptStatement(QString fileName);

    ClassCommon::Error SetModelName(QString modelName, QString serialNumber = "");

//#ifdef TEC_INTERFACE
    // interface (other function should be private)
    // this should be defined in another class (this is not the interface of the camera itself)
    float   HW_ADC_TEC_Value(CameraCmvCxp::eDAC_Channel ucChannel, bool inPercent = false);
    float   HW_TMP10XReadTemperature(UCHAR i2c_dev, UCHAR Address);

    void    HW_DAC8571_WriteData(USHORT usValue);
    bool    HW_PCA9536_WriteRegister(UCHAR RegAddress, UCHAR ubValue);
    UCHAR   HW_PCA9536_ReadRegister(UCHAR RegAddress);
    void    HW_PCA9536_I2CSelectI2CBus(UCHAR ucBusNumber);
    void    HW_PCA9536_SetOutputValue(UCHAR ubBit, UCHAR ubValue);
    void    HW_SetTriggerGain(UCHAR ubGain);
    float   HW_Get_TEC_Hot_Temperature();
    float   HW_Get_Sensor_Cold_Temperature();
    void    HW_Disable_TEC();
    void    HW_Enable_TEC();
    void    HW_Set_DAC_TEC_OutputValue(float fValue);
    float   HW_Get_CMOSTemperature();
    float   HW_Get_MainboardTemperature();
    float   HW_Get_CPUTemperature();
    float   HW_Get_FanSpeed(int intFanSelect);
    void    HW_Set_FanSpeed(int intFanSelect, float fValueInPercent);
//#endif

//#ifdef TEC_INTERFACE
    void    HW_DAC8571_WritePWDSettings(UCHAR ubSettings);
    void    HW_ADC128_WriteRegister(UCHAR ubRegAddress, UCHAR ubValue);
    void    HW_PCA9536_ConfigureAsOutput(UCHAR ubBit);
//#endif

    bool    HW_I2CIsDeviceReady(unsigned char i2c_dev, unsigned char Address);
    void    HW_I2CSelectDevice(unsigned char ucBUS, unsigned char ucAddress) ;
    bool    HW_I2CSendAndReceiveData(const unsigned char *TXBuffer, unsigned char TXCount, unsigned char *RXBuffer, unsigned char RXCount) ;

    QString GetStringValue(const std::string feature);
    void    SetStringValue(const std::string feature, QString value);
    int     GetIntegerValue(const std::string feature);
    void    SetIntegerValue(const std::string feature, int value);
    float   GetFloatValue(const std::string feature);
    void    SetFloatValue(const std::string feature, float value);

    float   GetHw(QString hwDeviceName, QString hwFeature);
    void    SetHw(QString hwDeviceName, QString hwFeature, float value, int index = 0);

     UCHAR   CurrentI2C_BUS_PCA ;

signals:
    void FrameCaptured(int frameIndex);

public slots:

private:
    Euresys::EGenTL*  mGentl;
    CoaXpressGrabber* mGrabber;

    typedef enum
    {
        CameraState_NotConnected,
        CameraState_Connected,
        CameraState_Undefined
    } CameraState_t;

    CameraState_t eState;

    Camera::Status mCaptureState;
    int mCurrentFrameIndex;

    void NotifyEvent(Event eEvent);

    void onFrameCaptured(int frameIndex);

    ClassCommon::Error _Disconnect();

    ClassCommon::Error _Connect(bool bPowerCycle);

    QMap<QString, CoaXpressGrabber::eFirmwareType> mFwType;

    CameraModel_t GetCameraModel(CoaXpressGrabber_CameraInfo& cameraInfo);
};

#endif // CAMERA_CMV_CXP_H
