#ifndef CFGHELPER_H
#define CFGHELPER_H

#include <QObject>
#include <QString>
#include <QDomDocument>

#include "PipelineTypes.h"
#include "conoscopeTypes.h"

#include "toolTypes.h"

#define AIRSHIP_CFG_PATH         "AIRSHIP_%1.cfg"
#define CAMERA_CFG_PATH          "CAMERA_%1.cfg"

#define CAMERA_PRNU_PATH(x) x.replace(".cfg", "_prnu.bin")

#define OPTICAL_COLUMN_FILE_NAME "OpticalColumn.xml"
#define FLAT_FIELD_CFG_FILE_NAME "FlatField.xml"
#define FLAT_FIELD_FILE_NAME     "FlatField_iris_%1_filter_%2.bin"

typedef struct
{
    int value;
} CalibrationStep_t;

typedef struct
{
    std::string date;
    std::string time;
    std::string comment;
} Summary_t;

typedef struct
{
    std::string type;
    std::string description;
    std::string location;
    std::string revision;
    std::string serialNumber;
} Equipement_t;

typedef struct
{
    std::string date;
    std::string time;
    std::string comment;
} CalibrationSummary_t;

typedef struct
{
    struct
    {
        float current;
        float averaged;
    } die;
    float heatsink;
} SensorTemperature_t;

typedef struct
{
    struct
    {
        float slope;
        int table;
        bool correctionEnable;
        long timeStamp;
        std::string sensorSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;
    } sensorTemperatureDependency;

    struct
    {
        struct
        {
            short X;
            short Y;
        } opticalAxis;
        long measurementRadius;
        long timeStamp;
        std::string equipmentSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;
    } captureArea;

    float maximumIncidentAngle;
    short calibratedDataRadius;

    struct
    {
        float A1;
        float A3;
        float A5;
        float A7;
        float A9;
        long timeStamp;
        std::string equipmentSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;
    } linearizationCoefficients;

    struct
    {
        int isCalibrated;
        float conversionFactor;
        float maximumIncidentAngle;
        short radius;
        short maxBinaryValue;
        int saturationOccurs;
        long timeStamp;

        std::vector<char> data;

        SensorTemperature_t sensorTemperature;
        std::string equipmentSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;

    } flatField;

    struct
    {
        float value;

        SensorTemperature_t sensorTemperature;

        long timeStamp;
        std::string equipmentSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;

    } conversionFactor;

} OpticalColumnCalibration_t;

typedef enum{
    BiasMode_insideMeasurement = 0,
    BiasMode_interPulses = 1,
    BiasMode_darkPixels = 2,
    BiasMode_storedDarkMeasurement = 3
} BiasMode_t;

typedef struct
{
    struct
    {
        BiasMode_t mode;
        bool compensationEnabled;
    } biasMode;

    struct
    {
        short value;
        SensorTemperature_t sensorTemperature;
        long timeStamp;
        std::string sensorSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;
    } sensorSaturation;

    struct
    {
        std::vector<Defect> pixels;

        int correctionEnabled;
        SensorTemperature_t sensorTemperature;
        long timeStamp;
        std::string sensorSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;
    } sensorDefects;

    struct
    {
        struct
        {
            short width;
            short height;
        } captureSize;

        float scaleFactor;
        bool correctionEnabled;

        std::vector<char> data;

        SensorTemperature_t sensorTemperature;

        long timeStamp;
        std::string sensorSerialNumber;
        std::string stationSerialNumber;
        bool calibrationDone;
    } sensorPrnu;

} CameraPipeline_t;

typedef struct
{
    float value;
} CurrentGain_t;

typedef struct
{
    CalibrationStep_t          calibrationStep;
    Equipement_t               equipement;
    CalibrationSummary_t       calibrationSummary;
    OpticalColumnCalibration_t opticalColumnCalibration;
    CameraPipeline_t           cameraPipeline;
    CurrentGain_t              currentGain;
} ConfigContent_t;

typedef struct
{
    float A1;
    float A3;
    float A5;
    float A7;
    float A9;
} LinearisationCoefficient_t;

typedef struct
{
    std::map<int, LinearisationCoefficient_t> coefficients; // index is filterIndex
} Linearisation_t;

typedef struct
{
    short X;
    short Y;
} OpticalAxis_t;

typedef struct
{
    std::map<int, std::map<int, OpticalAxis_t>> opticalAxis; // index is filterIndex; index is ndIndex
} Center_t;

typedef struct
{
    float MaximumIncidentAngle;
    int radius;
} FlatField_t;

typedef struct
{
    std::map<int, std::map<int, std::map<int, float>>> coefficients; // index is irisIndex, index is color comp, index is filterIndex
} ColorCoefComp_t;

