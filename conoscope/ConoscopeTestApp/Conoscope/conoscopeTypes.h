#ifndef CONOSCOPETYPES_H
#define CONOSCOPETYPES_H

#include <string>
#include <QString>

#ifdef AE_MEAS_AREA
#include <QRect>
#endif

// SETUP
#define TIMEOUT_MAX 60000
#define TEMP_MIN 15
#define TEMP_MAX 35

// MEASURE
#define EXPOSURE_TIME_MAX 980000
#define NB_ACQUISITION_MAX 25
#define NB_BINNING_MAX 2

// json return code entry values
#define RETURN_ITEM_OPTION_CFG_PATH "CfgPath"
#define RETURN_ITEM_ERROR           "Error"
#define RETURN_ITEM_MESSAGE         "Message"

#define RETURN_ITEM_CAMERA_SERIAL_NUMBER "CameraSerialNumber"
#define RETURN_ITEM_CAMERA_VERSION       "CameraVersion"

#define RETURN_ITEM_LIB_DATE        "Lib_Date"
#define RETURN_ITEM_LIB_VERSION     "Lib_Version"
#define RETURN_ITEM_LIB_NAME        "Lib_Name"

#define RETURN_ITEM_CAPTURE_FILE       "CaptureFile"

#define RETURN_ITEM_SENSOR_TEMPERATURE "SensorTemperature"
#define RETURN_ITEM_FILTER             "Filter"
#define RETURN_ITEM_ND                 "Nd"
#define RETURN_ITEM_IRIS               "Iris"

#define RETURN_ITEM_EXPOSURE_TIME_US   "ExposureUs"
#define RETURN_ITEM_NB_ACQUISITION     "NbAcquisition"
#define RETURN_ITEM_HEIGHT             "Height"
#define RETURN_ITEM_WIDTH              "Width"

#define RETURN_ITEM_MIN              "Min"
#define RETURN_ITEM_MAX              "Max"

#define RETURN_ITEM_CAMERA_CFG_FILE          "CameraCfgFile"
#define RETURN_ITEM_CAMERA_CFG_VALID         "CameraCfgValid"
#define RETURN_ITEM_OPTICAL_COLUMN_CFG_FILE  "OpticalColumnCfgFile"
#define RETURN_ITEM_OPTICAL_COLUMN_CFG_VALID "OpticalColumnCfgValid"
#define RETURN_ITEM_FLAT_FIELD_FILE          "FlatFieldFile"
#define RETURN_ITEM_FLAT_FIELD_VALID         "FlatFieldValid"

#define RETURN_ITEM_CONVERTION_FACTOR        "ConversionFactor"

#define RETURN_ITEM_CONVERTION_FACTOR_X      "ConversionFactorCompX"
#define RETURN_ITEM_CONVERTION_FACTOR_Y      "ConversionFactorCompY"
#define RETURN_ITEM_CONVERTION_FACTOR_Z      "ConversionFactorCompZ"

#define RETURN_ITEM_SETUP_STATUS_TEMPERATURE_STATE "TemperatureMonitoringState"
#define RETURN_ITEM_SETUP_STATUS_TEMPERATURE       "Temperature"
#define RETURN_ITEM_SETUP_STATUS_WHEEL_STATUS      "WheelStatus"
#define RETURN_ITEM_SETUP_STATUS_FILTER            "Filter"
#define RETURN_ITEM_SETUP_STATUS_ND                "ND"
#define RETURN_ITEM_SETUP_STATUS_IRIS              "Iris"

#define RETURN_ITEM_CONFIG_CFG_PATH                "CfgPath"
#define RETURN_ITEM_CONFIG_CAPTURE_PATH            "CapturePath"
#define RETURN_ITEM_CONFIG_AUTO_EXPOSURE           "AutoExposure"
#define RETURN_ITEM_CONFIG_AUTO_EXPOSURE_PIXEL_MAX "AutoExposurePixelMax"

