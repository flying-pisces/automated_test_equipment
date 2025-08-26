#ifdef REMOVED
#ifndef CONOSCOPEAPI_H
#define CONOSCOPEAPI_H

#include "classcommon.h"
#include "conoscopeTypes.h"

class ConoscopeApi : public ClassCommon
{
public:
    ConoscopeApi(QObject *parent);

    ~ConoscopeApi();

public:
    virtual QString GetVersion();

    virtual ClassCommon::Error CmdOpen() = 0;

    virtual ClassCommon::Error CmdSetup(SetupConfig_t& config) = 0;

    virtual ClassCommon::Error CmdSetupOptional(SetupOptionalConfig_t& config) = 0;

    virtual ClassCommon::Error CmdSetupStatus(SetupStatus_t& status) = 0;

    virtual ClassCommon::Error CmdMeasure(MeasureConfig_t& config, bool bDisplayLog = true) = 0;

    virtual ClassCommon::Error CmdExportRaw() = 0;

    virtual ClassCommon::Error CmdExportProcessed(ProcessingConfig_t& status) = 0;

    virtual ClassCommon::Error CmdExportRawBuffer(std::vector<int16_t>& buffer, bool bDisplayLog) = 0;

    virtual ClassCommon::Error CmdExportProcessedBuffer(ProcessingConfig_t& config, std::vector<int16_t>& buffer, bool bDisplayLog) = 0;

    virtual ClassCommon::Error CmdClose() = 0;

    virtual ClassCommon::Error CmdReset() = 0;

    virtual ClassCommon::Error CmdSetConfig(ConoscopeSettings_t& config) = 0;

    virtual ClassCommon::Error CmdGetConfig(ConoscopeSettings_t& config) = 0;

    virtual ClassCommon::Error CmdGetCmdConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig) = 0;

    virtual ClassCommon::Error CmdSetDebugConfig(ConoscopeDebugSettings_t& conoscopeSettings) = 0;

    virtual ClassCommon::Error CmdGetDebugConfig(ConoscopeDebugSettings_t& conoscopeSettings) = 0;

    virtual ClassCommon::Error CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig) = 0;

    virtual ClassCommon::Error CmdRegisterLogCallback(void (*callback)(char*)) = 0;

    virtual ClassCommon::Error CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t)) = 0;

    virtual ClassCommon::Error CmdCfgFileWrite() = 0;

    virtual ClassCommon::Error CmdCfgFileRead() = 0;

    virtual ClassCommon::Error CmdCfgFileStatus(CfgFileStatus_t& status) = 0;

    virtual ClassCommon::Error CmdGetCaptureSequence(CaptureSequenceConfig_t& config) = 0;

    virtual ClassCommon::Error CmdCaptureSequence(CaptureSequenceConfig_t& config) = 0;

    virtual ClassCommon::Error CmdCaptureSequenceCancel() = 0;

    virtual ClassCommon::Error CmdCaptureSequenceStatus(CaptureSequenceStatus_t& status) = 0;

    QString mFilePath;
    int     mHeight;
    int     mWidth;

    QVariant GetOption(QString itemName)
    {
        QVariant value;
        value = mLastCmdOption[itemName];
        return value;
    }

protected:
    QMap<QString, QVariant> mLastCmdOption;
};

#endif // CONOSCOPEAPI_H
#endif
