#include "conoscopeLib.h"

#include <QString>

#include "configuration.h"
#include "ConoscopeApp.h"
#include "toolReturnCode.h"

#include "ConoscopeResource.h"

#define _Log(a)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

#define USE_QCORE
#ifdef USE_QCORE
static int _LaunchApplication();
static void _QuitApplication();
#endif

static ConoscopeApp* _GetInstance();

static void _DeleteInstance();

#define CONOSCOPE(instance) ConoscopeApp* instance = _GetInstance()

#define RETURN_NO_TAKT return _GetReturn
#define RETURN jsonError.SetOption(RETURN_ITEM_TAKT_TIME, RESOURCE->TaktTimeMs()); return _GetReturn
#define RETURN_ERROR(err) ToolReturnCode jsonError = ToolReturnCode(err); jsonError.SetOption(RETURN_ITEM_TAKT_TIME, RESOURCE->TaktTimeMs()); return _GetReturn(jsonError.GetJsonCode())

#define LOG_HEADER() _Log(QString("%1...").arg(__func__))
#define LOG_TRAILER() _Log(QString("...%1").arg(__func__)); _Log("")

#define LOG_APP_HEADER "[Lib]"
#define LogInApp(text) RESOURCE->Log(QString("%1 %2").arg(LOG_APP_HEADER).arg(text));

static char * cstr = new char [0];

static void _JsonOutput(Conoscope::CmdExportRawOutput_t output, ToolReturnCode& jsonError);

static void _JsonOutput(Conoscope::CmdExportProcessedOutput_t output, ToolReturnCode& jsonError);

static const char* _GetReturn(QString message)
{
    std::string str = message.toStdString();

    delete[] cstr;
    cstr = new char [str.length()+1];

    strcpy_s (cstr, str.length()+1, str.c_str());

    return cstr;
}

#include "QLibrary"

static ConoscopeApp* _instance = NULL;

#ifdef USE_QCORE

#include <QCoreApplication>
static QCoreApplication* mApplication;

int _LaunchApplication()
{
    int status;

    int argc = 0;
    char *argv[1];

#ifdef MEASUREAE_WA
    CONOSCOPE(instance);
    instance->SetExternalCoreLoop();
#endif

    // create a qcore application in order to use thread
    mApplication = new QCoreApplication(argc, argv);

    status = mApplication->exec();
    return status;
}

void _QuitApplication()
{
    mApplication->quit();
}
#endif

ConoscopeApp * _GetInstance()
{
    if(_instance == NULL)
    {
        _instance = new ConoscopeApp();

        _instance->Start();
    }

    return _instance;
}

void _DeleteInstance()
{
    if(_instance != NULL)
    {
        _instance->Stop();

        delete _instance;
        _instance = NULL;
    }
}

#ifdef USE_QCORE
const char* CmdRunApp()
{
    ToolReturnCode eError = ToolReturnCode(ClassCommon::Error::Ok);

    _LaunchApplication();

    RETURN_NO_TAKT(eError.GetJsonCode());
}

const char* CmdQuitApp()
{
    ToolReturnCode eError = ToolReturnCode(ClassCommon::Error::Ok);

    _QuitApplication();

    RETURN_NO_TAKT(eError.GetJsonCode());
}
#endif

const char* CmdGetVersion()
{
    ToolReturnCode eError = ToolReturnCode(ClassCommon::Error::Ok);

    eError.SetOption(RETURN_ITEM_LIB_DATE, RELEASE_DATE);
    eError.SetOption(RETURN_ITEM_LIB_VERSION, VERSION_STR);
    eError.SetOption(RETURN_ITEM_LIB_NAME, APPLICATION_NAME);

    // retrieve the version of the pipeline
    CONOSCOPE(instance);
    QString pipelineVersion = instance->CmdGetPipelineVersion();

    ToolReturnCode pipelineError = ToolReturnCode(pipelineVersion);

    QMap<QString, QVariant> pipelineOptions = pipelineError.GetOption();

    QMapIterator<QString, QVariant> iter(pipelineOptions);
    while (iter.hasNext())
    {
        iter.next();

        QString iKey = iter.key();
        QString iValue = iter.value().toString();

        eError.SetOption(QString("Pipeline_%1").arg(iKey), iValue);
    }

    RETURN_NO_TAKT(eError.GetJsonCode());
}

