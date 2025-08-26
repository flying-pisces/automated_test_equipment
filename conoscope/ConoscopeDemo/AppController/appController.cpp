#include <QVector>
#include <QMap>
#include <QMetaEnum>

#include "configuration.h"

#include "appController.h"
#include "appControllerWorker.h"

#ifdef CONOSCOPE_DLL
#include "conoscopeLib.h"
#else
#include "conoscope.h"
#endif

#define LOG_I(x)        if(RESOURCES->IsMaskEnable(LogMask_Any) == true)          Log("AppController", x)
#define LOG_C(x)        if(RESOURCES->IsMaskEnable(LogMask_State) == true)        Log("AppController", x)
#define LOG_C_ERROR(x)  if(RESOURCES->IsMaskEnable(LogMask_Error) == true)        Log("AppController", x)
#define LOG_SM(x)       if(RESOURCES->IsMaskEnable(LogMask_StateMachine) == true) Log("AppController SM", x)
#define LOG_SM_ERROR(x) if(RESOURCES->IsMaskEnable(LogMask_Error) == true)        Log("AppController SM", x)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

// configuration
#define DEBUG_PROCESSING

// this is not clean but
// there will be only one instance
// and so it is the trick to connect a static function (callback)
// to the instance
static AppController* mInstance = nullptr;

AppController::AppController(QObject *parent) : ClassThreadCommon(parent)
{
    mInstance = this;

    m_state = State::Undefined;

#ifdef CONOSCOPE_DLL
    RESOURCES->mConoscopeApi = new ConoscopeLib(this);
#else
    RESOURCES->mConoscopeApi = new Conoscope(this);
#endif

    connect(RESOURCES->mConoscopeApi, &ConoscopeLib::WriteLog,
            this, &AppController::on_log);

    connect(RESOURCES->mConoscopeApi, &ConoscopeLib::WriteStatus,
            this, &AppController::on_status);

    // create the worker
    // and all the connections
    m_worker = new AppControllerWorker();

    connect(m_worker, &AppControllerWorker::WriteLog,
            this, &AppController::WriteLog);

    connect(m_worker, &AppControllerWorker::WriteStatus,
            this, &AppController::WriteStatus);

    // request the worker to do something
    connect(this, &AppController::WorkRequest,
            m_worker, &AppControllerWorker::OnWorkRequest);

    // notification from the worker when job is done
    connect(m_worker, &AppControllerWorker::WorkDone,
            this, &AppController::on_worker_jobDone);

    // notification when the raw buffer can be displayed
    connect(m_worker, &AppControllerWorker::RawBufferReady,
            this, &AppController::RawBufferReady);

    connect(m_worker, &AppControllerWorker::RawBufferAeRoi,
            this, &AppController::RawBufferAeRoi);

    connect(m_worker, &AppControllerWorker::PleaseUpdateExposureTime,
            this, &AppController::PleaseUpdateExposureTime);

    connect(m_worker, &AppControllerWorker::WarningMessage,
            this, &AppController::WarningMessage);

    m_worker->moveToThread(this);
}

AppController::~AppController()
{
    delete RESOURCES->mConoscopeApi;

    Stop();

    terminate();
    wait();

    delete m_worker;

    // release resource
    // removed else crash
    // free(RESOURCES->mConoscopeApi);
}

void AppController::Start()
{
    if(m_state == State::Undefined)
    {
        // execute the run function in a new thread
        start();

        while(m_state == State::Undefined)
        {
            msleep(50);
        }
    }
}

ClassCommon::Error AppController::Stop()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    StopThread();

    return eError;
}

void AppController::StopThread()
{
    if(m_state != State::Undefined)
    {
        // if ever streaming is on going, stop it
        m_worker->CancelCmd();

        quit();
        wait();

        // delete the worker
        // delete m_worker;

        while(m_state != State::Undefined)
        {
            msleep(50);
        }

        // ChangeState(State::Undefined);
    }
}

void AppController::SetRawDataBuffer(FrameBuffer* pFrameBuffer)
{
    m_worker->pFrameBuffer = pFrameBuffer;
}

bool AppController::SendRequest(AppControllerWorker::Request event)
{
    // indicate that the state has changed
    emit WorkRequest((int) event);

    return true;
}

