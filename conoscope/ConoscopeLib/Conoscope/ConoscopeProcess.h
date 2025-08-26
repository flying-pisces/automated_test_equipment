#ifndef CONOSCOPEPROCESS_H
#define CONOSCOPEPROCESS_H

// configuration
#define SET_TEMPERATURE

#include "classcommon.h"
#include "camera.h"

#include "conoscopeTypes.h"
#include "PipelineLib.h"

#include "TempMonitoring.h"

#include "CDevices.h"

#include <QApplication>
#include <QDateTime>

#include "CfgHelper.h"

#include "ConoscopeStaticTypes.h"
#include <QImage>

#ifdef CAPTURE_SETTINGS_DEBUG
#include <QPainter>
#endif

#define CREATE_CAMERA_DURING_OPEN

#define FILE_NAME_FORMAT
#define ANALYSE_SAT_LEVEL

typedef enum
{
    WheelStatus_Idle      = MOTORIdle,       // 0
    WheelStatus_Success   = MOTORSuccess,    // 0x10
    WheelStatus_Operating = MOTOROperating,  // 0x20
    WheelStatus_Error     = MOTORError,      // 0x40
    WheelStatus_Bits      = MOTORStatusBits, // 0x70
} WheelStatus_t;

typedef enum
{
    WheelType_Filter,
    WheelType_Nd,
} WheelType_t;

typedef enum
{
    SetupWheelStatus_Success,
    SetupWheelStatus_Failure,
    SetupWheelStatus_Unknown
} SetupWheelStatus_t;

typedef struct
{
    QString cameraSerialNumber;
    QString cameraVersion;

    QString cfgPath;
    QString cfgFileName;

    QString capturePath;

    // output for export functions
    QString      captureFileName;
    float        sensorTemperature; // setup: temperature target
    Filter_t     eFilter;           // setup: filter
    Nd_t         eNd;               // setup: nd filter
    IrisIndex_t  eIris;             // setup: iris

    // measure
    int   exposureTimeUs;        // exposure time in micro seconds
    int   nbAcquisition;         // number of frames acquired (average)
    int   binningFactor;         //
    bool  bTestPattern;          // if set, return a test pattern returned by the sensor

    int height;
    int width;

    // cfg file
    ParamData<QString> cameraCfgFileName;
    ParamData<QString> opticalColumnCfgFileName;
    ParamData<QString> flatFieldFileName;
    ParamData<float>   colorCoefCompX;
    ParamData<float>   colorCoefCompY;
    ParamData<float>   colorCoefCompZ;

    ParamData<QString> opticalColumnCfgDate;
    ParamData<QString> opticalColumnCfgTime;
    ParamData<QString> opticalColumnCfgComment;

    double conversionFactorCompX;
    double conversionFactorCompY;
    double conversionFactorCompZ;

    int min;
    int max;

    bool saturationFlag;
    float saturationLevel;

    bool  AeEnable;
    int   AeExpoTimeGranularityUs;
} Info_t;

typedef struct
{
    bool bAeEnable;
    int  AEMeasAreaHeight;
    int  AEMeasAreaWidth;
    int  AEMeasAreaX;
    int  AEMeasAreaY;
} MeasurementAdditionalInfo_t;

typedef struct
{
    bool bCameraCfg;
    bool bOpticalColumn;
    bool bFlatField;
} NeededCfgFiles_t;

#ifdef FILE_NAME_FORMAT
typedef enum
{
    TimeStamp,
    Filter,
    Nd,
    Iris,
    ExpoTime,
    NbAcq,
    Height,
    Width,
    SatLevel,
    SatFlag,
    AeExpoGran
} FileFormatKey_t;
#endif

class ConoscopeProcess : public ClassCommon
{
    Q_OBJECT

public:
    static void Stop();

    static void Delete();

    static QString CmdGetPipelineVersion();