#define ERROR_DEBUG(fct)

const char* CmdOpen()
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    Conoscope::CmdOpenOutput_t output;
    eError = instance->CmdOpen(output);

    LOG_TRAILER();

    ERROR_DEBUG(CmdOpen);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    // cfgPath is empty in case the camera is emulated
    if(!output.cfgPath.isEmpty())
    {
        jsonError.SetOption(RETURN_ITEM_OPTION_CFG_PATH, output.cfgPath);
    }

    if(!output.cameraSn.isEmpty())
    {
        jsonError.SetOption(RETURN_ITEM_CAMERA_SERIAL_NUMBER, output.cameraSn);
    }

    if(!output.cameraVersion.isEmpty())
    {
        jsonError.SetOption(RETURN_ITEM_CAMERA_VERSION, output.cameraVersion);
    }

    RETURN(jsonError.GetJsonCode());
}

const char* CmdSetup(SetupConfig_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    eError = instance->CmdSetup(config);

    LOG_TRAILER();

    ERROR_DEBUG(CmdSetup);

    RETURN_ERROR(eError);
}

const char* CmdSetupStatus(SetupStatus_t &status)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdSetupStatus(status);

    LOG_TRAILER();

    ERROR_DEBUG(CmdSetupStatus);

    RETURN_ERROR(eError);
}

const char* CmdMeasure(MeasureConfig_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    MeasureConfigWithCropFactor_t configWithCropFactor = CopyMeasureConfig(config);

    configWithCropFactor.bAeEnable        = false;
    configWithCropFactor.AEMeasAreaHeight = 0;
    configWithCropFactor.AEMeasAreaWidth  = 0;
    configWithCropFactor.AEMeasAreaX      = 0;
    configWithCropFactor.AEMeasAreaY      = 0;

    eError = instance->CmdMeasure(configWithCropFactor);

    LOG_TRAILER();

    ERROR_DEBUG(CmdMeasure);

    RETURN_ERROR(eError);
}

const char* CmdExportRaw()
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;
    Conoscope::CmdExportRawOutput_t output;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    eError = instance->CmdExportRaw(output);

    LOG_TRAILER();

    ERROR_DEBUG(CmdExportRaw);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    // populate
    jsonError.SetOption(RETURN_ITEM_CAPTURE_FILE       , output.fileName);

#ifdef SATURATION_FLAG_RAW
    jsonError.SetOption(RETURN_ITEM_SATURATION_FLAG, (int)output.saturationFlag);
    jsonError.SetOption(RETURN_ITEM_SATURATION_LEVEL, (float)output.saturationLevel);
#endif

    _JsonOutput(output, jsonError);

    RETURN(jsonError.GetJsonCode());
}

const char *CmdExportRawBuffer(std::vector<uint16_t> &buffer)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    Conoscope::CmdExportRawOutput_t output;
    Conoscope::CmdExportAdditionalInfo_t additionalInfo;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    eError = instance->CmdExportRaw(buffer, output, additionalInfo);

    LOG_TRAILER();

    ERROR_DEBUG(CmdExportRawBuffer);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    _JsonOutput(output, jsonError);

    jsonError.SetOption(RETURN_ITEM_MIN, (int)output.min);
    jsonError.SetOption(RETURN_ITEM_MAX, (int)output.max);

    if(additionalInfo.bAeEnable == true)
    {
        jsonError.SetOption(RETURN_ITEM_AE_ROI_WIDTH,  (int)additionalInfo.AEMeasAreaWidth);
        jsonError.SetOption(RETURN_ITEM_AE_ROI_HEIGHT, (int)additionalInfo.AEMeasAreaHeight);
        jsonError.SetOption(RETURN_ITEM_AE_ROI_X,      (int)additionalInfo.AEMeasAreaX);
        jsonError.SetOption(RETURN_ITEM_AE_ROI_Y,      (int)additionalInfo.AEMeasAreaY);
    }

