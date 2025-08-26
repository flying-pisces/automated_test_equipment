#ifndef COAXPRESSGRABBER_H
#define COAXPRESSGRABBER_H

#include <QObject>

#include <EGrabber.h>
using namespace Euresys;

#include "CoaXpressConfiguration.h"
#include "CoaXpressTypes.h"
#include "CoaXpressFrame.h"

#include <QTime>
// TODO replace QRect by custom class
#include <QRect>
#include <QFile>
#include <QTextStream>
#include <QThread>

#include "classcommon.h"
#include "toolTypes.h"

#define PRINT

// #define FIXED_FPS

#define VENDOR_CRITICAL_LINK "CriticalLink"
#define VENDOR_ADIMEC "Adimec"

#define USE_POP

class CoaXpressGrabber_CameraInfoItem
{
public:
    std::string name;
    std::string value;

    CoaXpressGrabber_CameraInfoItem(std::string itemName, std::string itemValue)
    {
        name = itemName;
        value = itemValue;
    }
};

class CoaXpressGrabber_CameraInfo
{
public:
    std::string deviceId;
    std::string deviceVendorName;
    std::string deviceModelName;

    std::string cameraVendorName;
    std::string cameraModelName;
    std::string cameraManufacturerInfo;
    std::string cameraVersion;
    std::string cameraFirmwareVersion;
    std::string cameraSerialNumber;

    QList<CoaXpressGrabber_CameraInfoItem> cameraSpecific;

    CoaXpressGrabber_CameraInfo()
    {

    }

    CoaXpressGrabber_CameraInfo(const CoaXpressGrabber_CameraInfo& info)
    {
        deviceId               = info.deviceId;
        deviceVendorName       = info.deviceVendorName;
        deviceModelName        = info.deviceModelName;

        cameraVendorName       = info.cameraVendorName;
        cameraModelName        = info.cameraModelName;
        cameraManufacturerInfo = info.cameraManufacturerInfo;
        cameraVersion          = info.cameraVersion;
        cameraFirmwareVersion  = info.cameraFirmwareVersion;
        cameraSerialNumber     = info.cameraSerialNumber;

        for(int index = 0; index < info.cameraSpecific.length(); index ++)
        {
            cameraSpecific.append(info.cameraSpecific[index]);
        }
    }
};

class CoaXpressGrabber_InterfaceInfo
{
public:
    // std::string version;
    QList<int> version;

    CoaXpressGrabber_InterfaceInfo()
    {

    }

    CoaXpressGrabber_InterfaceInfo(const CoaXpressGrabber_InterfaceInfo& info)
    {
        version = info.version;
    }
};

class CoaXpressGrabber_CameraSettings
{
public:
    std::string pixelFormat;
    int binningHorizontal;
    int binningVertical;
    bool DefectPixelCorrectionEnable;
    bool DF_ColumnOffsetCorrection;
    bool BF_ColumnGainCorrection;
    std::string exposureMode;
    int blackLevel;
    bool SensitivityMatching;

    CoaXpressGrabber_CameraSettings()
    {

    }

    CoaXpressGrabber_CameraSettings(const CoaXpressGrabber_CameraSettings& settings);
};

class CoaXpressGrabber_Config
{
public:
    int   exposureUs;         // exposure time (micro second)
    int   acquisitionNumber;  // number of pictures per acquisition
    int   VBin;
    QRect dimensions;         // dimension of the picture
    bool  bExtTrig;
    bool  bTestPattern;
    int   trigDelayMs;
#ifdef STD_DEV_FILE
    bool  bStoreStdDev;
#endif

    CoaXpressGrabber_Config();

    CoaXpressGrabber_Config(const CoaXpressGrabber_Config& value);
};

typedef struct
{
    uint64_t   tx;
    uint64_t   rx;
} AddressDefinition;