    static ClassCommon::Error CmdOpen();
    static ClassCommon::Error CmdSetup(SetupConfig_t &config);
    static ClassCommon::Error CmdSetupStatus(SetupStatus_t &status);
#ifdef AE_MEAS_AREA
    static ClassCommon::Error CmdMeasure(MeasureConfigWithCropFactor_t &config, bool updateCaptureDate);
#else
    static ClassCommon::Error CmdMeasure(MeasureConfig_t &config, bool updateCaptureDate);
#endif
    static ClassCommon::Error CmdExportRaw();
    static ClassCommon::Error CmdExportRaw(std::vector<uint16_t> &buffer);
    static ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config);
    static ClassCommon::Error CmdExportProcessed(ProcessingConfig_t& config, std::vector<int16_t> &buffer, bool bSaveImage);
    static ClassCommon::Error CmdClose();
    static ClassCommon::Error CmdReset();

    static ClassCommon::Error CmdSetupDebug(SetupConfig_t &config);

    static ClassCommon::Error CmdCfgFileWrite();
    static ClassCommon::Error CmdCfgFileRead();
    static ClassCommon::Error CmdCfgFileStatus(CfgFileStatus_t &status);

    static ClassCommon::Error CmdConvertRaw(ConvertRaw_t &param);

    static void GetSomeInfo(SomeInfo_t &info);

    static ConoscopeProcess* GetInstance();

#ifdef FILE_NAME_FORMAT
    static QString FormatFileName(QMap<FileFormatKey_t, QString> params);
    static void _CleanFileName(QString& fileName);
#endif

signals:
    void OnLog(QString message);