#ifdef SATURATION_FLAG_RAW
    jsonError.SetOption(RETURN_ITEM_SATURATION_FLAG, (int)output.saturationFlag);
    jsonError.SetOption(RETURN_ITEM_SATURATION_LEVEL, (float)output.saturationLevel);
#endif

    RETURN(jsonError.GetJsonCode());
}

const char* CmdExportProcessed(ProcessingConfig_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;
    Conoscope::CmdExportProcessedOutput_t output;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdExportProcessed(config, output);

    if(eError != ClassCommon::Error::Ok)
    {
        // emit a warning
        RESOURCE->SendWarning();
    }

    LOG_TRAILER();

    ERROR_DEBUG(CmdExportProcessed);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    // populate
    jsonError.SetOption(RETURN_ITEM_CAPTURE_FILE       , output.fileName);

    jsonError.SetOption(RETURN_ITEM_SATURATION_FLAG, (int)output.saturationFlag);
    jsonError.SetOption(RETURN_ITEM_SATURATION_LEVEL, (float)output.saturationLevel);

    _JsonOutput(output, jsonError);

    RETURN(jsonError.GetJsonCode());
}

const char *CmdExportProcessedBuffer(ProcessingConfig_t& config, std::vector<int16_t>& buffer)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;
    Conoscope::CmdExportProcessedOutput_t output;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdExportProcessed(config, buffer, output);

    if(eError != ClassCommon::Error::Ok)
    {
        // emit a warning
        RESOURCE->SendWarning();
    }

    LOG_TRAILER();

    ERROR_DEBUG(CmdExportProcessed);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    // populate
    _JsonOutput(output, jsonError);

    jsonError.SetOption(RETURN_ITEM_MIN, (int)output.min);
    jsonError.SetOption(RETURN_ITEM_MAX, (int)output.max);

    jsonError.SetOption(RETURN_ITEM_SATURATION_FLAG, (int)output.saturationFlag);
    jsonError.SetOption(RETURN_ITEM_SATURATION_LEVEL, (float)output.saturationLevel);

    RETURN(jsonError.GetJsonCode());
}

const char* CmdClose()
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdClose();

    LOG_TRAILER();

    ERROR_DEBUG(CmdClose);

    RETURN_ERROR(eError);
}

const char* CmdReset()
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    QString cfgPath;
    eError = instance->CmdReset(cfgPath);

    LOG_TRAILER();

    ERROR_DEBUG(CmdReset);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    jsonError.SetOption(RETURN_ITEM_OPTION_CFG_PATH, cfgPath);

    RETURN(jsonError.GetJsonCode());
}

const char *CmdSetConfig(ConoscopeSettings2_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    ConoscopeSettings_t _config;

    _config.cfgPath              = std::string(config.cfgPath);
    _config.capturePath          = std::string(config.capturePath);

    _config.fileNamePrepend      = std::string(config.fileNamePrepend);
    _config.fileNameAppend       = std::string(config.fileNameAppend);
    _config.exportFileNameFormat = std::string(config.exportFileNameFormat);
    _config.exportFormat         = config.exportFormat;
    _config.AEMinExpoTimeUs      = config.AEMinExpoTimeUs;
    _config.AEMaxExpoTimeUs      = config.AEMaxExpoTimeUs;
    _config.AEExpoTimeGranularityUs = config.AEExpoTimeGranularityUs;
    _config.AELevelPercent       = config.AELevelPercent;

    _config.AEMeasAreaHeight     = config.AEMeasAreaHeight;
    _config.AEMeasAreaWidth      = config.AEMeasAreaWidth;
    _config.AEMeasAreaX          = config.AEMeasAreaX;
    _config.AEMeasAreaY          = config.AEMeasAreaY;

    _config.bUseRoi              = config.bUseRoi;
    _config.RoiXLeft             = config.RoiXLeft;
    _config.RoiXRight            = config.RoiXRight;
    _config.RoiYTop              = config.RoiYTop;
    _config.RoiYBottom           = config.RoiYBottom;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdSetConfig(_config);

    LOG_TRAILER();

    ERROR_DEBUG(CmdSetConfig);

    RETURN_ERROR(eError);
}

