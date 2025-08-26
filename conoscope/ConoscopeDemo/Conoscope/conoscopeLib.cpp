#include "conoscopeLib.h"
#include <QLibrary>
#include "toolReturnCode.h"

#define ErrorMessage_AlreadyInstanciated "Error: Already instanciated"
#define ErrorMessage_LoadingDll          "Error: Loading Dll"
#define ErrorMessage_ResolvingApi        "Error: Resolving Api"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define RESOLVE(a) Lib##a = (f_##a)conoscopelib.resolve(TOSTRING(a)); if(!Lib##a) {std::string message = ErrorMessage_ResolvingApi; message.append(" "); message.append(TOSTRING(a)); throw std::exception(message.c_str()); }

QString ConoscopeLib::_mErrorDescription = "no message";

#ifdef REMOVED
ConoscopeLib::ConoscopeLib(QObject *parent) : ConoscopeApi(parent)
#else
ConoscopeLib::ConoscopeLib(QObject *parent) : ClassCommon(parent)
#endif
{
    _Load();
}

ConoscopeLib::~ConoscopeLib()
{
    CmdTerminate();
}

#define LIB_EXECUTE(fct) QString::fromStdString(std::string(fct))

void ConoscopeLib::ErrorMessage(ToolReturnCode& errorCode)
{
    ConoscopeLib::_mErrorDescription = errorCode.GetErrorDescription();
}

QString ConoscopeLib::GetVersion()
{
    QString message = LIB_EXECUTE(LibCmdGetVersion());
    return message;
}