private:
    /*
     * image info
     * information about the capture image
     */
    typedef struct
    {
        QString   timeStampString;

        float     temperature;
    } ImageInfo_t;

    class CaptureInfo_t
    {
    public:
        // capture
        QString   timeStampDate;
        QString   timeStampTime;
        QString   timeStampString;

        float     temperature;

        // settings
        int imageHeight;
        int imageWidth;
#ifdef AE_MEAS_AREA
        int imageOffsetX;
        int imageOffsetY;
#endif

        int exposureUs;

        float temperatureCpu;
        float temperatureMainBoard;
        float temperatureSensor;

        QString cameraBoardSerialNumber;

        void Clone(CaptureInfo_t& input)
        {
            timeStampDate           = input.timeStampDate;
            timeStampTime           = input.timeStampTime;
            timeStampString         = input.timeStampString;
            temperature             = input.temperature;
            imageHeight             = input.imageHeight;
            imageWidth              = input.imageWidth;
#ifdef AE_MEAS_AREA
            imageOffsetX            = input.imageOffsetX;
            imageOffsetY            = input.imageOffsetY;
#endif
            exposureUs              = input.exposureUs;
            temperatureCpu          = input.temperatureCpu;
            temperatureMainBoard    = input.temperatureMainBoard;
            temperatureSensor       = input.temperatureSensor;
            cameraBoardSerialNumber = input.cameraBoardSerialNumber;
        }
    } ;

    explicit ConoscopeProcess(QObject *parent = nullptr);
    ~ConoscopeProcess();

    void _ConfigureHal();
    void _CreateCamera();
    void _DeleteCamera();

    QString _CmdGetPipelineVersion();

    ClassCommon::Error _CmdOpen();
    ClassCommon::Error _CmdSetup(SetupConfig_t &config);
    ClassCommon::Error _CmdSetupStatus(SetupStatus_t &status);
    ClassCommon::Error _CmdMeasure(MeasureConfigWithCropFactor_t &config, bool updateCaptureDate);
    ClassCommon::Error _CmdExportRaw();
    ClassCommon::Error _CmdExportRaw(std::vector<uint16_t> &bufferV);

    void _FillInfo(SetupConfig_t &setupConfig, QString fileName = "");

    ClassCommon::Error _CmdExportProcessed(ProcessingConfig_t &config);
    ClassCommon::Error _CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &bufferV, bool bSaveImage);
    ClassCommon::Error _Processed(SetupConfig_t& setupConfig, ProcessingConfig_t &config, QString fileName = QString(""), bool bAlwaysComputeKLib = true);

    ClassCommon::Error _CmdClose();
    ClassCommon::Error _CmdReset();

    ClassCommon::Error _CmdSetupDebug(SetupConfig_t &config);

    ClassCommon::Error _CmdCfgFileWrite();
    ClassCommon::Error _CmdCfgFileRead();
    ClassCommon::Error _CmdCfgFileStatus(CfgFileStatus_t& status);

    ClassCommon::Error _CmdConvertRaw(ConvertRaw_t &param);

    CameraInfo_t _OpeningInfo();

    Error _WriteImageFile(QString filename,
                          char* pImage,
                          int imageSize,
                          CaptureInfo_t &captureInfo,
                          QMap<QString, QMap<QString, QVariant>> &settings = QMap<QString, QMap<QString, QVariant>>(),
                          const QRect& fullImage = QRect(),
                          const QRect& zoneToSave = QRect());

    Error _WriteImageFile(QString filename,
                          char* pImage,
                          int imageSize,
                          const QRect& fullImage = QRect(),
                          const QRect& zoneToSave = QRect());

    void _WriteImageInfo(QString filePath,
                         CaptureInfo_t& captureInfo,
                         QMap<QString, QMap<QString, QVariant> > &settings);

    template<typename T>
    Error _SaveImage(QString fileName, T* pRawData, int imageHeight, int imageWidth)
    {
        ClassCommon::Error eError = ClassCommon::Error::Ok;

        int imageSize = imageHeight * imageWidth;

        QByteArray bmpArray;
        bmpArray.resize(imageSize * 4);

        unsigned char   *bmpData;
        bmpData = (unsigned char*)&(bmpArray.data()[0]);

        int pixIndex = 0;

        unsigned char   ucCurrentValue ;
        T currentValue;

/*
        int nbBits = 12;
        int minPixelValue = 0;
        int maxPixelValue = pow(2, nbBits) - 1;
        int shift = 16 - nbBits;
*/
        int minPixelValue = 0;
        int maxPixelValue = 4095;
        int shift = 4;

#pragma omp parallel num_threads(4)
        for (int index = 0 ; index < imageSize ; index ++)
        {
            // ucCurrentValue = ((long)mQVSubSamplingData.at(lIndex)*255)/(MAX_DATA_VALUE) ;
            // ucCurrentValue = ((long)mQVSubSamplingData.at(lIndex)*255)/(mWhiteLevel);
            //ucCurrentValue = (unsigned char)(_rawData.at(index) >> 4);
            currentValue = pRawData[index];

            // clamp value to 12 bits
            if(currentValue < minPixelValue)
            {
                currentValue = minPixelValue;
            } else if(currentValue > maxPixelValue)
            {
                currentValue = maxPixelValue;
            }

            // convert the pixel to 8 bits by removing less significant part
            currentValue = currentValue >> shift;

            ucCurrentValue = (unsigned char) currentValue;

            bmpData[pixIndex++] = ucCurrentValue;
            bmpData[pixIndex++] = ucCurrentValue;
            bmpData[pixIndex++] = ucCurrentValue;
            bmpData[pixIndex++] = 0;
        }

        QImage image(bmpData, imageWidth, imageHeight, QImage::Format_RGBX8888);

#ifdef CAPTURE_SETTINGS_DEBUG

#define TEXT_LEFT     50
#define TEXT_TOP     100
#define TEXT_WIDTH   1000
#define TEXT_HEIGHT  100

        QRect textRect (TEXT_LEFT, TEXT_TOP, TEXT_WIDTH, TEXT_HEIGHT);
        QImage textImage(TEXT_WIDTH, TEXT_HEIGHT, QImage::Format_RGBX8888);

        QPainter textPainter;

        if(textPainter.begin(&textImage))
        {
            textPainter.setPen(QPen(Qt::red));
            textPainter.setFont(QFont("Times", 50, QFont::Bold));


            QString imageMessage = QString("setup %1 - measure %1").arg(debugSetupIndex).arg(debugExportIndex);

            textPainter.drawText(textImage.rect(), Qt::AlignCenter, imageMessage);
            textPainter.end();
        }

        // QPainter::CompositionMode mode = currentMode();

        QImage resultImage(image.width(), image.height(), QImage::Format_RGBX8888);
        QPainter painter(&resultImage);

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(resultImage.rect(), Qt::red);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(0, 0, image);

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(0, 0, textImage);

        painter.end();

        resultImage.save(fileName, "JPG");
#else
        image.save(fileName, "JPG");
#endif

        return eError;
    }

#define AVERAGE_ANALYSE

#ifdef AVERAGE_ANALYSE
#ifdef REMOVED
    void _ListMin(QList<uint16_t> &list, int listSize, int16 value);
#endif
    void _ListMax(QList<uint16_t> &list, int listSize, uint16_t value);
    void _ListTrim(QList<uint16_t> &list, int listSize);
    uint16 _ListAverage(QList<uint16>& list);
