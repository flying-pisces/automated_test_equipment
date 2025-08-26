#ifndef CONOSCOPELIB_H
#define CONOSCOPELIB_H

#include <QLibrary>

#ifdef REMOVED
#include "conoscopeApi.h"
#else
#include "classcommon.h"
#include "conoscopeTypes.h"
#endif
#include "toolReturnCode.h"

#define CMD(a) f_##a Lib##a

#ifdef REMOVED
class ConoscopeLib : public ConoscopeApi
#else
class ConoscopeLib : public ClassCommon
#endif
{
    Q_OBJECT
public:
    ConoscopeLib(QObject *parent = nullptr);

    ~ConoscopeLib();

public:
    QString GetVersion();

    ClassCommon::Error CmdOpen();

    ClassCommon::Error CmdSetup(SetupConfig_t &config);

    ClassCommon::Error CmdSetupStatus(SetupStatus_t &status);

    ClassCommon::Error CmdMeasure(MeasureConfig_t &config, bool bDisplayLog = true);

    ClassCommon::Error CmdExportRaw();

    ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config);

    ClassCommon::Error CmdExportRawBuffer(std::vector<int16_t> &buffer, bool bDisplayLog = false);

    ClassCommon::Error CmdExportProcessedBuffer(ProcessingConfig_t& config, std::vector<int16_t> &buffer, bool bDisplayLog = false);

    ClassCommon::Error CmdClose();

    ClassCommon::Error CmdReset();

    ClassCommon::Error CmdSetConfig(ConoscopeSettings_t& config);

    ClassCommon::Error CmdGetConfig(ConoscopeSettings_t& config);

    ClassCommon::Error CmdGetCmdConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig);

    ClassCommon::Error CmdSetDebugConfig(ConoscopeDebugSettings_t& settings);

    ClassCommon::Error CmdGetDebugConfig(ConoscopeDebugSettings_t& settings);

    ClassCommon::Error CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig);

    ClassCommon::Error CmdRegisterLogCallback(void (*callback)(char*));

    ClassCommon::Error CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t));

    ClassCommon::Error CmdCfgFileWrite();

    ClassCommon::Error CmdCfgFileRead();

    ClassCommon::Error CmdCfgFileStatus(CfgFileStatus_t& status);

    ClassCommon::Error CmdGetCaptureSequence(CaptureSequenceConfig_t& config);

    ClassCommon::Error CmdCaptureSequence(CaptureSequenceConfig_t& config);

    ClassCommon::Error CmdCaptureSequenceCancel();

    ClassCommon::Error CmdCaptureSequenceStatus(CaptureSequenceStatus_t& status);

    ClassCommon::Error CmdMeasureAE(MeasureConfig_t& config);

    ClassCommon::Error CmdMeasureAECancel();

    ClassCommon::Error CmdMeasureAEStatus(MeasureStatus_t& config);

    ClassCommon::Error CmdConvertRaw(ConvertRaw_t& param);

    typedef const char* (*f_CmdGetVersion)();
    CMD(CmdGetVersion);

    typedef char* (*f_CmdOpen)();
    CMD(CmdOpen);

    typedef char* (*f_CmdSetup)(SetupConfig_t& config);
    CMD(CmdSetup);

    typedef char* (*f_CmdSetupStatus)(SetupStatus_t& status);
    CMD(CmdSetupStatus);

    typedef char* (*f_CmdMeasure)(MeasureConfig_t& config);
    CMD(CmdMeasure);

    typedef char* (*f_CmdExportRaw)();
    CMD(CmdExportRaw);

    typedef char* (*f_CmdExportRawBuffer)(std::vector<int16_t>& buffer);
    CMD(CmdExportRawBuffer);

    typedef char* (*f_CmdExportProcessed)(ProcessingConfig_t& config);
    CMD(CmdExportProcessed);

    typedef char* (*f_CmdExportProcessedBuffer)(ProcessingConfig_t& config, std::vector<int16_t>& buffer);
    CMD(CmdExportProcessedBuffer);

    typedef char* (*f_CmdClose)();
    CMD(CmdClose);

    typedef char* (*f_CmdReset)();
    CMD(CmdReset);

    typedef char* (*f_CmdSetConfig)(ConoscopeSettings2_t& config);
    CMD(CmdSetConfig);

    typedef char* (*f_CmdGetConfig)(ConoscopeSettings2_t& config);
    CMD(CmdGetConfig);

    typedef char* (*f_CmdGetCmdConfig)(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig);
    CMD(CmdGetCmdConfig);

    typedef char* (*f_CmdSetDebugConfig)(ConoscopeDebugSettings2_t& conoscopeSettings);
    CMD(CmdSetDebugConfig);

    typedef char* (*f_CmdGetDebugConfig)(ConoscopeDebugSettings2_t& conoscopeSettings);
    CMD(CmdGetDebugConfig);

    typedef char* (*f_CmdSetBehaviorConfig)(ConoscopeBehavior_t &behaviorConfig);
    CMD(CmdSetBehaviorConfig);

    typedef char* (*f_CmdRegisterLogCallback)(void (*callback)(char*));
    CMD(CmdRegisterLogCallback);

    typedef char* (*f_CmdRegisterEventCallback)(void (*callback)(ConoscopeEvent_t));
    CMD(CmdRegisterEventCallback);

    typedef char* (*f_CmdCfgFileWrite)();
    CMD(CmdCfgFileWrite);

    typedef char* (*f_CmdCfgFileRead)();
    CMD(CmdCfgFileRead);

    typedef char* (*f_CmdCfgFileStatus)(CfgFileStatus_t& status);
    CMD(CmdCfgFileStatus);

    typedef char* (*f_CmdGetCaptureSequence)(CaptureSequenceConfig_t& config);
    CMD(CmdGetCaptureSequence);

    typedef char* (*f_CmdCaptureSequence)(CaptureSequenceConfig_t& config);
    CMD(CmdCaptureSequence);

    typedef char* (*f_CmdCaptureSequenceCancel)();
    CMD(CmdCaptureSequenceCancel);

    typedef char* (*f_CmdCaptureSequenceStatus)(CaptureSequenceStatus_t& status);
    CMD(CmdCaptureSequenceStatus);

    typedef char* (*f_CmdMeasureAE)(MeasureConfig_t& config);
    CMD(CmdMeasureAE);

    typedef char* (*f_CmdMeasureAECancel)();
    CMD(CmdMeasureAECancel);

    typedef char* (*f_CmdMeasureAEStatus)(MeasureStatus_t& config);
    CMD(CmdMeasureAEStatus);

    typedef char* (*f_CmdConvertRaw)(ConvertRaw_t& param);
    CMD(CmdConvertRaw);

    QVariant GetOption(QString itemName)
    {
        QVariant value;
        value = mLastCmdOption[itemName];
        return value;
    }

    QString GetFilePath()
    {
        return mFilePath;
    }

protected:
    QMap<QString, QVariant> mLastCmdOption;
    QString mFilePath;

private:
    ClassCommon::Error CmdTerminate();

    typedef char* (*f_CmdTerminate)();
    CMD(CmdTerminate);

    void _LogOption(ToolReturnCode &errorCode);
    void _LogOptionItem(QMap<QString, QVariant> options, QString itemName, std::map<int, QString>* map = nullptr);

    void _Load();
};

#endif // CONOSCOPELIB_H
