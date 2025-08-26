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

#define LOG_C(x) Log("AppController", x)
#define LOG_SM(x) Log("AppController SM", x)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

// configuration
#define DEBUG_PROCESSING

AppController::AppController(QObject *parent) : ClassThreadCommon(parent)
{
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

bool AppController::SendRequest(AppControllerWorker::Request event)
{
    // indicate that the state has changed
    emit WorkRequest((int) event);

    return true;
}

ClassCommon::Error AppController::ChangeState(State eState)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    m_state =  eState;
    emit StateChange((int)eState);

    LOG_C(QString("state change to  [%1]")
        .arg(EnumToString("State", (int)m_state)));

    switch(eState)
    {
    case State::Undefined:
        // Status("Undefined");
        break;

    case State::Idle:
        break;

    case State::Resetting:
        SendRequest(AppControllerWorker::Request::CmdReset);
        break;

    case State::Test1:
        SendRequest(AppControllerWorker::Request::Test1);
        break;

    case State::Test2:
        SendRequest(AppControllerWorker::Request::Test2);
        break;

    case State::TestSetup:
        SendRequest(AppControllerWorker::Request::TestSetup);
        break;

    case State::ProcessRawData:
        SendRequest(AppControllerWorker::Request::ProcessRawData);
        break;

    case State::TestMeasureAE:
        SendRequest(AppControllerWorker::Request::TestMeasureAE);
        break;

    case State::TestCapture:
        SendRequest(AppControllerWorker::Request::TestCapture);
        break;

    case State::TestCaptureSequence:
        SendRequest(AppControllerWorker::Request::TestCaptureSequence);
        break;

    case State::Error:
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

ClassCommon::Error AppController::ProcessStateMachine(Event eEvent)
{
    ClassCommon::Error eError = ClassCommon::Error::Failed;

    LOG_SM(QString("process event    [%1] in state [%2]")
        .arg(EnumToString("Event", (int)eEvent))
        .arg(EnumToString("State", (int)m_state)));

    switch(m_state)
    {
    case State::Idle:
        if(eEvent == Event::CmdReset)
        {
            eError = ChangeState(State::Resetting);
        }
        else if(eEvent == Event::Test1)
        {
            eError = ChangeState(State::Test1);
        }
        else if(eEvent == Event::Test2)
        {
            eError = ChangeState(State::Test2);
        }
        else if(eEvent == Event::TestSetup)
        {
            eError = ChangeState(State::TestSetup);
        }
        else if(eEvent == Event::ProcessRawData)
        {
            eError = ChangeState(State::ProcessRawData);
        }
        else if(eEvent == Event::TestMeasureAE)
        {
            eError = ChangeState(State::TestMeasureAE);
        }
        else if(eEvent == Event::TestCapture)
        {
            eError = ChangeState(State::TestCapture);
        }
        else if(eEvent == Event::TestCaptureSequence)
        {
            eError = ChangeState(State::TestCaptureSequence);
        }
        break;

    case State::Test1:
    case State::Test2:
    case State::TestSetup:
    case State::ProcessRawData:
    case State::TestMeasureAE:
    case State::TestCapture:
    case State::TestCaptureSequence:
        if(eEvent == Event::TestDone)
        {
            eError = ChangeState(State::Idle);
        }
        else if(eEvent == Event::Error)
        {
            eError = ChangeState(State::Error);
        }
        break;

    case State::Resetting:
        if(eEvent == Event::CmdResetDone)
        {
            eError = ChangeState(State::Idle);
        }
        break;

    default:
        break;
    }

    if(eError != ClassCommon::Error::Ok)
    {
        LOG_SM(QString("ERROR   event %1 in state %2 not handled")
            .arg(EnumToString("Event", (int)eEvent))
            .arg(EnumToString("State", (int)m_state)));

        eError = ClassCommon::Error::InvalidState;
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

    if((eError == ClassCommon::Error::Ok) ||
       (eError == ClassCommon::Error::Aborted))
    {
        switch(eRequest)
        {
        case AppControllerWorker::Request::CmdReset:
            ProcessStateMachine(Event::CmdResetDone);
            break;

        case AppControllerWorker::Request::Test1:
        case AppControllerWorker::Request::Test2:
        case AppControllerWorker::Request::TestSetup:
        case AppControllerWorker::Request::ProcessRawData:
        case AppControllerWorker::Request::TestMeasureAE:
        case AppControllerWorker::Request::TestCapture:
        case AppControllerWorker::Request::TestCaptureSequence:
            ProcessStateMachine(Event::TestDone);
            break;
        }
    }
    else
    {
        //ProcessStateMachine(Event::Error);
        ProcessStateMachine(Event::TestDone);
    }
}

QString AppController::GetVersion()
{
    return RESOURCES->mConoscopeApi->GetVersion();
}

void AppController::CmdSetConfig(ConoscopeSettings_t& config)
{
    m_worker->params.cmdSetConfig = config;
    RESOURCES->mConoscopeApi->CmdSetConfig(config);
}

void AppController::CmdReset()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::CmdReset);
    LOG_C(QString ("CmdReset %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::CmdGetConfig(ConoscopeSettings_t& config)
{
    RESOURCES->mConoscopeApi->CmdGetConfig(config);
}

void AppController::CmdGetConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig)
{
    RESOURCES->mConoscopeApi->CmdGetCmdConfig(cmdSetupConfig, cmdMeasureConfig, cmdProcessingConfig);
}

void AppController::CmdGetConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig, ConoscopeDebugSettings_t& conoscopeDebugSettings)
{
    RESOURCES->mConoscopeApi->CmdGetCmdConfig(cmdSetupConfig, cmdMeasureConfig, cmdProcessingConfig);
    RESOURCES->mConoscopeApi->CmdGetDebugConfig(conoscopeDebugSettings);
}

#ifdef ENABLE_DEBUG_INTERFACE
void AppController::CmdSetDebugMode(DebugMode_t& eDebugMode)
{
    RESOURCES->mConoscopeApi->CmdSetDebugMode(eDebugMode);
}
#endif

void AppController::Test1()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::Test1);
    LOG_C(QString ("Test1 %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::Test2()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::Test2);
    LOG_C(QString ("Test2 %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::TestSetup()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::TestSetup);
    LOG_C(QString ("TestSetup %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::ProcessRawData()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::ProcessRawData);
    LOG_C(QString ("ProcessRawData %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::TestMeasureAE()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::TestMeasureAE);
    LOG_C(QString ("TestMeasureAE %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::TestCapture()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::TestCapture);
    LOG_C(QString ("TestCapture %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::TestCaptureSequence()
{
    ClassCommon::Error eError = ProcessStateMachine(Event::TestCaptureSequence);
    LOG_C(QString ("TestCaptureSequence %1").arg(ClassCommon::ErrorToString(eError)));
}

void AppController::TestAbort()
{
    m_worker->CancelCmd();
}

void AppController::on_log(QString message)
{
    emit WriteLog(message);
}

void AppController::on_status(QString message)
{
    emit WriteStatus(message);
}