#endif

    // function to get the max value of an image
    // actually, concider the captureSequenceMaxNbPixel'th brighter pixel
    template<typename T>
    ClassCommon::Error _AnalyseData(int width, int height, std::vector<T>& arr, int& outMax)
    {
        ClassCommon::Error eError = ClassCommon::Error::Failed;

        T* pData = (T*)arr.data();
        int dataValue = 0;
        int dataIndex = 0;

        int nbPixels = ConoscopeProcess::mSettingsI.AEMaxNbPixel;

        // at least one pixel in the list
        if(nbPixels < 1)
        {
            nbPixels = 1;
        }

        QList<uint16_t> maxList;

        for(int index = 0; index < nbPixels; index ++)
        {
            maxList.push_back(0);
        }

        if(arr.size() >= (unsigned int)(height * width))
        {
            for(int indexLine = 0; indexLine < height; indexLine ++)
            {
                for(int indexCol = 0; indexCol < width; indexCol ++)
                {
                    dataValue = pData[dataIndex ++];

                    _ListMax(maxList, nbPixels, dataValue);
                }
            }

            outMax = maxList.last();

            eError = ClassCommon::Error::Ok;
        }

        return eError;
    }

    void _GetSomeInfo(SomeInfo_t& info);

#ifdef SATURATION_FLAG_RAW
    ClassCommon::Error _GetSaturationFlag(uint16_t saturationValue, int nbPixels, uint16_t* data, bool& saturationFlag, float& saturationLevel);
#endif

    /*
     * image info (from image read)
     * This may not match with the data stored
     */
    typedef struct
    {
        int imageHeight;
        int imageWidth;
        int exposureUs;
        int nbAcquisition;

        Filter_t eFilter;
        Nd_t eNd;
        IrisIndex_t eIris;
        float temperature;

        bool bProcessed;

        bool bAbsolute;
        bool bBiasCompensation;
        bool bFlatField;
        bool bLinearisation;
        bool bSensorDefectCorrection;
        bool bSensorPrnuCorrection;

        QString date;
        QString time;

        bool saturationFlag;
        float saturationLevel;
    } ImageInfoRead_t;

    Error _ReadImageFile(QString acFilename, QByteArray& imgData, ImageInfoRead_t& info);

    Error _ReadImageInfo(QString filePath, ImageInfoRead_t& info);

    ClassCommon::Error _GetCfg(SetupConfig_t& setupConfig, ConfigContent_t& configContent, QMap<ComposantType_t, float> &colorCoef, NeededCfgFiles_t neededCfgFiles);

    void _Log(QString message);

    NeededCfgFiles_t _GetNeededCfgFiles(ProcessingConfig_t &config);

    ClassCommon::Error _ReadCfgCameraPipeline(QString sn);

    bool _HasSetupChanged(float sensorTemperature);

    void _WaitForSetupIsDone();

    WheelStatus_t _GetWheelStatus();

    WheelStatus_t _GetWheelStatus(Nd_t &eNdWheel, Filter_t &eFilterWheel);

    SetupConfig_t _setupConfig;

    SetupConfig_t _measurementConfig;

    // RawData
    QByteArray                 _rawData;           /* raw data captured */
    QByteArray                 _rawDataStdDev;     /* raw data captured standard dev (not used) */

    CaptureInfo_t _captureInfo;
    QString _timeStampString_test;

    // ProcessedData
    QByteArray                 _inputData; /* copy of raw data used in process function */
    std::vector<char>          _klibData;
    std::vector<char>          _klibDataCrop;

protected:
    static ConoscopeProcess* mInstance;
    static ConoscopeProcess* _GetInstance();
    Camera* mCamera;
    CDevices  *mDevices ;

    PipelineLib* mPipelineLib;

public:
    static ConoscopeDebugSettings_t mDebugSettings;
    static ConoscopeSettings_t      mSettings;
    static ConoscopeSettingsI_t     mSettingsI;
    static Info_t                   mInfo;

    static MeasurementAdditionalInfo_t mAdditionalInfo;

private:
    std::map<Nd_t, int> NdWheelMap;
    std::map<Filter_t, int> FilterWheelMap;

    std::map<int, Nd_t> NdWheelRevertMap;
    std::map<int, Filter_t> FilterWheelRevertMap;

    std::map<WheelStatus_t, WheelState_t> WheelStatusMap;

    std::map<WheelType_t, int> WheelTypeIndexMap;

    TempMonitoring* mTempMonitor;

    QString _CreateFolder(std::string path, QString cameraSerialNumber = "");

    QString _GetFileName(QString name, QString path, QString extension);

    void _GetCameraInfo();

    CfgFileStatus_t mCfgFileStatus;

    QString mErrorDescription;

    // wheel setup error status: indicate if an error happen during wheel setup
    SetupWheelStatus_t mSetupWheelFailure;

    SetupConfig_t measureSettings;

public slots:
    void OnCameraLogInFile(QString header, QString message);
};

#endif // CONOSCOPEWORKER_H