#define RETURN_ITEM_CONFIG_FILE_NAME_PREPEND       "FileNamePrepend"
#define RETURN_ITEM_CONFIG_FILE_NAME_APPEND        "FileNameAppend"
#define RETURN_ITEM_CONFIG_EXPORT_FILE_NAME_FORMAT "ExportFileNameFormat"
#define RETURN_ITEM_CONFIG_EXPORT_FORMAT           "ExportFormat"
#define RETURN_ITEM_CONFIG_AE_MIN_EXPO             "AEMinExposureTimeUs"
#define RETURN_ITEM_CONFIG_AE_MAX_EXPO             "AEMaxExposureTimeUs"

#define RETURN_ITEM_CONFIG_AE_EXPO_TIME_GRANULARITY "AEExpoTimeGranularityUs"
#define RETURN_ITEM_CONFIG_AE_LEVEL_PERCENT         "AELevelPercent"
#define RETURN_ITEM_CONFIG_AE_MEAS_AREA_HEIGHT      "AEMeasAreaHeight"
#define RETURN_ITEM_CONFIG_AE_MEAS_AREA_WIDTH       "AEMeasAreaWidth"
#define RETURN_ITEM_CONFIG_AE_MEAS_AREA_X           "AEMeasAreaX"
#define RETURN_ITEM_CONFIG_AE_MEAS_AREA_Y           "AEMeasAreaY"
#define RETURN_ITEM_CONFIG_ROI_ENABLE               "bUseRoi"
#define RETURN_ITEM_CONFIG_ROI_X_LEFT               "RoiXLeft"
#define RETURN_ITEM_CONFIG_ROI_X_RIGHT              "RoiXRight"
#define RETURN_ITEM_CONFIG_ROI_Y_TOP                "RoiYTop"
#define RETURN_ITEM_CONFIG_ROI_Y_BOTTOM             "RoiYBottom"

#define RETURN_ITEM_CONFIG_DEBUG_MODE              "debugMode"
#define RETURN_ITEM_CONFIG_DEBUG_EMULATED_CAMERA   "emulatedCamera"
#define RETURN_ITEM_CONFIG_DEBUG_IMAGE_PATH        "dummyRawImagePath"

#define RETURN_ITEM_TAKT_TIME                      "TaktTimeMs"

#define RETURN_ITEM_SATURATION_FLAG                "SaturationFlag"
#define RETURN_ITEM_SATURATION_LEVEL               "SaturationLevel"

typedef enum
{
    Filter_BK7,
    Filter_Mirror,
    Filter_X,
    Filter_Xz,
    Filter_Ya,
    Filter_Yb,
    Filter_Z,
    Filter_IrCut,
    Filter_Invalid,
} Filter_t;

typedef enum
{
    Nd_0,
    Nd_1,
    Nd_2,
    Nd_3,
    Nd_4,
    Nd_Invalid,
} Nd_t;

typedef enum
{
    IrisIndex_2mm,
    IrisIndex_3mm,
    IrisIndex_4mm,
    IrisIndex_5mm,
    IrisIndex_Invalid,
} IrisIndex_t;

typedef enum
{
    ComposantType_X,
    ComposantType_Y,
    ComposantType_Z
} ComposantType_t;

typedef struct
{
    float        sensorTemperature; // temperature target
    Filter_t     eFilter;           // set filter wheel position
    Nd_t         eNd;               // set nd wheel position
    IrisIndex_t  eIris;             // indicate installed iris (this is not a command)
} SetupConfig_t;

typedef enum
{
    WheelState_Idle,
    WheelState_Success,
    WheelState_Operating,
    WheelState_Error,
} WheelState_t;

typedef enum
{
    TemperatureMonitoringState_NotStarted,
    TemperatureMonitoringState_Processing,
    TemperatureMonitoringState_Locked,
    TemperatureMonitoringState_Aborted,
    TemperatureMonitoringState_Error,
} TemperatureMonitoringState_t;

