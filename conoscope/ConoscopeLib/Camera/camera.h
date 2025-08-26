#ifndef CAMERA_H
#define CAMERA_H

/*!
 * \file    camera.h
 * \brief   instance of thorlabs
 *          (not the instrument itself but the engine)
 * \author  lclementine@eldim.fr
 * \version 0.1
 */

#include "classcommon.h"
#include "toolTypes.h"
#include "imageConfigurationConst.h"

#include <QSharedPointer>
#include <QRect>

#include <QJsonObject>

#define ADDR_SIZE 32

typedef enum
{
    CameraModel_CmvEth_8k,
    CameraModel_CmvCxp_8k,
    CameraModel_CmvCxp_50k,

    CameraModel_Adimec_50k,
    CameraModel_Unknown
} CameraModel_t;

/*! \class Thorlabs
 *  \brief instance of a module
 */
class Camera : public ClassCommon
{
    Q_OBJECT

public:
    enum class Event {
        Connect,
        Disconnect,
    };
    Q_ENUM(Event)

    struct CaptureConfig {
        int mnExposureMicros;
        int mnNumImages;
        int mnVBin;
        QRect mcDimensions;
        bool mbExtTrig;
        bool mbTestPattern;
        int mnTrigDelayMicros;
        bool bStoreStdDev;

        CaptureConfig()
            : mnExposureMicros(0)
            , mnNumImages(1)
            , mnVBin(1)
            , mcDimensions(_IMAGE)
            , mbExtTrig(false)
            , mbTestPattern(false)
            , mnTrigDelayMicros(0)
            , bStoreStdDev(false)
        {}
    };

    class CameraInfoItem
    {
    public:
        std::string name;
        std::string value;

        CameraInfoItem(std::string itemName, std::string itemValue)
        {
            name = itemName;
            value = itemValue;
        }
    };

    typedef struct
    {
        CameraSettings settings;

        ParamData<QString> cameraBoardSerialNumber;
        ParamData<QString> cameraSerialNumber;
        ParamData<QString> CpuBoardRev;
        ParamData<QString> SensorBoardRev;
        ParamData<QString> CameraRev;
        ParamData<QString> SoftwareVersion;

        ParamData<QString> ModelName;
        ParamData<QString> VendorName;

        QList<CameraInfoItem> cameraSpecific;

    } CameraInfo_t;

    typedef struct
    {
        ParamData<QList<int>> version;
    } InterfaceInfo_t;

    class settingItem
    {
    public:
        std::string name;
        std::string value;

        settingItem(){}

        settingItem(std::string name_, std::string value_)
        {
            name = name_;
            value = value_;
        }
    };

    typedef struct
    {
        QVector<settingItem> settingList;
    } CameraSettings_t;

    struct PipelineConfig
    {
        bool bias;
        bool defect;
        bool prnu;
    };

    struct Pulse
    {
        int usWidth;
        int usPeriod;
    };

    enum Status
    {
        Ready = 0,
        MeasurementPending = 1,
        MeasurementDone = 2,
        Fault = 3,
        Connected = 4,
        NotInitialised,
        Invalid
    };

    struct RawDataInfo
    {
        int miLines;
        int miCols;
        QRect cropArea;
        CameraSettings settings;
    };

private:
    CameraInfo_t mInfo;

protected:
    QString mIpAddr;
    QString mPort;
    QString mConnectionConfig;

    CameraModel_t mModel;

public:
    Camera(QObject *parent = nullptr);

    ~Camera();

    virtual ClassCommon::Error Connect();

    virtual ClassCommon::Error ConnectPowerCycle();

    virtual ClassCommon::Error Disconnect();

    virtual ClassCommon::Error SetDefaultConfiguration();

    virtual bool IsConnected();

    virtual bool IsFileTransferSupported();

    virtual void FileTransferOpen();

    virtual void FileTransferClose();

    virtual void FileTransferWrite();

    virtual void FileTransferRead();

    virtual void FileTransferWrite(const void *data, size_t size);

    virtual void FileTransferRead(void *data, size_t size);

    virtual int FwUpdateConfigure(size_t size);

    virtual ClassCommon::Error FwUpdate(QString type, char* firmware, size_t firmwareSize);

    virtual ClassCommon::Error Register(QString cameraSerialNumber, QString cameraBoardSerialNumber);

