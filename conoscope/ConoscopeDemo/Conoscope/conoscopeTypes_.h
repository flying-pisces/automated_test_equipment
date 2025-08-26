#ifndef CONOSCOPETYPES_H
#define CONOSCOPETYPES_H

#include <string>

// SETUP
#define TIMEOUT_MAX 60000
#define TEMP_MIN 15
#define TEMP_MAX 35
#define FILT_WHEEL_MAX 5
#define ND_WHEEL_MAX 4
#define IRIS_MAX 3

// MEASURE
#define EXPOSURE_TIME_MAX 980000
#define NB_ACQUISITION_MAX 25
#define NB_BINNING_MAX 2

// json return code entry values
#define RETURN_ITEM_OPTION_CFG_PATH "CfgPath"
#define RETURN_ITEM_ERROR           "Error"
#define RETURN_ITEM_MESSAGE         "Message"

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
#define RETURN_ITEM_FLAT_FIELD_CFG_FILE      "FlatFieldCfgFile"
#define RETURN_ITEM_FLAT_FIELD_CFG_VALID     "FlatFieldCfgValid"
#define RETURN_ITEM_FLAT_FIELD_FILE          "FlatFieldFile"
#define RETURN_ITEM_FLAT_FIELD_VALID         "FlatFieldValid"

#define RETURN_ITEM_CONVERTION_FACTOR        "ConversionFactor"

#define LED_SOURCE_FILTER

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
    IrisIndex_02,
    IrisIndex_03,
    IrisIndex_04,
    IrisIndex_05,
    IrisIndex_Invalid,
} IrisIndex_t;

#ifdef LED_SOURCE_FILTER
typedef enum
{
    LedSourceFilter_White,
    LedSourceFilter_Red,
    LedSourceFilter_Orange,
    LedSourceFilter_Yellow,
    LedSourceFilter_Green1,
    LedSourceFilter_Green2,
    LedSourceFilter_Green3,
    LedSourceFilter_Green4,
    LedSourceFilter_Cyan,
    LedSourceFilter_Blue,
    LedSourceFilter_Magenta,
    LedSourceFilter_Invalid,
} LedSourceFilter_t;
#endif

typedef struct
{
    float        sensorTemperature; // temperature target
    Filter_t     eFilter;           // set filter wheel position
    Nd_t         eNd;               // set nd wheel position
    IrisIndex_t  eIris;             // indicate installed iris (this is not a command)

#ifdef LED_SOURCE_FILTER
    LedSourceFilter_t eLedSourceFilter; // configuration of the source led in order to set the capture name
#endif

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

typedef enum
{

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
    std::string cfgPath;
    std::string capturePath;
    bool        autoExposure;         // enable auto exposure (or not)
    float       autoExposurePixelMax; // indicate the max value of a pixel in autoexposure algo
} ConoscopeSettings_t;

typedef struct
{
    bool        debugMode;
    bool        emulateCamera;
    std::string dummyRawImagePath; // path of the dymmy image when camera is emulated
} ConoscopeDebugSettings_t;

typedef struct
{
    bool bEnableDebug;                  /* allow debug: when set to false, none of following parameters are used */

    bool bForceErrorCmdOpen;            /* CmdOpen          returns an error when called */
    bool bForceErrorCmdSetup;           /* CmdSetup         returns an error when called */
    bool bForceErrorCmdSetupStatus;     /* CmdSetupStatus   returns an error when called */
    bool bForceErrorCmdMeasure;         /* CmdMeasure       returns an error when called */
    bool bForceErrorCmdMeasureStatus;   /* CmdMeasureStatus returns an error when called */
    bool bForceErrorCmdExportRaw;       /* CmdExportRaw     returns an error when called */
    bool bForceErrorCmdExportProcessed; /* CmdExportProcess returns an error when called */
    bool bForceErrorCmdClose;           /* CmdClose         returns an error when called */
    bool bForceErrorCmdReset;           /* CmdReset         returns an error when called */
    bool bForceErrorCmdGetConfig;       /* CmdGetConfig     returns an error when called */

} DebugMode_t;

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

#endif // CONOSCOPETYPES_H
