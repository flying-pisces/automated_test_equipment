#include "ConoscopeApp.h"

#include <QString>

#include "ConoscopeAppProcess.h"
#include "ConoscopeResource.h"
#include "ConoscopeAppWorker.h"

#define _Log(a)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

#define LOG_HEADER "[conoscopeApp]"
// #define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | ").arg(LOG_HEADER, -20), text)

ConoscopeApp::ConoscopeApp(QObject *parent) : ClassThreadCommon(parent)
{
#ifdef MEASUREAE_WA
    mExternalCoreLoop = false;
#endif

    mConfig = new ConoscopeConfig();
    mConfig->GetConfig(ConoscopeAppWorker::mDebugSettings);
    mConfig->GetConfig(ConoscopeAppWorker::mSettings);

    mConoscope = new Conoscope(mConfig);

    mConoscope->start();

    mState = State::Undefined;

    // create the worker
    // and all the connections
    mWorker = new ConoscopeAppWorker();

    // request the worker to do something
    connect(this, &ConoscopeApp::WorkRequest,
            mWorker, &ConoscopeAppWorker::OnWorkRequest);

    // notification from the worker when job is done
    connect(mWorker, &ConoscopeAppWorker::WorkDone,
            this, &ConoscopeApp::on_worker_jobDone);

    mWorker->moveToThread(this);

    ConoscopeAppProcess::SetConoscope(mConoscope);

    ConoscopeAppProcess* ptr = ConoscopeAppProcess::GetInstance();

    connect(ptr, &ConoscopeAppProcess::OnLog,
            this, &ConoscopeApp::on_conoscopeProcess_Log);
}

void ConoscopeApp::on_conoscopeProcess_Log(QString message)
{
    _Log(message);
}

ConoscopeApp::~ConoscopeApp()
{
    ConoscopeAppProcess::Delete();

    mConoscope->Stop();

    delete mConoscope;
}

void ConoscopeApp::Start()
{
    if(mState == State::Undefined)
    {
        // execute the run function in a new thread
        start();

        while(mState == State::Undefined)
        {
            msleep(50);
        }
    }
}

ClassCommon::Error ConoscopeApp::Stop()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    StopThread();

    return eError;
}

void ConoscopeApp::StopThread()
{
    if(mState != State::Undefined)
    {
        quit();
        wait();

        // delete the worker
        // delete m_worker;

        while(mState != State::Undefined)
        {
            msleep(50);
        }

        // ChangeState(State::Undefined);
    }
}

bool ConoscopeApp::SendRequest(ConoscopeAppWorker::Request event)
{
    // indicate that the state has changed
    emit WorkRequest((int) event, nullptr);

    return true;
}

void ConoscopeApp::_SetState(State eState)
{
    mState =  eState;

    _Log(QString("  ** state change to  [%1]")
        .arg(EnumToString("State", (int)mState)));

    emit StateChange((int)eState);
}

ClassCommon::Error ConoscopeApp::ChangeState(State eState)
{
    return ChangeState(eState, nullptr);
}