bool AppController::SendWarning(QString message)
{
    emit WarningMessage(message);

    return true;
}

ClassCommon::Error AppController::ChangeState(State eState)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    m_state =  eState;
    emit StateChange((int)eState);

    LOG_SM(QString("state change to  [%1]")
        .arg(EnumToString("State", (int)m_state)));

    switch(eState)
    {
    case State::Undefined:
        // Status("Undefined");
        break;

    case State::Idle:
        break;

    case State::Opening:
        SendRequest(AppControllerWorker::Request::CmdOpen);
        break;

    case State::Opened:
        break;

    case State::SettingUp:
        SendRequest(AppControllerWorker::Request::CmdSetup);
        break;

    case State::Ready:
        break;

    case State::Measuring:
        SendRequest(AppControllerWorker::Request::CmdMeasure);
        break;

    case State::MeasureDone:
        break;

    case State::ExportRaw:
        SendRequest(AppControllerWorker::Request::CmdExportRaw);
        break;

    case State::ExportProcessing:
        SendRequest(AppControllerWorker::Request::CmdExportProcessed);
        break;

    case State::Closing:
        SendRequest(AppControllerWorker::Request::CmdClose);
        break;

    case State::Reseting:
        SendRequest(AppControllerWorker::Request::CmdReset);
        break;

    case State::CfgFileReading:
        SendRequest(AppControllerWorker::Request::CmdCfgFileRead);
        break;

    case State::CfgFileWriting:
        SendRequest(AppControllerWorker::Request::CmdCfgFileWrite);
        break;

    case State::Error:
        // Status("Error");
        break;

    case State::Streaming:
        SendRequest(AppControllerWorker::Request::CmdStream);
        break;

    case State::StreamStoping:
        m_worker->CancelCmd();
        break;

    case State::DisplayRaw:
        SendRequest(AppControllerWorker::Request::CmdExportRawBuffer);
        break;

    case State::DisplayProcessed:
        SendRequest(AppControllerWorker::Request::CmdExportProcessedBuffer);
        break;

    case State::CapturingSequence:
        SendRequest(AppControllerWorker::Request::CmdCaptureSequence);
        break;

    case State::CaptureSequenceCanceling:
        SendRequest(AppControllerWorker::Request::CmdCaptureSequenceCancel);
        break;

    case State::MeasuringAE:
        SendRequest(AppControllerWorker::Request::CmdMeasureAE);
        break;

    case State::MeasuringAECanceling:
        SendRequest(AppControllerWorker::Request::CmdMeasureAECancel);
        break;

    case State::ConvertingRaw:
        SendRequest(AppControllerWorker::Request::CmdConvertRaw);
        break;

    default:
        // keep that line to explicitelly define all transitions
        // event if there is no actions
        eError = ClassCommon::Error::Failed;
        break;
    }

    return eError;
}