const char *CmdGetConfig(ConoscopeSettings2_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    static ConoscopeSettings_t _config;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdGetConfig(_config);

    config.cfgPath               = (char*)_config.cfgPath.c_str();
    config.capturePath           = (char*)_config.capturePath.c_str();

    config.fileNamePrepend       = (char*)_config.fileNamePrepend.c_str();
    config.fileNameAppend        = (char*)_config.fileNameAppend.c_str();
    config.exportFileNameFormat  = (char*)_config.exportFileNameFormat.c_str();
    config.exportFormat          = _config.exportFormat;
    config.AEMinExpoTimeUs       = _config.AEMinExpoTimeUs;
    config.AEMaxExpoTimeUs       = _config.AEMaxExpoTimeUs;
    config.AEExpoTimeGranularityUs = _config.AEExpoTimeGranularityUs;
    config.AELevelPercent        = _config.AELevelPercent;

    config.AEMeasAreaHeight      = _config.AEMeasAreaHeight;
    config.AEMeasAreaWidth       = _config.AEMeasAreaWidth;
    config.AEMeasAreaX           = _config.AEMeasAreaX;
    config.AEMeasAreaY           = _config.AEMeasAreaY;

    config.bUseRoi               = _config.bUseRoi;
    config.RoiXLeft              = _config.RoiXLeft;
    config.RoiXRight             = _config.RoiXRight;
    config.RoiYTop               = _config.RoiYTop;
    config.RoiYBottom            = _config.RoiYBottom;

    LOG_TRAILER();

    ERROR_DEBUG(CmdGetConfig);

    ToolReturnCode jsonError = ToolReturnCode(eError);

    // populate
    jsonError.SetOption(RETURN_ITEM_CONFIG_CFG_PATH,     CONVERT_TO_QSTRING(_config.cfgPath));
    jsonError.SetOption(RETURN_ITEM_CONFIG_CAPTURE_PATH, CONVERT_TO_QSTRING(_config.capturePath));

    jsonError.SetOption(RETURN_ITEM_CONFIG_FILE_NAME_PREPEND, config.fileNamePrepend);
    jsonError.SetOption(RETURN_ITEM_CONFIG_FILE_NAME_APPEND, config.fileNameAppend);
    jsonError.SetOption(RETURN_ITEM_CONFIG_EXPORT_FILE_NAME_FORMAT, config.exportFileNameFormat);
    jsonError.SetOption(RETURN_ITEM_CONFIG_EXPORT_FORMAT, config.exportFormat);
    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_MIN_EXPO, config.AEMinExpoTimeUs);
    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_MAX_EXPO, config.AEMaxExpoTimeUs);

    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_EXPO_TIME_GRANULARITY, config.AEExpoTimeGranularityUs);
    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_LEVEL_PERCENT, config.AELevelPercent);

    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_MEAS_AREA_HEIGHT, config.AEMeasAreaHeight);
    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_MEAS_AREA_WIDTH, config.AEMeasAreaWidth);
    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_MEAS_AREA_X, config.AEMeasAreaX);
    jsonError.SetOption(RETURN_ITEM_CONFIG_AE_MEAS_AREA_Y, config.AEMeasAreaY);

    jsonError.SetOption(RETURN_ITEM_CONFIG_ROI_ENABLE, config.bUseRoi);
    jsonError.SetOption(RETURN_ITEM_CONFIG_ROI_X_LEFT, config.RoiXLeft);
    jsonError.SetOption(RETURN_ITEM_CONFIG_ROI_X_RIGHT, config.RoiXRight);
    jsonError.SetOption(RETURN_ITEM_CONFIG_ROI_Y_TOP, config.RoiYTop);
    jsonError.SetOption(RETURN_ITEM_CONFIG_ROI_Y_BOTTOM, config.RoiYBottom);

    RETURN(jsonError.GetJsonCode());
}