#ifndef USE_POP
class CoaXpressGrabber : public QObject, public EGrabber<CallbackMultiThread>
//class CoaXpressGrabber : public QObject, public EGrabber<CallbackSingleThread>
#else
class CoaXpressGrabber : public QObject, public EGrabber<CallbackOnDemand>
#endif
{
    Q_OBJECT

public:
    enum eFirmwareType
    {
        FPGA,
        NIOS,
        unknown
    };

    static QString mScriptFile;

    explicit CoaXpressGrabber(EGenTL &gentl, bool bPowerCycle, QObject *parent = NULL);

    void getInterfaceInfo();

    void DetectCamera(std::string connection);

    void SetCameraDefaultConfiguration();

    void SetCameraConstants();

    void Configure(CoaXpressGrabber_Config& config);

    void PowerCycle();

    bool Start(int acquisitionNumber = 0);

    void Stop();

    void GetTemperature(CameraSettings &settings);

    void GetExposureTime(int &exposureTime);

    typedef struct
    {
        QString serialNumber;
        QString pixelFormat;
        int     DefectPixelCorrectionEnable;
        int     DF_ColumnOffsetCorrection;
        int     BF_ColumnGainCorrection;
        int     SensitivityMatching;
        QString ExposureMode;
        int     BlackLevel;
        int     ReverseX;
        int     BinningHorizontal;
        int     BinningVertical;
        int     Width;
        int     Height;
        int     OffsetX;
        int     OffsetY;
        int     AcquisitionFramePeriod;
        int     AcquisitionFramePeriodRaw;
        int     ExposureTime;
        int     ExposureTimeRaw;
    } GetInfo_t;

    void GetInfo(GetInfo_t& infoData);

    void UpdateFrameSize();

    void PrintConfig();

    //void Configure(CoaXpressGrabber_Config& config):

    int GetCurrentCaptureTimeoutSteps(int stepMs = 1);

    void GetCameraInfo(CoaXpressGrabber_CameraInfo& info);

    void GetCameraSettings(CoaXpressGrabber_CameraSettings& settings);

    void GetInterfaceInfo(CoaXpressGrabber_InterfaceInfo &info);

    void ErrorRecovery();

    bool IsFileTransferSupported();

    void FileTransferOpen();

    void FileTransferOpen(eFirmwareType eType);

    void FileTransferClose();

    // const long trans_size = 168;
    //const long trans_size = 192;
    //const long trans_size = 100;

#ifdef REMOVED_FILE_T
    void FileTransferWrite(uint64_t address, const void *data, size_t size,
                           const void *fileData, size_t fileSize);

    void FileTransferRead(uint64_t address, void *data, size_t size,
                         void *fileData, size_t fileSize);
#endif

    void FileTransferWrite();

    void FileTransferRead();

    void FileTransferWrite(const void *data, size_t size);

    void FileTransferWrite(uint64_t address, const void *data, size_t size);

    void FileTransferRead(void *data, size_t size);

    void FileTransferRead(uint64_t address, void *data, size_t size);

    int FwUpdateConfigure(size_t size);

    ClassCommon::Error FwUpdate(eFirmwareType eType, const void *data, size_t size);

    // DG Functions !

    void I2CSelectDevice(unsigned char ucBUS, unsigned char ucAddress) ;
    bool I2CSendAndReceiveData(const unsigned char *TXBuffer, unsigned char TXCount, unsigned char * RXBuffer, unsigned char RXCount) ;

    void ChangeEepromModelName(QString modelName, QString serialNumber = "");

    int GetCurrentFrameIndex(){return mCurrentFrameIndex;}

    bool HasCameraChanged();

signals:
    void FrameCaptured(int frameIndex);

    void ProgressUpdate(int stepCount);

    void LogInFile(QString header, QString message);

private:
    void SetConnectionConfig(std::string connection);

    virtual void onNewBufferEvent(const NewBufferData &data);

    ImageFeature                    mImageFeature;
    int                             mAcquisitionCount;
    int                             mCurrentFrameIndex;
    int                             mCurrentAcquisitionFramePeriod;
    int                             mAcquisitionFramePeriodMin;

    CoaXpressGrabber_Config         mConfig;
    CameraManufacturer_t            mCameraManufacturer;
    CoaXpressGrabber_CameraInfo     mCameraInfo;
    CoaXpressGrabber_CameraSettings mCameraSettings;
    CoaXpressGrabber_InterfaceInfo  mInterfaceInfo;

    std::string _LoadScriptStatement(QString fileName);

    // DG Variables
    unsigned char                   mucBus ;
    unsigned char                   mucSlaveAddress ;

    void _ReadCameraInfo();

    std::vector<char> mFileTransferBuffer;

    int mMajorNumber = 0;
    int mMinorNumber = 0;
    int mI2CAddressIndex;

    const static AddressDefinition mI2CAddressList[];

    int mFileTransferProgressStepSize;
    int mFileTransferPacketSize;

    void _LogInFile(QString message);
};

#endif // COAXPRESSGRABBER_H