    virtual ClassCommon::Error ConfigureConnection(
        QString ipAddr,
        QString port,
        QString connectionConfig);

    virtual ClassCommon::Error GetInfo(CameraInfo_t& info);

    virtual ClassCommon::Error CheckConnection();

    virtual ClassCommon::Error GetInterfaceInfo(Camera::InterfaceInfo_t& interfaceInfo);

    // this is the serial number of the camera
    // it may differ from the serial number of the camera device
    virtual ClassCommon::Error GetSerialNumber(QString& cameraSerialNumber, QString &cameraBoardSerialNumber);

    virtual ClassCommon::Error GetSettings(CameraSettings_t& settings);

    virtual ClassCommon::Error Configure(struct CaptureConfig* pConfig);

    virtual int GetCurrentCaptureTimeoutSteps(int stepMs = 1);

    virtual ClassCommon::Error ConfigurePipeline(struct PipelineConfig *pConfig);

    virtual ClassCommon::Error StartMeasurement();

    virtual ClassCommon::Error StopMeasurement();

    virtual ClassCommon::Error GetPulse(struct Pulse* pPulse);

    virtual ClassCommon::Error GetStatus(enum Status& eStatus);

    virtual ClassCommon::Error GetRawData(struct RawDataInfo &info, QByteArray& arr, QByteArray &stdDev);

#ifdef SOAP_INTERFACE
    virtual void SetPrnu(ns1__prnu *data, struct ns1__setPRNUResponse &_param_1);

    virtual void SetSensorDefects(ns1__sensorDefects *defects, struct ns1__setSensorDefectsResponse &_param_1);

    virtual void SetSensorSaturation(ns1__sensorSaturation *value, struct ns1__setSensorSaturationResponse &_param_1);

    virtual void SetPartInformation(ns1__part *partIndex, ns1__partInformation *partInformation, struct ns1__setPartInformationResponse &_param_1);

    virtual void StoreConfigurationFile(struct ns1__storeConfigurationFileResponse &_param_1);
#endif

    virtual void UpdateCaptureConfiguration(
            int& numberReads,
            int& numberCaptures);

    virtual void GetCameraInfo(
            QString     &cameraInfo,
            QJsonObject &cameraInfoJson);

    virtual ClassCommon::Error ErrorRecovery();

    virtual ClassCommon::Error SetModelName(QString modelName, QString serialNumber = "");

    // TEC specific

#ifdef TEC_INTERFACE
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

    float   HW_ADC_TEC_Value(Camera::eDAC_Channel ucChannel, bool inPercent = false);
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
#endif

#ifdef TEC_INTERFACE
    void HW_DAC8571_WritePWDSettings(UCHAR ubSettings);
    void HW_ADC128_WriteRegister(UCHAR ubRegAddress, UCHAR ubValue);
    void HW_PCA9536_ConfigureAsOutput(UCHAR ubBit);
#endif

    // function for I2C. May be replaced by I2C interface
    virtual bool    HW_I2CIsDeviceReady(unsigned char i2c_dev, unsigned char Address);
    virtual void    HW_I2CSelectDevice(unsigned char ucBUS, unsigned char ucAddress) ;
    virtual bool    HW_I2CSendAndReceiveData(const unsigned char *TXBuffer, unsigned char TXCount, unsigned char *RXBuffer, unsigned char RXCount) ;

    // those functions should not be public
    // define interface instead
    virtual QString GetStringValue(const std::string feature);
    virtual void    SetStringValue(const std::string feature, QString value);
    virtual int     GetIntegerValue(const std::string feature);
    virtual void    SetIntegerValue(const std::string feature, int value);
    virtual float   GetFloatValue(const std::string feature);
    virtual void    SetFloatValue(const std::string feature, float value);

    virtual float   GetHw(QString hwDeviceName, QString hwFeature);
    virtual void    SetHw(QString hwDeviceName, QString hwFeature, float value, int index = 0);

signals:
    void EventOccured(int event);

    void ProgressUpdate(int stepCount);

    void LogMessage(QString message);

    void LogInFile(QString header, QString message);

private:
    float mDummySensorTarget;
    float mDummySensorTargetStep;

protected:
    void _LogMessage(QString message);

public slots:
    void _LogInFile(QString header, QString message);
};
#endif // CAMERA_H