const char *CmdGetCmdConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdGetCmdConfig(cmdSetupConfig,
                                       cmdMeasureConfig,
                                       cmdProcessingConfig);

    LOG_TRAILER();

    ERROR_DEBUG(CmdGetConfig);

    RETURN_ERROR(eError);
}

const char *CmdSetDebugConfig(ConoscopeDebugSettings2_t &conoscopeConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    ConoscopeDebugSettings_t conoscopeConfig_;

    conoscopeConfig_.debugMode = conoscopeConfig.debugMode;
    conoscopeConfig_.emulateCamera = conoscopeConfig.emulateCamera;
    conoscopeConfig_.dummyRawImagePath = std::string(conoscopeConfig.dummyRawImagePath);
    conoscopeConfig_.emulateWheel = conoscopeConfig.emulateWheel;

    eError = instance->CmdSetDebugConfig(conoscopeConfig_);

    LOG_TRAILER();

    ERROR_DEBUG(CmdGetDebugConfig);

    RETURN_ERROR(eError);
}

const char *CmdGetDebugConfig(ConoscopeDebugSettings2_t &conoscopeConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    static ConoscopeDebugSettings_t conoscopeConfig_;

    eError = instance->CmdGetDebugConfig(conoscopeConfig_);

    conoscopeConfig.debugMode = conoscopeConfig_.debugMode;
    conoscopeConfig.emulateCamera = conoscopeConfig_.emulateCamera;
    conoscopeConfig.dummyRawImagePath = (char*)conoscopeConfig_.dummyRawImagePath.c_str();
    conoscopeConfig.emulateWheel = conoscopeConfig_.emulateWheel;

    LOG_TRAILER();

    ToolReturnCode jsonError = ToolReturnCode(eError);

    // populate
    jsonError.SetOption(RETURN_ITEM_CONFIG_DEBUG_MODE, conoscopeConfig_.debugMode);
    jsonError.SetOption(RETURN_ITEM_CONFIG_DEBUG_EMULATED_CAMERA, conoscopeConfig_.emulateCamera);
    jsonError.SetOption(RETURN_ITEM_CONFIG_DEBUG_IMAGE_PATH, CONVERT_TO_QSTRING(conoscopeConfig_.dummyRawImagePath));
    jsonError.SetOption(RETURN_ITEM_CONFIG_DEBUG_EMULATED_WHEEL, conoscopeConfig_.emulateWheel);

    RETURN(jsonError.GetJsonCode());
}

const char *CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);

    eError = instance->CmdSetBehaviorConfig(behaviorConfig);

    LOG_TRAILER();

    ERROR_DEBUG(CmdSetBehaviorConfig);

    RETURN_ERROR(eError);
}

const char *CmdRegisterLogCallback(void (*callback)(char*))
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    CONOSCOPE(instance);
    instance->CmdRegisterLogCallback(callback);

    RETURN_ERROR(eError);
}

const char *CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString))
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    CONOSCOPE(instance);
    instance->CmdRegisterEventCallback(callback);

    RETURN_ERROR(eError);
}

const char *CmdRegisterWarningCallback(void (*callback)(QString))
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    CONOSCOPE(instance);
    instance->CmdRegisterWarningCallback(callback);

    RETURN_ERROR(eError);
}