typedef struct
{
    TemperatureMonitoringState_t eTemperatureMonitoringState;
    float                        sensorTemperature;     // current temperature target
    WheelState_t                 eWheelStatus;
    Filter_t                     eFilter;
    Nd_t                         eNd;
    IrisIndex_t                  eIris;
} SetupStatus_t;

typedef struct
{
    int   exposureTimeUs;        // exposure time in micro seconds
    int   nbAcquisition;         // number of frames acquired (average)
    int   binningFactor;         //
    bool  bTestPattern;          // if set, return a test pattern returned by the sensor
} MeasureConfig_t;

#ifdef AE_MEAS_AREA
typedef struct
{
    int   exposureTimeUs;        // exposure time in micro seconds
    int   nbAcquisition;         // number of frames acquired (average)
    int   binningFactor;         //
    bool  bTestPattern;          // if set, return a test pattern returned by the sensor

    QRect cropArea;

} MeasureConfigWithCropFactor_t;

static MeasureConfigWithCropFactor_t CopyMeasureConfig(MeasureConfig_t& config)
{
    MeasureConfigWithCropFactor_t configOut;

    configOut.exposureTimeUs = config.exposureTimeUs;
    configOut.nbAcquisition  = config.nbAcquisition;
    configOut.binningFactor  = config.binningFactor;
    configOut.bTestPattern   = config.bTestPattern;

    configOut.cropArea = QRect();

    return configOut;
}

static MeasureConfig_t CopyMeasureConfig(MeasureConfigWithCropFactor_t& config)
{
    MeasureConfig_t configOut;

    configOut.exposureTimeUs = config.exposureTimeUs;
    configOut.nbAcquisition  = config.nbAcquisition;
    configOut.binningFactor  = config.binningFactor;
    configOut.bTestPattern   = config.bTestPattern;

    return configOut;
}
#endif

typedef struct
{
    int   exposureTimeUs;        // exposure time in micro seconds
    int   nbAcquisition;         // number of frames acquired (average)
    enum State_t
    {
        State_NotStarted,
        State_Process,
        State_Done,
        State_Error,
        State_Cancel,
    } state;
} MeasureStatus_t;

typedef struct
{
    bool  bBiasCompensation;
    bool  bSensorDefectCorrection;
    bool  bSensorPrnuCorrection;
    bool  bLinearisation;
    bool  bFlatField;
    bool  bAbsolute;
} ProcessingConfig_t;

typedef struct
{
    bool        debugMode;
    bool        emulateCamera;
    std::string dummyRawImagePath; // path of the image when camera is emulated
    bool        emulateWheel;
} ConoscopeDebugSettings_t;

typedef struct
{
    bool        debugMode;
    bool        emulateCamera;
    char*       dummyRawImagePath; // path of the dummy image when camera is emulated
    bool        emulateWheel;
} ConoscopeDebugSettings2_t;

typedef enum
{
    ExportFormat_bin,
    ExportFormat_bin_jpg,
    // ExportFormat_jpg,
}ExportFormat_t;

typedef struct
{
    std::string cfgPath;              // location where cfg files are stored
    std::string capturePath;          // location where captures are stored

    std::string    fileNamePrepend;
    std::string    fileNameAppend;
    std::string    exportFileNameFormat;
    ExportFormat_t exportFormat;
    int            AEMinExpoTimeUs;  // auto exposure time lower threshold
    int            AEMaxExpoTimeUs;  // auto exposure time higher threshold
    int            AEExpoTimeGranularityUs;  // auto exposure time granularity
    float          AELevelPercent;   // auto exposure saturation level

    int            AEMeasAreaHeight; // auto exposure measurement area
    int            AEMeasAreaWidth;  // auto exposure measurement area
    int            AEMeasAreaX;      // auto exposure measurement area
    int            AEMeasAreaY;      // auto exposure measurement area

    bool           bUseRoi;          // ROI of the processed data
    int            RoiXLeft;         // ROI
    int            RoiXRight;        // ROI
    int            RoiYTop;          // ROI
    int            RoiYBottom;       // ROI
} ConoscopeSettings_t;