ClassCommon::Error ConoscopeApp::ChangeState(State eState, void* parameter)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    SetupConfig_t* pSetupConfig = (SetupConfig_t*)parameter;

    MeasureConfigWithCropFactor_t* pMeasureConfig = (MeasureConfigWithCropFactor_t*)parameter;

    ProcessingConfig_t* pProcessingConfig = (ProcessingConfig_t*)parameter;

    mStatePrevious = mState;

    _SetState(eState);

    switch(eState)
    {
    case State::Undefined:
        // nothing specific to do
        break;

    case State::Idle:
        // nothing specific to do
        // the engine is started
        break;

    case State::CmdOpenProcessing:
        eError = ConoscopeAppProcess::CmdOpen();

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::Opened);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdSetupProcessing:
        eError = ConoscopeAppProcess::CmdSetup(*pSetupConfig);

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::Ready);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdMeasureProcessing:
        LogInFile("CmdMeasureProcessing");

        eError = ConoscopeAppProcess::CmdMeasure(*pMeasureConfig);

        LogInFile("CmdMeasureProcessing Done");

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::CaptureDone);
        }
        else if(eError == ClassCommon::Error::InvalidConfiguration)
        {
            _SetState(State::Ready);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdExportRawProcessing:
        eError = ConoscopeAppProcess::CmdExportRaw();

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::CaptureDone);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdExportProcessedProcessing:
        eError = ConoscopeAppProcess::CmdExportProcessed(*pProcessingConfig);

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::CaptureDone);
        }
        else
        {
            // do not go in error mode
            _SetState(State::CaptureDone);
        }
        break;

    case State::CmdCloseProcessing:
        eError = ConoscopeAppProcess::CmdClose();

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::Idle);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdResetProcessing:
        eError = ConoscopeAppProcess::CmdReset();

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::Opened);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdCfgFileWriteProcessing:
        eError = ConoscopeAppProcess::CmdCfgFileWrite();

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(mStatePrevious);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdCfgFileReadProcessing:
        eError = ConoscopeAppProcess::CmdCfgFileRead();

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(mStatePrevious);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdCapturingSequence:
        // this is a WA (might be a wrong status when it is checked)
        ConoscopeAppWorker::mCaptureSequenceStatus.state = CaptureSequenceStatus_t::State_t::State_NotStarted;

        SendRequest(ConoscopeAppWorker::Request::CmdCaptureSequence);
        break;

    case State::CmdMeasuringAE:
        // this is a WA (might be a wrong status when it is checked)
        ConoscopeAppWorker::mMeasureStatus.state = MeasureStatus_t::State_t::State_NotStarted;

        SendRequest(ConoscopeAppWorker::Request::CmdMeasureAE);
        break;

    case State::CaptureDone:
        // nothing to do
        break;

    case State::Opened:
        // nothing to do
        break;

    default:
        // keep that line to explicitelly define all transitions
        // event if there is no actions
        eError = ClassCommon::Error::Failed;
        break;
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::ProcessStateMachine(Event eEvent)
{
    return ProcessStateMachine(eEvent, nullptr);
}