typedef struct
{
    std::map<int, std::map<int, float>> coefficients; // index is filterIndex; index is ndIndex
} ColorCoefCorr_t;

typedef struct
{
    Summary_t       summary;
    Equipement_t    equipement;

    Linearisation_t linearisation;
    Center_t        center;

    FlatField_t     flatField;

    ColorCoefComp_t colorCoefComp;

    ColorCoefCorr_t colorCoefCorr;

} OpticalColumnConfig_t;

class CfgOutput
{
public:
    ParamData<QString> cameraCfgFileName;
    ParamData<QString> opticalColumnCfgFileName;
    ParamData<QString> flatFieldFileName;
    ParamData<float>   colorCoefCompX;
    ParamData<float>   colorCoefCompY;
    ParamData<float>   colorCoefCompZ;
};

class CfgHelper : public QObject
{
    Q_OBJECT
public:
    explicit CfgHelper(QObject *parent = 0);
    ~CfgHelper();

signals:
    void OnLog(QString message);

private:
    static CfgHelper* mInstance;

    static CfgHelper* getInstance();

    void _Log(QString message);

    bool _ReadCfgFile(QString fileName, ConfigContent_t &configContent);

    bool _ReadCalibrationStep(QDomElement& inCalibrationStep, CalibrationStep_t& calibrationStep);

    bool _ReadSummarySection(QDomElement& inSummary, Summary_t &summary);
    bool _ReadEquipmentSection(QDomElement& inEquipment, Equipement_t &equipement);
    bool _ReadOpticalColumnCalibrationSection(QDomElement& inOpticalColumnCalibration, OpticalColumnCalibration_t &opticalColumnCalibration);
    bool _ReadCameraPipeLineSection(QDomElement& inCameraPipeline, CameraPipeline_t &cameraPipeline);
    bool _ReadCurrentGain(QDomElement &inCurrentGain, CurrentGain_t &currentGain);

    bool _CreateCfgFile(QString airshipFileName, QString fileName, ConfigContent_t& configContent);

    bool _ReadSensorTemperature(QDomElement temperatureNode, SensorTemperature_t& sensorTemperature);
    void _ConvertNewPRNUArray(int16 *newArray, float *oldArray, float scaleFactor, int32 size);

    bool _GenerateCfgCamera(QString sn, QString path);
    bool _ReadCfgCameraPipeline(QString sn, QString path, CfgOutput &output);

    bool _GetCfgFile(QString path,
                     int irisIndex,
                     int filterIndex,
                     int ndIndex,
                     ConfigContent_t &configContent,
                     CfgOutput &output,
                     bool bLoadOpticalColumn,
                     bool bLoadFlatField);

    bool _ReadConfigFile(QString fileName, CfgOutput &output);

    bool _ReadLinearisationSection(QDomElement& inLinearisation, Linearisation_t& data);
    bool _ReadCenterSection(QDomElement& inEquipment, Center_t& data);
    bool _ReadColorCoefSection(QDomElement& inEquipment, ColorCoefComp_t& data);
    bool _ReadColorCoefCompSection(QDomElement& inColorCoefComp, ColorCoefComp_t& data);
    bool _ReadColorCoefCorrSection(QDomElement& inColorCoefs, ColorCoefCorr_t& data);

    bool _ReadFlatFieldSection(QDomElement& inFlatField,
                               FlatField_t& flatField);

    bool _ReadFlatFieldFile(QString fileName, std::vector<char>& data, long bufferSize);

    bool _PackCameraFile(QString sn, QString path, QString zipFilePath);

    bool _PackCameraFileListLoad(QStringList &fileArray);
    bool _PackCameraFileListSave(QStringList &fileArray);

    // config content is stored internally because the cfg is now in separate files
    ConfigContent_t mConfigContent;
    QString mConfigContentCameraSn;

    int mConfigContentFlatFieldIrisIndex;
    int mConfigContentFlatFieldFilterIndex;

    bool mOpticalColumnConfigValid;
    OpticalColumnConfig_t mOpticalColumnConfig;

public:
    static bool ReadCfgCameraPipeline(QString sn,
                                      QString path,
                                      CfgOutput &output);

    static bool GetCfgFile(QString path,
                           IrisIndex_t irisIndex,
                           Filter_t filterIndex,
                           Nd_t ndIndex,
                           ConfigContent_t &configContent,
                           CfgOutput &output,
                           bool bLoadOpticalColumn = true,
                           bool bLoadFlatField = true);

    static bool PackCameraFile(QString sn, QString path, QString zipFilePath);
};

#endif // CFGHELPER_H