typedef struct
{
    bool updateCaptureDate;  // time stamp is updated on each capture
    bool saveParamOnCmd;     // parameters are saved when a command is sent
} ConoscopeBehavior_t;

typedef struct
{
    char*       cfgPath;       // location where cfg files are stored
    char*       capturePath;   // location where captures are stored

    char*          fileNamePrepend;
    char*          fileNameAppend;
    char*          exportFileNameFormat;
    ExportFormat_t exportFormat;
    int            AEMinExpoTimeUs;  // auto exposure time lower threshold
    int            AEMaxExpoTimeUs;  // auto exposure time higher threshold
    int            AEExpoTimeGranularityUs;  // auto exposure time granularity
    float          AELevelPercent;   // auto exposure saturation level

    int            AEMeasAreaHeight; // auto exposure measurement area
    int            AEMeasAreaWidth;  // auto exposure measurement area
    int            AEMeasAreaX;      // auto exposure measurement area
    int            AEMeasAreaY;      // auto exposure measurement area

    bool           bUseRoi;          // ROI of the processed data
    int            RoiXLeft;         // ROI
    int            RoiXRight;        // ROI
    int            RoiYTop;          // ROI
    int            RoiYBottom;       // ROI
} ConoscopeSettings2_t;

// following structure has no interfaces
typedef struct
{
    std::string cfgFileName;   // cfg files name
    bool        cfgFileIsZip;  // indicate if the file is zipped

    int         AEMaxNbPixel;  // indicate the number of max pixels not taken into account (apply only to raw data)
#ifndef FORCE_PROCESS_DATA
    bool        measureAEProcessed;         // indicate if measurement autoExposure is done with processed data
#endif
} ConoscopeSettingsI_t;

typedef enum
{
    CfgFileState_NotDone,

    CfgFileState_Reading,
    CfgFileState_Writing,

    CfgFileState_ReadDone,
    CfgFileState_WriteDone,

    CfgFileState_ReadError,
    CfgFileState_WriteError,
} CfgFileState_t;

typedef struct
{
    CfgFileState_t eState;
    int            progress;
    qint64         elapsedTime;
    std::string    fileName;
} CfgFileStatus_t;

typedef struct
{
    float     sensorTemperature; // temperature target
    bool      bWaitForSensorTemperature;

    Nd_t         eNd;               // set nd wheel position
    IrisIndex_t  eIris;

    int       exposureTimeUs_FilterX;     // exposure time in micro seconds
    int       exposureTimeUs_FilterXz;    // exposure time in micro seconds
    int       exposureTimeUs_FilterYa;    // exposure time in micro seconds
    int       exposureTimeUs_FilterYb;    // exposure time in micro seconds
    int       exposureTimeUs_FilterZ;     // exposure time in micro seconds

    int       nbAcquisition;     // number of frames acquired (average)
    bool      bAutoExposure;
    bool      bUseExpoFile;      // exposure time are defined in a json file
    bool      bSaveCapture;      // save all the captures

    bool      bUseRoi;          // ROI of the processed data
    int       RoiXLeft;         // ROI
    int       RoiXRight;        // ROI
    int       RoiYTop;          // ROI
    int       RoiYBottom;       // ROI
} CaptureSequenceConfig_t;

typedef struct
{
    int nbSteps;
    int currentSteps;

    Filter_t  eFilter;

    enum State_t
    {
        State_NotStarted,
        State_Setup,
        State_WaitForTemp,
        State_AutoExpo,
        State_Measure,
        State_Process,
        State_Done,
        State_Error,
        State_Cancel,
    } state;
} CaptureSequenceStatus_t;

typedef enum
{
    ConoscopeEvent_CaptureSequenceDone,
    ConoscopeEvent_CaptureSequenceCancel,
    ConoscopeEvent_MeasureAEDone,
    ConoscopeEvent_MeasureAECancel,
} ConoscopeEvent_t;

typedef struct
{
    std::string fileName; // input file name
} ConvertRaw_t;

#endif // CONOSCOPETYPES_H