ClassCommon::Error ConoscopeLib::CmdOpen()
{
    QString returnValue = LIB_EXECUTE(LibCmdOpen());

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdSetup(SetupConfig_t& config)
{
    QString returnValue = LIB_EXECUTE(LibCmdSetup(config));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdSetupStatus(SetupStatus_t& status)
{
    QString returnValue = LIB_EXECUTE(LibCmdSetupStatus(status));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdMeasure(MeasureConfig_t& config, bool bDisplayLog)
{
    QString returnValue = LIB_EXECUTE(LibCmdMeasure(config));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    if((bDisplayLog == true) || (errorCode.GetError() != ClassCommon::Error::Ok))
    {
        _LogOption(errorCode);
    }

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdExportRaw()
{
    QString returnValue = LIB_EXECUTE(LibCmdExportRaw());

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    // temp
    QMap<QString, QVariant> options = errorCode.GetOption();
    QString itemName = RETURN_ITEM_CAPTURE_FILE;

    if(options.contains(itemName))
    {
        QString itemValue = "";

        itemValue = options[itemName].toString();

        // Log("",QString ("-> %1 %2").arg(itemName, -25).arg(itemValue));

        mFilePath = itemValue;
    }

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdExportProcessed(ProcessingConfig_t& config, ProcessingResult_t &result)
{
    QString returnValue = LIB_EXECUTE(LibCmdExportProcessed(config));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);

    _LogOption(errorCode);

    // temp
    QMap<QString, QVariant> options = errorCode.GetOption();
    QString itemName = RETURN_ITEM_CAPTURE_FILE;

    if(options.contains(itemName))
    {
        mFilePath = options[itemName].toString();
    }

    // fill the result structure
    result.saturationFlag = false;
    result.SaturationLevel = 0.0;

    itemName = "SaturationFlag";
    if(options.contains(itemName))
    {
        result.saturationFlag = options[itemName].toBool();
    }

    itemName = "SaturationLevel";
    if(options.contains(itemName))
    {
        result.SaturationLevel = options[itemName].toFloat();
    }

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdExportRawBuffer(std::vector<int16_t> &buffer, bool bDisplayLog)
{
    QString returnValue = LIB_EXECUTE(LibCmdExportRawBuffer(buffer));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);

    if((bDisplayLog == true) || (errorCode.GetError() != ClassCommon::Error::Ok))
    {
        _LogOption(errorCode);
    }

    // store last option returned by the API
    mLastCmdOption = errorCode.GetOption();

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdExportProcessedBuffer(ProcessingConfig_t& config, std::vector<int16_t>& buffer, bool bDisplayLog)
{
    QString returnValue = LIB_EXECUTE(LibCmdExportProcessedBuffer(config, buffer));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);

    if((bDisplayLog == true) || (errorCode.GetError() != ClassCommon::Error::Ok))
    {
        _LogOption(errorCode);
    }

    // store last option returned by the API
    mLastCmdOption = errorCode.GetOption();

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdClose()
{
    QString returnValue = LIB_EXECUTE(LibCmdClose());

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdReset()
{
    QString returnValue = LIB_EXECUTE(LibCmdReset());

    ToolReturnCode errorCode = ToolReturnCode(returnValue);

    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdSetConfig(ConoscopeSettings_t& config)
{
    static ConoscopeSettings2_t config_;

    config_.cfgPath               = (char*)config.cfgPath.c_str();
    config_.capturePath           = (char*)config.capturePath.c_str();

    config_.fileNamePrepend       = (char*)config.fileNamePrepend.c_str();
    config_.fileNameAppend        = (char*)config.fileNameAppend.c_str();
    config_.exportFileNameFormat  = (char*)config.exportFileNameFormat.c_str();
    config_.exportFormat          = config.exportFormat;
    config_.AEMinExpoTimeUs       = config.AEMinExpoTimeUs;
    config_.AEMaxExpoTimeUs       = config.AEMaxExpoTimeUs;
    config_.AEExpoTimeGranularityUs = config.AEExpoTimeGranularityUs;
    config_.AELevelPercent        = config.AELevelPercent;

    config_.AEMeasAreaHeight      = config.AEMeasAreaHeight;
    config_.AEMeasAreaWidth       = config.AEMeasAreaWidth;
    config_.AEMeasAreaX           = config.AEMeasAreaX;
    config_.AEMeasAreaY           = config.AEMeasAreaY;

    config_.bUseRoi               = config.bUseRoi;
    config_.RoiXLeft              = config.RoiXLeft;
    config_.RoiXRight             = config.RoiXRight;
    config_.RoiYTop               = config.RoiYTop;
    config_.RoiYBottom            = config.RoiYBottom;

    QString returnValue = LIB_EXECUTE(LibCmdSetConfig(config_));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode.GetOption());

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdGetConfig(ConoscopeSettings_t& config)
{
    static ConoscopeSettings2_t config_;

    QString returnValue = LIB_EXECUTE(LibCmdGetConfig(config_));

    config.cfgPath = ".\\CFG";
    config.capturePath = ".\\CAPT";
    if(config_.cfgPath != 0)
    {
        config.cfgPath          = std::string(config_.cfgPath);
    }
    if(config_.capturePath != 0)
    {
        config.capturePath      = std::string(config_.capturePath);
    }

    config.fileNamePrepend = "";
    config.fileNameAppend = "";
    config.exportFileNameFormat = "";

    if(config_.fileNamePrepend != 0)
    {
        config.fileNamePrepend = std::string(config_.fileNamePrepend);
    }

    if(config_.fileNameAppend != 0)
    {
        config.fileNameAppend = std::string(config_.fileNameAppend);
    }

    if(config_.exportFileNameFormat != 0)
    {
        config.exportFileNameFormat = std::string(config_.exportFileNameFormat);
    }

    config.exportFormat    = (ExportFormat_t)config_.exportFormat;
    config.AEMinExpoTimeUs = config_.AEMinExpoTimeUs;
    config.AEMaxExpoTimeUs = config_.AEMaxExpoTimeUs;
    config.AEExpoTimeGranularityUs = config_.AEExpoTimeGranularityUs;
    config.AELevelPercent  = config_.AELevelPercent;

    config.AEMeasAreaHeight  = config_.AEMeasAreaHeight;
    config.AEMeasAreaWidth   = config_.AEMeasAreaWidth;
    config.AEMeasAreaX       = config_.AEMeasAreaX;
    config.AEMeasAreaY       = config_.AEMeasAreaY;

    config.bUseRoi         = config_.bUseRoi;
    config.RoiXLeft        = config_.RoiXLeft;
    config.RoiXRight       = config_.RoiXRight;
    config.RoiYTop         = config_.RoiYTop;
    config.RoiYBottom      = config_.RoiYBottom;

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode.GetOption());

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdGetCmdConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig)
{
    QString returnValue = LIB_EXECUTE(LibCmdGetCmdConfig(cmdSetupConfig, cmdMeasureConfig, cmdProcessingConfig));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode.GetOption());

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdSetDebugConfig(ConoscopeDebugSettings_t& settings)
{
    static ConoscopeDebugSettings2_t settings_;

    settings_.debugMode                 = settings.debugMode;
    settings_.emulateCamera             = settings.emulateCamera;
    settings_.emulateWheel              = settings.emulateWheel;
    settings_.dummyRawImagePath = (char *)settings.dummyRawImagePath.c_str();

    QString returnValue = LIB_EXECUTE(LibCmdSetDebugConfig(settings_));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode.GetOption());

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdGetDebugConfig(ConoscopeDebugSettings_t& settings)
{
    static ConoscopeDebugSettings2_t settings_;

    QString returnValue = LIB_EXECUTE(LibCmdGetDebugConfig(settings_));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);

    if(errorCode.GetError() == ClassCommon::Error::Ok)
    {
        settings.debugMode                     = settings_.debugMode;
        settings.emulateCamera                 = settings_.emulateCamera;
        settings.emulateWheel                  = settings_.emulateWheel;
        settings.dummyRawImagePath = std::string(settings_.dummyRawImagePath);
    }

    // _LogOption(errorCode.GetOption());

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig)
{
    QString error = LIB_EXECUTE(LibCmdSetBehaviorConfig(behaviorConfig));
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdRegisterLogCallback(void (*callback)(char*))
{
    QString error = LIB_EXECUTE(LibCmdRegisterLogCallback(callback));
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString))
{
    QString error = LIB_EXECUTE(LibCmdRegisterEventCallback(callback));
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdRegisterWarningCallback(void (*callback)(QString))
{
    QString error = LIB_EXECUTE(LibCmdRegisterWarningCallback(callback));
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdCfgFileWrite()
{
    QString error = LIB_EXECUTE(LibCmdCfgFileWrite());
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdCfgFileRead()
{
    QString error = LIB_EXECUTE(LibCmdCfgFileRead());
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdCfgFileStatus(CfgFileStatus_t &status)
{
    QString error = LIB_EXECUTE(LibCmdCfgFileStatus(status));
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdGetCaptureSequence(CaptureSequenceConfig_t& config)
{
    QString error = LIB_EXECUTE(LibCmdGetCaptureSequence(config));
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdCaptureSequence(CaptureSequenceConfig_t& config)
{
    QString returnValue = LIB_EXECUTE(LibCmdCaptureSequence(config));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdCaptureSequenceCancel()
{
    QString error = LIB_EXECUTE(LibCmdCaptureSequenceCancel());
    return ToolReturnCode(error).GetError();
}

ClassCommon::Error ConoscopeLib::CmdCaptureSequenceStatus(CaptureSequenceStatus_t &status)
{
    QString returnValue = LIB_EXECUTE(LibCmdCaptureSequenceStatus(status));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdMeasureAE(MeasureConfig_t& config)
{
    QString returnValue = LIB_EXECUTE(LibCmdMeasureAE(config));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdMeasureAECancel()
{
    QString returnValue = LIB_EXECUTE(LibCmdMeasureAECancel());

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdMeasureAEStatus(MeasureStatus_t& status)
{
    QString returnValue = LIB_EXECUTE(LibCmdMeasureAEStatus(status));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdConvertRaw(ConvertRaw_t& param)
{
    QString returnValue = LIB_EXECUTE(LibCmdConvertRaw(param));

    ToolReturnCode errorCode = ToolReturnCode(returnValue);
    // _LogOption(errorCode);

    ErrorMessage(errorCode);

    return errorCode.GetError();
}

ClassCommon::Error ConoscopeLib::CmdTerminate()
{
    QString error = LIB_EXECUTE(LibCmdTerminate());
    return ToolReturnCode(error).GetError();
}

#include "appResource.h"

void ConoscopeLib::_LogOption(ToolReturnCode& errorCode)
{
    // display error code
    ClassCommon::Error eError = errorCode.GetError();
    if(eError == ClassCommon::Error::Ok)
    {
        Log("", QString("-> %1").arg("Ok", -25));
    }
    else
    {
        Log("", QString("-> %1 %3 (%2)").arg("Failed", -25).arg((int)eError).arg(errorCode.GetMessage()));
        Log("", QString("-> %1 %2").arg("", -25).arg(errorCode.GetErrorDescription()));
    }

    // display options
    QMap<QString, QVariant> options = errorCode.GetOption();

    _LogOptionItem(options, RETURN_ITEM_OPTION_CFG_PATH);
    _LogOptionItem(options, RETURN_ITEM_ERROR);
    _LogOptionItem(options, RETURN_ITEM_MESSAGE);
    _LogOptionItem(options, RETURN_ITEM_LIB_DATE);
    _LogOptionItem(options, RETURN_ITEM_LIB_VERSION);
    _LogOptionItem(options, RETURN_ITEM_LIB_NAME);
    _LogOptionItem(options, RETURN_ITEM_CAPTURE_FILE);

    // open
    _LogOptionItem(options, RETURN_ITEM_CAMERA_SERIAL_NUMBER);
    _LogOptionItem(options, RETURN_ITEM_CAMERA_VERSION);

    // setup
    _LogOptionItem(options, RETURN_ITEM_SENSOR_TEMPERATURE);
    _LogOptionItem(options, RETURN_ITEM_FILTER, &RESOURCES->mFilterToStringMap);
    _LogOptionItem(options, RETURN_ITEM_ND, &RESOURCES->mNdToStringMap);
    _LogOptionItem(options, RETURN_ITEM_IRIS, &RESOURCES->mIrisIndexToStringMap);

    // capture result
    _LogOptionItem(options, RETURN_ITEM_EXPOSURE_TIME_US);
    _LogOptionItem(options, RETURN_ITEM_NB_ACQUISITION);
    _LogOptionItem(options, RETURN_ITEM_WIDTH);
    _LogOptionItem(options, RETURN_ITEM_HEIGHT);

    _LogOptionItem(options, RETURN_ITEM_MIN);
    _LogOptionItem(options, RETURN_ITEM_MAX);

    // CFG files
    _LogOptionItem(options, RETURN_ITEM_CAMERA_CFG_FILE);
    _LogOptionItem(options, RETURN_ITEM_CAMERA_CFG_VALID);
    _LogOptionItem(options, RETURN_ITEM_OPTICAL_COLUMN_CFG_FILE);
    _LogOptionItem(options, RETURN_ITEM_OPTICAL_COLUMN_CFG_VALID);
    _LogOptionItem(options, RETURN_ITEM_FLAT_FIELD_FILE);
    _LogOptionItem(options, RETURN_ITEM_FLAT_FIELD_VALID);

    // processing result
    _LogOptionItem(options, RETURN_ITEM_CONVERTION_FACTOR);

    _LogOptionItem(options, RETURN_ITEM_CONVERTION_FACTOR_X);
    _LogOptionItem(options, RETURN_ITEM_CONVERTION_FACTOR_Y);
    _LogOptionItem(options, RETURN_ITEM_CONVERTION_FACTOR_Z);

    _LogOptionItem(options, RETURN_ITEM_SATURATION_FLAG);
    _LogOptionItem(options, RETURN_ITEM_SATURATION_LEVEL);

    _LogOptionItem(options, RETURN_ITEM_TAKT_TIME);
}

void ConoscopeLib::_LogOptionItem(QMap<QString, QVariant> options, QString itemName, std::map<int, QString> *map)
{
    if(options.contains(itemName))
    {
        QString itemValue = "";

        if(map != nullptr)
        {
            // get the enum value
            int enumValue = options[itemName].toInt();

            if(map->count(enumValue) != 0)
            {
                itemValue = QString("%1 (%2)").arg(map->at(enumValue), -15).arg(enumValue);
            }
            else
            {
                itemValue = QString("ERROR (%1)").arg(enumValue);
            }
        }
        else
        {
            itemValue = options[itemName].toString();
        }

        Log("",QString ("-> %1 %2").arg(itemName, -25).arg(itemValue));
    }
}

void ConoscopeLib::_Load()
{
    QLibrary conoscopelib("ConoscopeLib");

    if (!conoscopelib.load())
    {
        throw std::exception(ErrorMessage_LoadingDll);
    }

    RESOLVE(CmdGetVersion);

    RESOLVE(CmdOpen);
    RESOLVE(CmdSetup);
    RESOLVE(CmdSetupStatus);
    RESOLVE(CmdMeasure);
    RESOLVE(CmdExportRaw);
    RESOLVE(CmdExportRawBuffer);
    RESOLVE(CmdExportProcessed);
    RESOLVE(CmdExportProcessedBuffer);
    RESOLVE(CmdClose);
    RESOLVE(CmdReset);

    RESOLVE(CmdSetConfig);
    RESOLVE(CmdGetConfig);

    RESOLVE(CmdGetCmdConfig);
    RESOLVE(CmdSetDebugConfig);
    RESOLVE(CmdGetDebugConfig);
    RESOLVE(CmdSetBehaviorConfig);

    RESOLVE(CmdRegisterLogCallback);
    RESOLVE(CmdRegisterEventCallback);
    RESOLVE(CmdRegisterWarningCallback);

    RESOLVE(CmdCfgFileWrite);
    RESOLVE(CmdCfgFileRead);
    RESOLVE(CmdCfgFileStatus);

    RESOLVE(CmdGetCaptureSequence);
    RESOLVE(CmdCaptureSequence);
    RESOLVE(CmdCaptureSequenceCancel);
    RESOLVE(CmdCaptureSequenceStatus);

    RESOLVE(CmdMeasureAE);
    RESOLVE(CmdMeasureAECancel);
    RESOLVE(CmdMeasureAEStatus);

    RESOLVE(CmdConvertRaw);

    RESOLVE(CmdTerminate);
}

