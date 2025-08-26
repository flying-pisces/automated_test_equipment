#ifndef CONOSCOPEAPP_H
#define CONOSCOPEAPP_H

#include <string>

#include "Conoscope.h"

#include "ConoscopeAppWorker.h"
#include "conoscopeTypes.h"

#define MEASUREAE_WA

class ConoscopeApp : public ClassThreadCommon
{
    Q_OBJECT

public:
    enum class Event {
        CmdOpen,
        CmdSetup,
        CmdMeasure,
        CmdExportRaw,
        CmdExportProcessed,
        CmdClose,
        CmdReset,

        CmdCfgFileWrite,
        CmdCfgFileRead,
        CmdCfgFileStatus,

        CmdCaptureSequence,
        CmdCaptureSequenceCancel,
        CmdCaptureSequenceDone,

        CmdMeasureAE,
        CmdMeasureAECancel,
        CmdMeasureAEDone,

        Error,
    };
    Q_ENUM(Event)

    enum class State {
        Undefined,  /*< worker is not started */
        Idle,       /*< module is started */

        Opened,
        Ready,
        CaptureDone,

        CmdOpenProcessing,
        CmdSetupProcessing,
        CmdMeasureProcessing,
        CmdExportRawProcessing,
        CmdExportProcessedProcessing,
        CmdCloseProcessing,
        CmdResetProcessing,

        CmdCfgFileWriteProcessing,
        CmdCfgFileReadProcessing,

        CmdCapturingSequence,

        CmdMeasuringAE,

        Error,
    };
    Q_ENUM(State)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = ConoscopeApp::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    ConoscopeApp(QObject *parent = nullptr);

    ~ConoscopeApp();

    void Start();

    ClassCommon::Error Stop();

    void StopThread();

#ifdef MEASUREAE_WA
    void SetExternalCoreLoop()
    {
        mExternalCoreLoop = true;
    }
#endif

    QString CmdGetPipelineVersion();

    ClassCommon::Error CmdOpen(Conoscope::CmdOpenOutput_t& output);
    ClassCommon::Error CmdSetup(SetupConfig_t &config);
    ClassCommon::Error CmdSetupStatus(SetupStatus_t &status);
    ClassCommon::Error CmdMeasure(MeasureConfigWithCropFactor_t &config);

    ClassCommon::Error CmdExportRaw(Conoscope::CmdExportRawOutput_t& output);
    ClassCommon::Error CmdExportRaw(std::vector<uint16_t> &buffer, Conoscope::CmdExportRawOutput_t& output, Conoscope::CmdExportAdditionalInfo_t &additionalInfo);

    ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config, Conoscope::CmdExportProcessedOutput_t& output);
    ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, Conoscope::CmdExportProcessedOutput_t& output);

    ClassCommon::Error CmdClose();
    ClassCommon::Error CmdReset(QString &cfgPath);

    ClassCommon::Error CmdSetConfig(ConoscopeSettings_t &config);
    ClassCommon::Error CmdGetConfig(ConoscopeSettings_t &config);

    ClassCommon::Error CmdGetCmdConfig(SetupConfig_t &setupConfig,
                                       MeasureConfig_t &measureConfig,
                                       ProcessingConfig_t &processingConfig);

    ClassCommon::Error CmdSetDebugConfig(ConoscopeDebugSettings_t &conoscopeConfig);
    ClassCommon::Error CmdGetDebugConfig(ConoscopeDebugSettings_t &conoscopeConfig);

    ClassCommon::Error CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig);
    ClassCommon::Error CmdRegisterLogCallback(void (*callback)(char*));
    ClassCommon::Error CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString));
    ClassCommon::Error CmdRegisterWarningCallback(void (*callback)(QString));

    ClassCommon::Error CmdCfgFileWrite();
    ClassCommon::Error CmdCfgFileRead();
    ClassCommon::Error CmdCfgFileStatus(CfgFileStatus_t &status);

    ClassCommon::Error CmdGetCaptureSequenceConfig(CaptureSequenceConfig_t& config);
    ClassCommon::Error CmdCaptureSequence(CaptureSequenceConfig_t& config);
    ClassCommon::Error CmdCaptureSequenceCancel();
    ClassCommon::Error CmdCaptureSequenceStatus(CaptureSequenceStatus_t &status);

    ClassCommon::Error CmdMeasureAE(MeasureConfig_t& config);
    ClassCommon::Error CmdMeasureAECancel();
    ClassCommon::Error CmdMeasureAEStatus(MeasureStatus_t& status);

    ClassCommon::Error CmdConvertRaw(ConvertRaw_t& param);

public:

private:
    bool SendRequest(ConoscopeAppWorker::Request event);

    void _SetState(State eState);

    ClassCommon::Error ChangeState(State eState);
    ClassCommon::Error ChangeState(State eState, void* parameter);

    ClassCommon::Error ProcessStateMachine(Event eEvent);
    ClassCommon::Error ProcessStateMachine(Event eEvent, void* parameter);

    State mState;
    State mStatePrevious;

    ConoscopeAppWorker* mWorker;

    ConoscopeConfig* mConfig;
    Conoscope* mConoscope = NULL;

    void on_conoscopeProcess_Log(QString message);

#ifdef MEASUREAE_WA
    // this is a WA because behavior is not the same when used with external loop
    // i.e. python scripts
    bool mExternalCoreLoop;
#endif
public slots:
    void on_worker_jobDone(int value, int eError);

protected:
    void run();

};

#endif // CONOSCOPEAPP_H