ClassCommon::Error ConoscopeApp::ProcessStateMachine(Event eEvent, void* parameter)
{
    ClassCommon::Error eError = ClassCommon::Error::InvalidState;

    _Log(QString("  ** process event    [%1] in state [%2]")
        .arg(EnumToString("Event", (int)eEvent))
        .arg(EnumToString("State", (int)mState)));

    switch(mState)
    {
    case State::Idle:
        if(eEvent == Event::CmdOpen)
        {
            eError = ChangeState(State::CmdOpenProcessing);
        }
        else if(eEvent == Event::CmdReset)
        {
            eError = ChangeState(State::CmdResetProcessing);
        }
        else if(eEvent == Event::CmdExportProcessed)
        {
            // this state is handled only in debug mode
            ConoscopeDebugSettings_t config;
            mConoscope->GetConfig(config);

            if(config.debugMode == true)
            {
                eError = ChangeState(State::CmdExportProcessedProcessing, parameter);
            }
        }
        break;

    case State::Opened:
        if(eEvent == Event::CmdSetup)
        {
            eError = ChangeState(State::CmdSetupProcessing, parameter);
        }
        else if(eEvent == Event::CmdClose)
        {
            eError = ChangeState(State::CmdCloseProcessing);
        }
        else if(eEvent == Event::CmdReset)
        {
            eError = ChangeState(State::CmdResetProcessing);
        }
        else if(eEvent == Event::CmdCfgFileWrite)
        {
            eError = ChangeState(State::CmdCfgFileWriteProcessing);
        }
        else if(eEvent == Event::CmdCfgFileRead)
        {
            eError = ChangeState(State::CmdCfgFileReadProcessing);
        }
        else if(eEvent == Event::CmdCaptureSequence)
        {
            eError = ChangeState(State::CmdCapturingSequence);
        }
        break;

    case State::Ready:
        if(eEvent == Event::CmdSetup)
        {
            eError = ChangeState(State::CmdSetupProcessing, parameter);
        }
        else if(eEvent == Event::CmdMeasure)
        {
            eError = ChangeState(State::CmdMeasureProcessing, parameter);
        }
        else if(eEvent == Event::CmdClose)
        {
            eError = ChangeState(State::CmdCloseProcessing);
        }
        else if(eEvent == Event::CmdReset)
        {
            eError = ChangeState(State::CmdResetProcessing);
        }
        else if(eEvent == Event::CmdCfgFileWrite)
        {
            eError = ChangeState(State::CmdCfgFileWriteProcessing);
        }
        else if(eEvent == Event::CmdCfgFileRead)
        {
            eError = ChangeState(State::CmdCfgFileReadProcessing);
        }
        else if(eEvent == Event::CmdCaptureSequence)
        {
            eError = ChangeState(State::CmdCapturingSequence);
        }
        else if(eEvent == Event::CmdMeasureAE)
        {
            eError = ChangeState(State::CmdMeasuringAE);
        }
        break;

    case State::CaptureDone:
        if(eEvent == Event::CmdSetup)
        {
            eError = ChangeState(State::CmdSetupProcessing, parameter);
        }
        else if(eEvent == Event::CmdMeasure)
        {
            eError = ChangeState(State::CmdMeasureProcessing, parameter);
        }
        else if(eEvent == Event::CmdExportRaw)
        {
            eError = ChangeState(State::CmdExportRawProcessing);
        }
        else if(eEvent == Event::CmdExportProcessed)
        {
            eError = ChangeState(State::CmdExportProcessedProcessing, parameter);
        }
        else if(eEvent == Event::CmdClose)
        {
            eError = ChangeState(State::CmdCloseProcessing);
        }
        else if(eEvent == Event::CmdReset)
        {
            eError = ChangeState(State::CmdResetProcessing);
        }
        else if(eEvent == Event::CmdCaptureSequence)
        {
            eError = ChangeState(State::CmdCapturingSequence);
        }
        else if(eEvent == Event::CmdMeasureAE)
        {
            eError = ChangeState(State::CmdMeasuringAE);
        }
        break;

    case State::CmdCapturingSequence:
        if(eEvent == Event::CmdCaptureSequenceCancel)
        {
            mWorker->CaptureSequenceCancel();
            eError = ClassCommon::Error::Ok;
        }
        else if(eEvent == Event::CmdCaptureSequenceDone)
        {
            eError = ChangeState(State::Opened);
        }
        break;

    case State::Error:
        if(eEvent == Event::CmdClose)
        {
            eError = ChangeState(State::CmdCloseProcessing);
        }
        else if(eEvent == Event::CmdReset)
        {
            eError = ChangeState(State::CmdResetProcessing);
        }
        break;

    case State::CmdMeasuringAE:
        if(eEvent == Event::CmdMeasureAECancel)
        {
            eError = mWorker->MeasureAECancel();
        }
        else if(eEvent == Event::CmdMeasureAEDone)
        {
            eError = ChangeState(State::CaptureDone);
        }
        break;

    default:
        break;
    }

    if(eError == ClassCommon::Error::InvalidState)
    {
        _Log(QString("  ** ERROR   event %1 in state %2 not handled")
            .arg(EnumToString("Event", (int)eEvent))
            .arg(EnumToString("State", (int)mState)));

        LogInFile(QString("  ** ERROR   event %1 in state %2 not handled")
            .arg(EnumToString("Event", (int)eEvent))
            .arg(EnumToString("State", (int)mState)));

        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

void ConoscopeApp::run()
{
    // indicate the thread is started
    ChangeState(State::Idle);

    exec();

    ChangeState(State::Undefined);
}

void ConoscopeApp::on_worker_jobDone(int value, int eError)
{
    ConoscopeAppWorker::Request eRequest = static_cast<ConoscopeAppWorker::Request>(value);

    switch(eRequest)
    {
    case ConoscopeAppWorker::Request::CmdCaptureSequence:
        LogInFile(QString("< CmdCaptureSequence [%1]").arg(eError));

        if(eError == (int)ClassCommon::Error::Ok)
        {
            _SetState(State::Opened);
            // ProcessStateMachine(Event::CmdCaptureSequenceDone);
            RESOURCE->SendEvent(ConoscopeEvent_CaptureSequenceDone);
        }
        else
        {
            _SetState(State::Error);
            // ProcessStateMachine(Event::CmdMeasureAEDone);
            RESOURCE->SendEvent(ConoscopeEvent_CaptureSequenceError);
        }
        break;

    case ConoscopeAppWorker::Request::CmdMeasureAE:
        LogInFile(QString("< CmdMeasureAE [%1]").arg(eError));

        if(eError == (int)ClassCommon::Error::Ok)
        {
            _SetState(State::CaptureDone);
            // ProcessStateMachine(Event::CmdMeasureAEDone);
            RESOURCE->SendEvent(ConoscopeEvent_MeasureAEDone);
        }
        else
        {
            _SetState(State::Error);
            // ProcessStateMachine(Event::CmdMeasureAEDone);
            RESOURCE->SendEvent(ConoscopeEvent_MeasureAEError);
        }
        break;
    }
}

QString ConoscopeApp::CmdGetPipelineVersion()
{
    QString message;

    message = ConoscopeAppProcess::CmdGetPipelineVersion();

    return message;
}

ClassCommon::Error ConoscopeApp::CmdOpen(Conoscope::CmdOpenOutput_t &output)
{
    LogInFile("> CmdOpen");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = ProcessStateMachine(Event::CmdOpen);

    output = ConoscopeAppProcess::cmdOpenOutput;

    if(eError == ClassCommon::Error::Ok)
    {
        LogInFile(QString("< CmdOpen | cfg path       %1").arg(output.cfgPath));
        LogInFile(QString("          | camera sn      %1").arg(output.cameraSn));
        LogInFile(QString("          | camera version %1").arg(output.cameraVersion));
    }
    else
    {
        LogInFile(QString("< CmdOpen - ERROR"));
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdSetup(SetupConfig_t &config)
{
    QString message;

    message.append("> CmdSetup\n");
    message.append(QString ("    temp   %1\n").arg(config.sensorTemperature));
    message.append(QString ("    filter %1\n").arg(RESOURCE->ToString(config.eFilter)));
    message.append(QString ("    nd     %1\n").arg(RESOURCE->ToString(config.eNd)));
    message.append(QString ("    iris   %1").arg(RESOURCE->ToString(config.eIris)));

    LogInFile(message);

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if(eError == ClassCommon::Error::Ok)
    {
        eError = ProcessStateMachine(Event::CmdSetup, &config);
    }

    if(eError != ClassCommon::Error::Ok)
    {
        LogInFile(QString("< CmdSetup - %1").arg(ClassCommon::ErrorToString(eError)));
    }
    else
    {
        LogInFile(QString("< CmdSetup"));
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdSetupStatus(SetupStatus_t &status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetupStatus");

    // this API does not go into the state machine because it is only retrieving state
    if((mState == State::Opened) ||
       (mState == State::Ready) ||
       (mState == State::CaptureDone) ||
       (mState == State::CmdSetupProcessing) ||
       (mState == State::CmdMeasureProcessing) ||
       (mState == State::CmdExportRawProcessing) ||
       (mState == State::CmdExportProcessedProcessing))
    {
        eError = ConoscopeAppProcess::CmdSetupStatus(status);
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdMeasure(MeasureConfigWithCropFactor_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdMeasure");

    if(config.exposureTimeUs < 10)
    {
        _Log(QString("CmdMeasure invalid parameter: exposureTime %1").arg(config.exposureTimeUs));
        LogInFile(QString("CmdMeasure invalid parameter: exposureTime %1").arg(config.exposureTimeUs));
        eError = ClassCommon::Error::InvalidParameter;
    }

    if((config.nbAcquisition < 1) || (config.nbAcquisition > 30))
    {
        _Log(QString("CmdMeasure invalid parameter: nbAcquisition %1").arg(config.nbAcquisition));
        LogInFile(QString("CmdMeasure invalid parameter: nbAcquisition %1").arg(config.nbAcquisition));
        eError = ClassCommon::Error::InvalidParameter;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        config.bAeEnable        = true;
        config.AEMeasAreaHeight = ConoscopeAppWorker::mSettings.AEMeasAreaHeight;
        config.AEMeasAreaWidth  = ConoscopeAppWorker::mSettings.AEMeasAreaWidth;
        config.AEMeasAreaX      = ConoscopeAppWorker::mSettings.AEMeasAreaX;
        config.AEMeasAreaY      = ConoscopeAppWorker::mSettings.AEMeasAreaY;

        eError = ProcessStateMachine(Event::CmdMeasure, &config);
    }

    LogInFile(QString("< CmdMeasure - %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdExportRaw(Conoscope::CmdExportRawOutput_t &output)
{
    ClassCommon::Error eError = ClassCommon::Error::InvalidState;;

    LogInFile("> CmdExportRaw");

    eError = ProcessStateMachine(Event::CmdExportRaw);

    output = ConoscopeAppProcess::cmdExportRawOutput;

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdExportRaw(std::vector<uint16_t> &buffer, Conoscope::CmdExportRawOutput_t& output, Conoscope::CmdExportAdditionalInfo_t& additionalInfo)
{
    ClassCommon::Error eError = ClassCommon::Error::InvalidState;

    LogInFile("> CmdExportRaw buffer");

    if(mState == State::CaptureDone)
    {
        eError = ConoscopeAppProcess::CmdExportRaw(buffer);

        output         = ConoscopeAppProcess::cmdExportRawOutput;
        additionalInfo = ConoscopeAppProcess::cmdExportAdditionalInfo;
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdExportProcessed(ProcessingConfig_t& config, Conoscope::CmdExportProcessedOutput_t &output)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdExportProcessed");

    eError = ProcessStateMachine(Event::CmdExportProcessed, &config);

    output = ConoscopeAppProcess::cmdExportProcessedOutput;

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, Conoscope::CmdExportProcessedOutput_t& output)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdExportProcessed");

    if(mState == State::CaptureDone)
    {
        eError = ConoscopeAppProcess::CmdExportProcessed(config, buffer);

        output = ConoscopeAppProcess::cmdExportProcessedOutput;
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdClose()
{
    ClassCommon::Error eError;

    LogInFile("> CmdClose");

    eError = ProcessStateMachine(Event::CmdClose);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdReset(QString &cfgPath)
{
    ClassCommon::Error eError;

    LogInFile("> CmdReset");

    eError = ProcessStateMachine(Event::CmdReset);

    if(eError == ClassCommon::Error::Ok)
    {
        cfgPath = ConoscopeAppProcess::cmdOpenOutput.cfgPath;
    }

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdSetConfig(ConoscopeSettings_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetConfig");

    LogInFile(QString("    cfgPath       %1").arg(CONVERT_TO_QSTRING(config.cfgPath)));
    LogInFile(QString("    capturePath   %1").arg(CONVERT_TO_QSTRING(config.capturePath)));

    LogInFile(QString("    fileNamePrepend %1").arg(CONVERT_TO_QSTRING(config.fileNamePrepend)));
    LogInFile(QString("    fileNameAppend  %1").arg(CONVERT_TO_QSTRING(config.fileNameAppend)));
    LogInFile(QString("    exportFileNameFormat %1").arg(CONVERT_TO_QSTRING(config.exportFileNameFormat)));
    LogInFile(QString("    exportFormat         %1").arg(config.exportFormat));
    LogInFile(QString("    AEMinExpoTimeUs         %1").arg(config.AEMinExpoTimeUs));
    LogInFile(QString("    AEMaxExpoTimeUs         %1").arg(config.AEMaxExpoTimeUs));

    mConoscope->CmdSetConfig(config);

    // retrieve the configuration that has been just set
    mConfig->GetConfig(ConoscopeAppWorker::mSettings);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdGetConfig(ConoscopeSettings_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mConoscope->CmdGetConfig(config);

    QString message;

    message.append("> CmdGetConfig\n");
    message.append(QString("    cfgPath              %1\n").arg(CONVERT_TO_QSTRING(config.cfgPath)));
    message.append(QString("    capturePath          %1\n").arg(CONVERT_TO_QSTRING(config.capturePath)));
    message.append(QString("    fileNamePrepend      %1\n").arg(CONVERT_TO_QSTRING(config.fileNamePrepend)));
    message.append(QString("    fileNameAppend       %1\n").arg(CONVERT_TO_QSTRING(config.fileNameAppend)));
    message.append(QString("    exportFileNameFormat %1\n").arg(CONVERT_TO_QSTRING(config.exportFileNameFormat)));
    message.append(QString("    exportFormat         %1").arg(config.exportFormat));

    LogInFile(message);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdGetCmdConfig(
        SetupConfig_t &setupConfig,
        MeasureConfig_t &measureConfig,
        ProcessingConfig_t &processingConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdGetCmdConfig");

    mConoscope->CmdGetCmdConfig(setupConfig,
                                measureConfig,
                                processingConfig);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdSetDebugConfig(
        ConoscopeDebugSettings_t& conoscopeConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetDebugConfig");

    eError = mConoscope->CmdSetDebugConfig(conoscopeConfig);

    // update settings in worker after it has been modified
    // probably the modification should be done at this level
    mConfig->GetConfig(ConoscopeAppWorker::mDebugSettings);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdGetDebugConfig(
        ConoscopeDebugSettings_t& conoscopeConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = mConoscope->CmdGetDebugConfig(conoscopeConfig);

    QString message;

    message.append("> CmdGetDebugConfig\n");
    message.append(QString("    debugMode      %1\n").arg(conoscopeConfig.debugMode));
    message.append(QString("    emulateCamera  %1\n").arg(conoscopeConfig.emulateCamera));
    message.append(QString("    imagePath      %1").arg(CONVERT_TO_QSTRING(conoscopeConfig.dummyRawImagePath)));
    message.append(QString("    emulateWheel   %1\n").arg(conoscopeConfig.emulateWheel));

    LogInFile(message);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdSetBehaviorConfig(
        ConoscopeBehavior_t &behaviorConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetBehaviorConfig");

    eError = mConoscope->CmdSetBehaviorConfig(behaviorConfig);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdRegisterLogCallback(void (*callback)(char*))
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdRegisterLogCallback");

    RESOURCE->RegisterLogCallback(callback);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString))
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdRegisterEventCallback");

    RESOURCE->RegisterEventCallback(callback);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdRegisterWarningCallback(void (*callback)(QString))
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdRegisterEventCallback");

    RESOURCE->RegisterWarningCallback(callback);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdCfgFileWrite()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCfgFileWrite");

    eError = ProcessStateMachine(Event::CmdCfgFileWrite);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdCfgFileRead()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCfgFileRead");

    eError = ProcessStateMachine(Event::CmdCfgFileRead);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdCfgFileStatus(CfgFileStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCfgFileStatus");

    eError = ConoscopeAppProcess::CmdCfgFileStatus(status);

    return eError;
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CONFIG_STR(a) QString("    %1 %2").arg(TOSTRING(a), -40).arg(config.##a, -30)
#define STATUS_STR(a) QString("    %1 %2").arg(TOSTRING(a), -40).arg(status.##a, -30)

ClassCommon::Error ConoscopeApp::CmdCaptureSequence(CaptureSequenceConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    QString message;

    message.append("> CmdCaptureSequence\n");
    message.append(CONFIG_STR(sensorTemperature) + "\n");
    message.append(CONFIG_STR(bWaitForSensorTemperature) + "\n");
    message.append(QString("    %1 %2\n").arg("eNd").arg(RESOURCE->ToString(config.eNd)));
    message.append(QString("    %1 %2\n").arg("eIris").arg(RESOURCE->ToString(config.eIris)));

    message.append(CONFIG_STR(exposureTimeUs_FilterX) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterXz) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterYa) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterYb) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterZ) + "\n");

    message.append(CONFIG_STR(nbAcquisition) + "\n");
    message.append(CONFIG_STR(bAutoExposure) + "\n");
    message.append(CONFIG_STR(bUseExpoFile) + "\n");
    message.append(CONFIG_STR(bSaveCapture));

    LogInFile(message);

    ConoscopeAppWorker::mCaptureSequenceConfig = config;

    ConoscopeSettingsI_t option;
    mConoscope->GetConfig(option);

    eError = ProcessStateMachine(Event::CmdCaptureSequence);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdGetCaptureSequenceConfig(CaptureSequenceConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = mConoscope->CmdGetCaptureSequenceConfig(config);

    QString message;

    message.append("> CmdGetCaptureSequenceConfig\n");

    message.append(CONFIG_STR(sensorTemperature) + "\n");
    message.append(CONFIG_STR(bWaitForSensorTemperature) + "\n");
    message.append(CONFIG_STR(eNd) + "\n");
    message.append(CONFIG_STR(eIris) + "\n");

    message.append(CONFIG_STR(exposureTimeUs_FilterX) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterXz) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterYa) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterYb) + "\n");
    message.append(CONFIG_STR(exposureTimeUs_FilterZ) + "\n");

    message.append(CONFIG_STR(nbAcquisition) + "\n");
    message.append(CONFIG_STR(bAutoExposure) + "\n");
    message.append(CONFIG_STR(bUseExpoFile) + "\n");
    message.append(CONFIG_STR(bSaveCapture));

    LogInFile(message);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdCaptureSequenceCancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCaptureSequenceCancel");

    eError = ProcessStateMachine(Event::CmdCaptureSequenceCancel);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdCaptureSequenceStatus(CaptureSequenceStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    status = ConoscopeAppWorker::mCaptureSequenceStatus;

    status.nbSteps      = ConoscopeAppWorker::mCaptureSequenceStatus.nbSteps;
    status.currentSteps = ConoscopeAppWorker::mCaptureSequenceStatus.currentSteps;
    status.state        = ConoscopeAppWorker::mCaptureSequenceStatus.state;
    status.eFilter      = ConoscopeAppWorker::mCaptureSequenceStatus.eFilter;

#ifdef MEASUREAE_WA
    if(mExternalCoreLoop == false)
    {
        if(mState == State::CmdCapturingSequence)
        {
             status.state = CaptureSequenceStatus_t::State_t::State_Process;
        }
    }
    else
#endif
    {
        // this is a WA because job done slot is not called when used with python :(
        if(status.state == CaptureSequenceStatus_t::State_t::State_Done)
        {
            _SetState(State::Opened);
        }
    }

/*
    LogInFile("> CmdCaptureSequenceStatus");
    LOG_STATUS(nbSteps);
    LOG_STATUS(currentSteps);
    LOG_STATUS(eFilter);
    LOG_STATUS(state);
*/

    return eError;
}

#define EXPO_DELTA_US 100

ClassCommon::Error ConoscopeApp::CmdMeasureAE(MeasureConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    QString message;

    message.append("> CmdMeasureAE\n");

    message.append(CONFIG_STR(exposureTimeUs) + "\n");
    message.append(CONFIG_STR(nbAcquisition));

    LogInFile(message);

    ConoscopeAppWorker::mMeasureConfig = config;

    ConoscopeSettingsI_t option;
    mConoscope->GetConfig(option);

    // clamp the exposure time not to be outside of AE boundary
    if(config.exposureTimeUs >= ConoscopeAppWorker::mSettings.AEMaxExpoTimeUs)
    {
        config.exposureTimeUs = ConoscopeAppWorker::mSettings.AEMaxExpoTimeUs - EXPO_DELTA_US;
    }
    else if(config.exposureTimeUs <= ConoscopeAppWorker::mSettings.AEMinExpoTimeUs)
    {
        config.exposureTimeUs = ConoscopeAppWorker::mSettings.AEMinExpoTimeUs + EXPO_DELTA_US;
    }

    eError = ProcessStateMachine(Event::CmdMeasureAE);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdMeasureAECancel()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdMeasureAECancel");

    eError = ProcessStateMachine(Event::CmdMeasureAECancel);

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdMeasureAEStatus(MeasureStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    status = ConoscopeAppWorker::mMeasureStatus;

#ifdef MEASUREAE_WA
    if(mExternalCoreLoop == false)
    {
        if(mState == State::CmdMeasuringAE)
        {
             status.state = MeasureStatus_t::State_t::State_Process;
        }
    }
    else
#endif
    {
        // this is a WA because job done slot is not called when used with python :(
        if(status.state == MeasureStatus_t::State_t::State_Done)
        {
            _SetState(State::CaptureDone);
        }
    }

/*
    QString message

    message.append("> CmdCaptureSequenceStatus\n");
    message.append(STATUS_STR(nbSteps) + "\n");
    message.append(STATUS_STR(currentSteps) + "\n");
    message.append(STATUS_STR(eFilter) + "\n");
    message.append(STATUS_STR(state) + "\n");

    LogInFile(message);
*/

    return eError;
}

ClassCommon::Error ConoscopeApp::CmdConvertRaw(ConvertRaw_t& param)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mConoscope->CmdConvertRaw(param);

    return eError;
}