ClassCommon::Error AppController::ProcessStateMachine(Event eEvent)
{
    ClassCommon::Error eError = ClassCommon::Error::InvalidState;

    LOG_SM(QString("process event    [%1] in state [%2]")
        .arg(EnumToString("Event", (int)eEvent))
        .arg(EnumToString("State", (int)m_state)));

    static State convertRawState;

    switch(m_state)
    {
    case State::Idle:
        if(eEvent == Event::Open)
        {
            eError = ChangeState(State::Opening);
        }
        else if(eEvent == Event::Reset)
        {
            eError = ChangeState(State::Reseting);
        }
        else if(eEvent == Event::ConvertRaw)
        {
            convertRawState = m_state;
            eError = ChangeState(State::ConvertingRaw);
        }
        break;

    case State::Opening:
        if(eEvent == Event::OpenDone)
        {
            eError = ChangeState(State::Opened);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::Opened:
        if(eEvent == Event::Setup)
        {
            eError = ChangeState(State::SettingUp);
        }
        else if(eEvent == Event::Close)
        {
            eError = ChangeState(State::Closing);
        }
        else if(eEvent == Event::Reset)
        {
            eError = ChangeState(State::Reseting);
        }
        else if(eEvent == Event::CfgFileRead)
        {
            eError = ChangeState(State::CfgFileReading);
        }
        else if(eEvent == Event::CfgFileWrite)
        {
            eError = ChangeState(State::CfgFileWriting);
        }
        else if(eEvent == Event::CaptureSequence)
        {
            eError = ChangeState(State::CapturingSequence);
        }
        break;

    case State::CfgFileReading:
    case State::CfgFileWriting:
        if(eEvent == Event::CmdCfgFileDone)
        {
            eError = ChangeState(State::Opened);
        }
        break;

    case State::SettingUp:
        if(eEvent == Event::SetupDone)
        {
            eError = ChangeState(State::Ready);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::Ready:
        if(eEvent == Event::Setup)
        {
            eError = ChangeState(State::SettingUp);
        }
        else if(eEvent == Event::SetupDone)
        {
            eError = ChangeState(State::Ready);
        }
        else if(eEvent == Event::Measure)
        {
            eError = ChangeState(State::Measuring);
        }
        else if(eEvent == Event::MeasureAE)
        {
            eError = ChangeState(State::MeasuringAE);
        }
        else if(eEvent == Event::Close)
        {
            eError = ChangeState(State::Closing);
        }
        else if(eEvent == Event::Reset)
        {
            eError = ChangeState(State::Reseting);
        }
        else if(eEvent == Event::Stream)
        {
            eError = ChangeState(State::Streaming);
        }
        else if(eEvent == Event::CaptureSequence)
        {
            eError = ChangeState(State::CapturingSequence);
        }
        break;

    case State::Measuring:
        if(eEvent == Event::MeasureDone)
        {
            eError = ChangeState(State::MeasureDone);
        }
        else if(eEvent == Event::MeasureNonFatalError)
        {
            eError = ChangeState(State::Ready);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::MeasuringAE:
        if(eEvent == Event::MeasureAEDone)
        {
            eError = ChangeState(State::MeasureDone);
        }
        else if(eEvent == Event::MeasureAECancel)
        {
            eError = ChangeState(State::MeasuringAECanceling);
        }
        if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::MeasuringAECanceling:
        if(eEvent == Event::MeasureAEDone)
        {
            eError = ChangeState(State::Ready);
        }
        break;

    case State::MeasureDone:
        if(eEvent == Event::Setup)
        {
            eError = ChangeState(State::SettingUp);
        }
        else if(eEvent == Event::Measure)
        {
            eError = ChangeState(State::Measuring);
        }
        else if(eEvent == Event::MeasureAE)
        {
            eError = ChangeState(State::MeasuringAE);
        }
        else if(eEvent == Event::ExportRaw)
        {
            eError = ChangeState(State::ExportRaw);
        }
        else if(eEvent == Event::ExportProcessed)
        {
            eError = ChangeState(State::ExportProcessing);
        }
        else if(eEvent == Event::Close)
        {
            eError = ChangeState(State::Closing);
        }
        else if(eEvent == Event::Reset)
        {
            eError = ChangeState(State::Reseting);
        }
        else if(eEvent == Event::DisplayRaw)
        {
            eError = ChangeState(State::DisplayRaw);
        }
        else if(eEvent == Event::DisplayProcessed)
        {
            eError = ChangeState(State::DisplayProcessed);
        }
        else if(eEvent == Event::CaptureSequence)
        {
            eError = ChangeState(State::CapturingSequence);
        }
        break;

    case State::ExportRaw:
        if(eEvent == Event::ExportDone)
        {
            eError = ChangeState(State::MeasureDone);
        }
        break;

    case State::ExportProcessing:
        if(eEvent == Event::ExportDone)
        {
            eError = ChangeState(State::MeasureDone);
        }
        break;

    case State::Closing:
        if(eEvent == Event::CloseDone)
        {
            eError = ChangeState(State::Idle);
        }
        break;

    case State::Reseting:
        if(eEvent == Event::ResetDone)
        {
            eError = ChangeState(State::Opened);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::Error:
        if(eEvent == Event::Close)
        {
            eError = ChangeState(State::Closing);
        }
        else if(eEvent == Event::Reset)
        {
            eError = ChangeState(State::Reseting);
        }
        break;

    case State::Streaming:
        if(eEvent == Event::Stream)
        {
            eError = ChangeState(State::StreamStoping);
        }
        else if(eEvent == Event::StreamDone)
        {
            eError = ChangeState(State::Ready);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::StreamStoping:
        if(eEvent == Event::StreamDone)
        {
            eError = ChangeState(State::Ready);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::DisplayRaw:
        if(eEvent == Event::ExportDone)
        {
            eError = ChangeState(State::MeasureDone);
        }
        else if(eEvent == Event::Error)
        {
            //eError = ChangeState(State::Error);
            eError = ChangeState(State::MeasureDone);
        }
        break;

    case State::DisplayProcessed:
        if(eEvent == Event::ExportDone)
        {
            eError = ChangeState(State::MeasureDone);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::MeasureDone);
        }
        break;

    case State::CapturingSequence:
        if(eEvent == Event::CaptureSequenceCancel)
        {
            eError = ChangeState(State::CaptureSequenceCanceling);
        }
        else if(eEvent == Event::CaptureSequenceDone)
        {
            eError = ChangeState(State::Opened);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::CaptureSequenceCanceling:
        if(eEvent == Event::CaptureSequenceDone)
        {
            eError = ChangeState(State::Opened);
        }
        break;

    case State::ConvertingRaw:
        if(eEvent == Event::ConvertRawDone)
        {
            eError = ChangeState(convertRawState);
        }
        break;

    default:
        break;
    }

    if(eError == ClassCommon::Error::InvalidState)
    {
        LOG_SM(QString("ERROR   event %1 in state %2 not handled")
            .arg(EnumToString("Event", (int)eEvent))
            .arg(EnumToString("State", (int)m_state)));
    }

    return eError;
}

void AppController::run()
{
    // indicate the thread is started
    ChangeState(State::Idle);

    exec();

    ChangeState(State::Undefined);
}

void AppController::on_worker_jobDone(int value, int error)
{
    // asynchronous execution is done
    AppControllerWorker::Request eRequest = static_cast<AppControllerWorker::Request>(value);
    ClassCommon::Error eError = static_cast<ClassCommon::Error>(error);

    LOG_C(QString("on request done  [%1] %2%3")
        .arg(AppControllerWorker::EnumToString("Request", value))
        .arg((eError == ClassCommon::Error::Ok) ? "" : "ERROR : ")
        .arg(ClassCommon::ErrorToString(eError)));

    bool bNonFatalError = false;

    if(eRequest == AppControllerWorker::Request::CmdMeasure)
    {
        if(eError == ClassCommon::Error::InvalidConfiguration)
        {
            bNonFatalError = true;
            eError = ClassCommon::Error::Ok;
        }
    }

    if(eError == ClassCommon::Error::Ok)
    {
        switch(eRequest)
        {
        case AppControllerWorker::Request::CmdOpen:
            ProcessStateMachine(Event::OpenDone);
            break;

        case AppControllerWorker::Request::CmdSetup:
            ProcessStateMachine(Event::SetupDone);
            break;

        case AppControllerWorker::Request::CmdMeasure:
            if(bNonFatalError == false)
            {
                ProcessStateMachine(Event::MeasureDone);
            }
            else
            {
                ProcessStateMachine(Event::MeasureNonFatalError);
            }
            break;

        case AppControllerWorker::Request::CmdExportRaw:
            ProcessStateMachine(Event::ExportDone);
            break;

        case AppControllerWorker::Request::CmdExportProcessed:
            ProcessStateMachine(Event::ExportDone);
            break;

        case AppControllerWorker::Request::CmdClose:
            ProcessStateMachine(Event::CloseDone);
            break;

        case AppControllerWorker::Request::CmdReset:
            ProcessStateMachine(Event::ResetDone);
            break;

        case AppControllerWorker::Request::CmdCfgFileRead:
        case AppControllerWorker::Request::CmdCfgFileWrite:
            ProcessStateMachine(Event::CmdCfgFileDone);
            break;

        case AppControllerWorker::Request::CmdStream:
            ProcessStateMachine(Event::StreamDone);
            break;

        case AppControllerWorker::Request::CmdExportRawBuffer:
            ProcessStateMachine(Event::ExportDone);
            break;

        case AppControllerWorker::Request::CmdExportProcessedBuffer:
            ProcessStateMachine(Event::ExportDone);
            break;

        case AppControllerWorker::Request::CmdCaptureSequenceCancel:
            //ProcessStateMachine(Event::CaptureSequenceDone);
            break;

        case AppControllerWorker::Request::EventCaptureSequenceDone:
            ProcessStateMachine(Event::CaptureSequenceDone);
            break;

        case AppControllerWorker::Request::EventCaptureSequenceCancel:
            ProcessStateMachine(Event::CaptureSequenceDone);
            break;

        case AppControllerWorker::Request::EventCaptureSequenceError:
            ProcessStateMachine(Event::Error);
            break;

        case AppControllerWorker::Request::EventMeasureAEDone:
            ProcessStateMachine(Event::MeasureAEDone);
            break;

        case AppControllerWorker::Request::EventMeasureAECancel:
            ProcessStateMachine(Event::MeasureAECancel);
            break;

        case AppControllerWorker::Request::EventMeasureAEError:
            ProcessStateMachine(Event::Error);
            break;

        case AppControllerWorker::Request::CmdConvertRaw:
            ProcessStateMachine(Event::ConvertRawDone);
            break;

        case AppControllerWorker::Request::CmdMeasureAE:
        case AppControllerWorker::Request::CmdMeasureAECancel:
        case AppControllerWorker::Request::CmdSetConfig:
        case AppControllerWorker::Request::CmdCaptureSequence:
            break;
        }
    }
    else
    {
        ProcessStateMachine(Event::Error);
    }
}

QString AppController::GetVersion()
{
    return RESOURCES->mConoscopeApi->GetVersion();
}

void AppController::CmdOpen()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::Open);
    LOG_I(QString ("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdSetup(SetupConfig_t &config)
{
    m_worker->params.cmdSetupConfig = config;
    ClassCommon::Error eError = ProcessStateMachine(Event::Setup);
    LOG_I(QString ("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdSetupStatus(SetupStatus_t &status)
{
    ClassCommon::Error eError = RESOURCES->mConoscopeApi->CmdSetupStatus(status);

    if(eError == ClassCommon::Error::Ok)
    {
        LOG_I("CmdSetupStatus");

        LOG_I(QString("    temp status  %1").arg(RESOURCES->mTemperatureMonitoringStateMap[status.eTemperatureMonitoringState]));
        LOG_I(QString("    temp         %1").arg(status.sensorTemperature));
        LOG_I(QString("    wheel        %1").arg(RESOURCES->mWheelStateToStringMap[status.eWheelStatus]));
        LOG_I(QString("    wheel filter %1").arg(RESOURCES->mFilterToStringMap[status.eFilter]));
        LOG_I(QString("    wheel ND     %1").arg(RESOURCES->mNdToStringMap[status.eNd]));
        LOG_I(QString("    wheel iris   %1").arg(RESOURCES->mIrisIndexToStringMap[status.eIris]));
    }
    else
    {
        LOG_I(QString ("CmdSetupStatus %1").arg(ClassCommon::ErrorToString(eError)));
    }
}

void AppController::CmdMeasure(MeasureConfig_t& config)
{
    m_worker->params.cmdMeasureConfig = config;

    ClassCommon::Error eError = ProcessStateMachine(Event::Measure);
    LOG_I(QString ("CmdMeasure %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdExportRaw()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::ExportRaw);
    LOG_I(QString ("CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdExportProcessed(ProcessingConfig_t &config)
{
    m_worker->params.cmdProcessingConfig = config;

    ClassCommon::Error eError = ProcessStateMachine(Event::ExportProcessed);
    LOG_I(QString ("CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdClose()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::Close);
    LOG_I(QString ("CmdClose %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdReset()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::Reset);
    LOG_I(QString ("CmdReset %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdCfgFileWrite()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::CfgFileWrite);
    LOG_I(QString ("CmdCfgFileWrite %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdCfgFileRead()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::CfgFileRead);
    LOG_I(QString ("CmdCfgFileRead %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdCfgFileStatus()
{
    CfgFileStatus_t status;

    ClassCommon::Error eError = RESOURCES->mConoscopeApi->CmdCfgFileStatus(status);

    if(eError == ClassCommon::Error::Ok)
    {
        LOG_I("CmdCfgFileStatus");

        LOG_I(QString("   file     %1").arg(CONVERT_TO_QSTRING(status.fileName)));
        LOG_I(QString("   state    %1").arg(RESOURCES->mCfgFileStateMap[(int)status.eState]));
        LOG_I(QString("   progress %1%").arg(status.progress));
        if(status.elapsedTime < 1000)
        {
            LOG_I(QString("   elapsed  %1 ms").arg(status.elapsedTime));
        }
        else
        {
            LOG_I(QString("   elapsed  %1 sec").arg(status.elapsedTime / 1000));
        }
    }
    else
    {
        LOG_I(QString ("CmdCfgFileStatus %1").arg(ClassCommon::ErrorToString(eError)));
    }
}

void AppController::CmdSetConfig(ConoscopeSettings_t& config)
{
    m_worker->params.cmdSetConfig = config;
    ClassCommon::Error eError = RESOURCES->mConoscopeApi->CmdSetConfig(config);
    LOG_I(QString("CmdSetConfig").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdSetConfig(ConoscopeDebugSettings_t& config)
{
    // m_worker->params.cmdSetConfig = config;
    RESOURCES->mConoscopeApi->CmdSetDebugConfig(config);
}

void AppController::CmdGetConfig(ConoscopeSettings_t& config)
{
    RESOURCES->mConoscopeApi->CmdGetConfig(config);
}

void AppController::CmdGetConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig)
{
    RESOURCES->mConoscopeApi->CmdGetCmdConfig(cmdSetupConfig, cmdMeasureConfig, cmdProcessingConfig);
}

ClassCommon::Error AppController::CmdGetConfig(SetupConfig_t& cmdSetupConfig,
                                 MeasureConfig_t& cmdMeasureConfig,
                                 ProcessingConfig_t& cmdProcessingConfig,
                                 ConoscopeDebugSettings_t& conoscopeDebugSettings,
                                 CaptureSequenceConfig_t & captureSequenceConfig)
{
    ClassCommon::Error eError;

    eError = RESOURCES->mConoscopeApi->CmdGetCmdConfig(cmdSetupConfig, cmdMeasureConfig, cmdProcessingConfig);
    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdGetDebugConfig(conoscopeDebugSettings);
    }
    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdGetCaptureSequence(captureSequenceConfig);
    }

    return eError;
}

void AppController::DisplayStream(int exposureTimeUs, bool bStreamProcessedData, bool autoExposure, float autoExposurePixelMax)
{
    m_worker->params.cmdMeasureConfig.exposureTimeUs = exposureTimeUs;
    m_worker->params.cmdMeasureConfig.nbAcquisition = 1;
    m_worker->params.cmdMeasureConfig.binningFactor = 1;
    m_worker->params.cmdMeasureConfig.bTestPattern = false;

    m_worker->params.applicationConfig.autoExposure = autoExposure;
    m_worker->params.applicationConfig.autoExposurePixelMax = autoExposurePixelMax;

    m_worker->params.applicationConfig.bStreamProcessedData = bStreamProcessedData;

    ClassCommon::Error eError = ProcessStateMachine(Event::Stream);
    LOG_I(QString ("CmdStream %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::DisplayStreamStop()
{
    m_worker->CancelCmd();
}

void AppController::DisplayRaw()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::DisplayRaw);
    LOG_I(QString ("CmdExportRawBuffer %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::DisplayProcessed(ProcessingConfig_t& config)
{
    m_worker->params.cmdProcessingConfig = config;

    ClassCommon::Error eError = ProcessStateMachine(Event::DisplayProcessed);
    LOG_I(QString ("CmdExportProcessedBuffer %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CaptureSequence(CaptureSequenceConfig_t& config)
{
    m_worker->params.cmdCaptureSequence = config;

    ClassCommon::Error eError = ProcessStateMachine(Event::CaptureSequence);
    LOG_I(QString ("CaptureSequence %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CaptureSequenceCancel()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::CaptureSequenceCancel);
    LOG_I(QString ("CaptureSequenceCancel %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CaptureSequenceStatus()
{
    CaptureSequenceStatus_t status;
    ClassCommon::Error eError = RESOURCES->mConoscopeApi->CmdCaptureSequenceStatus(status);

    if(eError == ClassCommon::Error::Ok)
    {
        LOG_I(QString("CaptureSequenceStatus step %1/%2 - filter %3 - state %4 (%5)")
              .arg(status.currentSteps).arg(status.nbSteps)
              .arg(RESOURCES->mFilterToStringMap[(int)status.eFilter], -6)
              .arg(RESOURCES->mCaptureSequenceToStringMap[(int)status.state], -10).arg(status.state));
    }
    else
    {
        LOG_I(QString ("CaptureSequenceStatus %1").arg(ClassCommon::ErrorToString(eError)));
    }
}

void AppController::MeasureAE(MeasureConfig_t& config)
{
    m_worker->params.cmdMeasureConfig = config;

    ClassCommon::Error eError = ProcessStateMachine(Event::MeasureAE);
    LOG_I(QString ("MeasureAE %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::MeasureAECancel()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::MeasureAECancel);
    LOG_I(QString ("MeasureAECancel %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::MeasureAEStatus()
{
    MeasureStatus_t status;

    ClassCommon::Error eError = RESOURCES->mConoscopeApi->CmdMeasureAEStatus(status);

    if(eError == ClassCommon::Error::Ok)
    {
        LOG_I(QString ("MeasureAEStatus state %1 (%2) expo %3 nbAcq %4")
              .arg(RESOURCES->mMeasureStateToStringMap[(int)status.state])
              .arg((int)status.state)
              .arg(status.exposureTimeUs, 7)
              .arg(status.nbAcquisition));
    }
    else
    {
        LOG_I(QString ("MeasureAEStatus %1").arg(ClassCommon::ErrorToString(eError)));
    }
}

void AppController::ConvertRaw(ConvertRaw_t& param)
{
    m_worker->params.cmdConvertRaw = param;

    ClassCommon::Error eError = ProcessStateMachine(Event::ConvertRaw);
    LOG_I(QString ("ConvertRaw %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::RegisterCallback()
{
    RESOURCES->mConoscopeApi->CmdRegisterLogCallback(&AppController::_LogCallback);
    RESOURCES->mConoscopeApi->CmdRegisterEventCallback(&AppController::_EventCallback);
    RESOURCES->mConoscopeApi->CmdRegisterWarningCallback(&AppController::_WarningCallback);
}

QString AppController::GetErrorMessage()
{
    return RESOURCES->mConoscopeApi->GetErrorDescription();
}

void AppController::_LogCallback(char* message)
{
    QString mes = QString::fromStdString(std::string(message));

    if(mInstance != nullptr)
    {
        mInstance->on_log(mes);
    }
}

void AppController::_EventCallback(ConoscopeEvent_t event, QString errorDescription)
{
    static QMap<ConoscopeEvent_t, AppControllerWorker::Request> eventMap;

    eventMap[ConoscopeEvent_CaptureSequenceDone]   = AppControllerWorker::Request::EventCaptureSequenceDone;
    eventMap[ConoscopeEvent_CaptureSequenceCancel] = AppControllerWorker::Request::EventCaptureSequenceCancel;
    eventMap[ConoscopeEvent_CaptureSequenceError]  = AppControllerWorker::Request::EventCaptureSequenceError;
    eventMap[ConoscopeEvent_MeasureAEDone]         = AppControllerWorker::Request::EventMeasureAEDone;
    eventMap[ConoscopeEvent_MeasureAECancel]       = AppControllerWorker::Request::EventMeasureAECancel;
    eventMap[ConoscopeEvent_MeasureAEError]        = AppControllerWorker::Request::EventMeasureAEError;

    ConoscopeLib::SetErrorDescription(errorDescription);

    if(mInstance != nullptr)
    {
        if(eventMap.contains(event))
        {
            mInstance->SendRequest(eventMap[event]);
        }
    }
}

void AppController::_WarningCallback(QString message)
{
    if(mInstance != nullptr)
    {
        mInstance->SendWarning(message);
    }
}

void AppController::on_log(QString message)
{
    emit WriteLog(message);
}

void AppController::on_status(QString message)
{
    emit WriteStatus(message);
}