const char *CmdCfgFileWrite()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdCfgFileWrite();

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdCfgFileRead()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdCfgFileRead();

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdCfgFileStatus(CfgFileStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdCfgFileStatus(status);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdGetCaptureSequence(CaptureSequenceConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdGetCaptureSequenceConfig(config);

    ERROR_DEBUG(CmdGetCaptureSequence);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdCaptureSequence(CaptureSequenceConfig_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdCaptureSequence(config);

    ERROR_DEBUG(CmdCaptureSequence);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdCaptureSequenceCancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdCaptureSequenceCancel();

    ERROR_DEBUG(CmdCaptureSequenceCancel);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdCaptureSequenceStatus(CaptureSequenceStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdCaptureSequenceStatus(status);

    ERROR_DEBUG(CmdCaptureSequenceStatus);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdMeasureAE(MeasureConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    RESOURCE->TaktTimeStart();
    CONOSCOPE(instance);
    eError = instance->CmdMeasureAE(config);

    ERROR_DEBUG(CmdMeasureAE);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdMeasureAECancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdMeasureAECancel();

    ERROR_DEBUG(CmdMeasureAECancel);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdMeasureAEStatus(MeasureStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdMeasureAEStatus(status);

    ERROR_DEBUG(CmdMeasureAEStatus);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

const char *CmdConvertRaw(ConvertRaw_t& param)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    CONOSCOPE(instance);
    eError = instance->CmdConvertRaw(param);

    ERROR_DEBUG(CmdConvertRaw);

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

// terminate the dll
const char *CmdTerminate()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_HEADER();

    _DeleteInstance();

    LOG_TRAILER();

    RETURN_ERROR(eError);
}

static void _JsonOutput(Conoscope::CmdExportRawOutput_t output, ToolReturnCode& jsonError)
{
    // setup
    jsonError.SetOption(RETURN_ITEM_SENSOR_TEMPERATURE , output.sensorTemperature);
    jsonError.SetOption(RETURN_ITEM_FILTER             , (int)output.eFilter);
    jsonError.SetOption(RETURN_ITEM_ND                 , (int)output.eNd);
    jsonError.SetOption(RETURN_ITEM_IRIS               , (int)output.eIris);

    // measure
    jsonError.SetOption(RETURN_ITEM_EXPOSURE_TIME_US, (int)output.exposureTimeUs);
    jsonError.SetOption(RETURN_ITEM_NB_ACQUISITION,   (int)output.nbAcquisition);
    jsonError.SetOption(RETURN_ITEM_HEIGHT,           (int)output.height);
    jsonError.SetOption(RETURN_ITEM_WIDTH,            (int)output.width);

}

static void _JsonOutput(Conoscope::CmdExportProcessedOutput_t output, ToolReturnCode& jsonError)
{
    // setup
    jsonError.SetOption(RETURN_ITEM_SENSOR_TEMPERATURE , output.sensorTemperature);
    jsonError.SetOption(RETURN_ITEM_FILTER             , (int)output.eFilter);
    jsonError.SetOption(RETURN_ITEM_ND                 , (int)output.eNd);
    jsonError.SetOption(RETURN_ITEM_IRIS               , (int)output.eIris);

    // measure
    jsonError.SetOption(RETURN_ITEM_EXPOSURE_TIME_US, (int)output.exposureTimeUs);
    jsonError.SetOption(RETURN_ITEM_NB_ACQUISITION,   (int)output.nbAcquisition);
    jsonError.SetOption(RETURN_ITEM_HEIGHT,           (int)output.height);
    jsonError.SetOption(RETURN_ITEM_WIDTH,            (int)output.width);

    // cfg files
    jsonError.SetOption(RETURN_ITEM_CAMERA_CFG_FILE,          output.cameraCfgFileName.GetValue());
    jsonError.SetOption(RETURN_ITEM_CAMERA_CFG_VALID,         output.cameraCfgFileName.GetValid());
    jsonError.SetOption(RETURN_ITEM_OPTICAL_COLUMN_CFG_FILE,  output.opticalColumnCfgFileName.GetValue());
    jsonError.SetOption(RETURN_ITEM_OPTICAL_COLUMN_CFG_VALID, output.opticalColumnCfgFileName.GetValid());
    jsonError.SetOption(RETURN_ITEM_FLAT_FIELD_FILE,          output.flatFieldFileName.GetValue());
    jsonError.SetOption(RETURN_ITEM_FLAT_FIELD_VALID,         output.flatFieldFileName.GetValid());

    // processing result
    jsonError.SetOption(RETURN_ITEM_CONVERTION_FACTOR_X,      output.conversionFactorCompX);
    jsonError.SetOption(RETURN_ITEM_CONVERTION_FACTOR_Y,      output.conversionFactorCompY);
    jsonError.SetOption(RETURN_ITEM_CONVERTION_FACTOR_Z,      output.conversionFactorCompZ);
}
