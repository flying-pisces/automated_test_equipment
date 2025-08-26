#include "TempMonitoring.h"

#include <QString>

#include "cameraCmvCxp.h"
#include "QApplication"

#include "ConoscopeProcess.h"
#include "ConoscopeResource.h"

#define MAX_ATTEMPTS    2000
#define SLEEP_TIME_MS   100
#define TIME_QUANTA_MS  10

#define LOG_HEADER "[temp]"
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))

QString TempMonitoring::_message = "";

TempMonitoring::TempMonitoring(Camera *camera, QObject *parent) : ClassThreadCommon(parent)
{
    mState = State::Undefined;

    mStatus = TemperatureMonitoringState_NotStarted;

    // create the worker
    // and all the connections
    mWorker = new TempMonitoringWorker(camera);

    // request the worker to do something
    connect(this, &TempMonitoring::WorkRequest,
            mWorker, &TempMonitoringWorker::OnWorkRequest);

    // notification from the worker when job is done
    connect(mWorker, &TempMonitoringWorker::WorkDone,
            this, &TempMonitoring::on_worker_jobDone);

    mWorker->moveToThread(this);

/*
    ConoscopeProcess* ptr = ConoscopeProcess::GetInstance();

    connect(ptr, &ConoscopeProcess::OnLog,
            this, &Conoscope::on_conoscopeProcess_Log);
*/
}

void TempMonitoring::on_conoscopeProcess_Log(QString message)
{
    _Log(message);
}

TempMonitoring::~TempMonitoring()
{
    Stop();
}

void TempMonitoring::Start()
{
    LogInFile("Start");

    if(mState == State::Undefined)
    {
        // execute the run function in a new thread
        start();

        while(mState == State::Undefined)
        {
            msleep(50);
        }

        LogInFile(" -> Started");
    }
}

ClassCommon::Error TempMonitoring::Stop()
{
    LogInFile("Stop");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    StopThread();

    return eError;
}

void TempMonitoring::StopThread()
{
    LogInFile("StopThread");

    if(mState != State::Undefined)
    {
        mWorker->CancelCmd();

        // wait for the end of the task
        // for any reason, signal does not come
        while(mWorker->IsRunning() == true)
        {
            msleep(50);
        }

        quit();
        wait();

        // delete the worker
        delete mWorker;
        mWorker = nullptr;

        while(mState != State::Undefined)
        {
            msleep(50);
        }

        // ChangeState(State::Undefined);
    }
}

bool TempMonitoring::SendRequest(TempMonitoringWorker::Request event, void *parameter)
{
    // indicate that the state has changed
    emit WorkRequest((int) event, parameter);

    return true;
}

void TempMonitoring::_SetState(State eState)
{
    mState =  eState;

    _Log(QString("  ** state change to  [%1]")
        .arg(EnumToString("State", (int)mState)));

    emit StateChange((int)eState);
}

ClassCommon::Error TempMonitoring::ChangeState(State eState)
{
    return ChangeState(eState, nullptr);
}

ClassCommon::Error TempMonitoring::ChangeState(State eState, void *parameter)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // State eStatePrevious = mState;

    _SetState(eState);

    switch(eState)
    {
    case State::CmdSetTemperatureProcessing:
        LogInFile(QString("state = processing"));
        mStatus = TemperatureMonitoringState_Processing;
        SendRequest(TempMonitoringWorker::Request::CmdSetTemperature, parameter);
        break;

    case State::TemperatureSet:
        LogInFile(QString("state = locked"));
        mStatus = TemperatureMonitoringState_Locked;
        break;

    case State::Error:
        LogInFile(QString("state = error"));
        mStatus = TemperatureMonitoringState_Error;
        break;

    case State::Aborted:
        LogInFile(QString("state = aborted"));
        mStatus = TemperatureMonitoringState_Aborted;
        _SetState(State::Idle);
        break;

    default:
        break;
    }

    return eError;
}

ClassCommon::Error TempMonitoring::ProcessStateMachine(Event eEvent)
{
    return ProcessStateMachine(eEvent, nullptr);
}

ClassCommon::Error TempMonitoring::ProcessStateMachine(Event eEvent, void* parameter)
{
    ClassCommon::Error eError = ClassCommon::Error::InvalidState;

/*
    _Log(QString("  ** process event    [%1] in state [%2]")
        .arg(EnumToString("Event", (int)eEvent))
        .arg(EnumToString("State", (int)mState)));
*/

    switch(mState)
    {
    case State::Idle:
    case State::TemperatureSet:
    case State::Error:
        if(eEvent == Event::CmdSetTemperature)
        {
            eError = ChangeState(State::CmdSetTemperatureProcessing, parameter);
        }
        break;

    case State::CmdSetTemperatureProcessing:
        if(eEvent == Event::CmdSetTemperatureDone)
        {
            eError = ChangeState(State::TemperatureSet);
        }
        else if(eEvent == Event::CmdSetTemperatureError)
        {
            eError = ChangeState(State::Error);
        }
        else if(eEvent == Event::CmdSetTemperatureAborted)
        {
            eError = ChangeState(State::Aborted);
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
    }

    return eError;
}

void TempMonitoring::run()
{
    // indicate the thread is started
    ChangeState(State::Idle, nullptr);

    exec();

    ChangeState(State::Undefined, nullptr);
}

void TempMonitoring::on_worker_jobDone(int value, int error)
{
    TempMonitoringWorker::Request eRequest = static_cast<TempMonitoringWorker::Request>(value);
    ClassCommon::Error eError = static_cast<ClassCommon::Error>(error);

    if(eRequest == TempMonitoringWorker::Request::CmdSetTemperature)
    {
        if(eError == ClassCommon::Error::Ok)
        {
            ProcessStateMachine(Event::CmdSetTemperatureDone);
        }
        else if(eError == ClassCommon::Error::Aborted)
        {
            ProcessStateMachine(Event::CmdSetTemperatureAborted);
        }
        else
        {
            ProcessStateMachine(Event::CmdSetTemperatureError);
        }
    }
}

ClassCommon::Error TempMonitoring::CmdSetTemperature(float temperature, int timeoutMs)
{
    LogInFile("CmdSetTemperature");

    ClassCommon::Error eError;

    mCmdSetTemperatureParam.temperature = temperature;
    mCmdSetTemperatureParam.timeoutMs = timeoutMs;

    eError = ProcessStateMachine(Event::CmdSetTemperature, &mCmdSetTemperatureParam);

    return eError;
}

ClassCommon::Error TempMonitoring::CmdCancel()
{
    LogInFile("CmdCancel");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mWorker->CancelCmd();

    return eError;
}

ClassCommon::Error TempMonitoring::CmdReset()
{
    LogInFile("CmdReset");

    ClassCommon::Error eError;

    if(mState == State::CmdSetTemperatureProcessing)
    {
        // if monotoring is in progress,
        // it must be canceled before close
        eError = ClassCommon::Error::InvalidState;
    }
    else
    {
        mState = State::Idle;
        mStatus = TemperatureMonitoringState_NotStarted;
    }

    return eError;
}

void TempMonitoring::GetTemperature(float& temp)
{
    // LogInFile("GetTemperature");

    temp = mWorker->GetTemperature();
}

void TempMonitoring::GetTemperature(
        TemperatureMonitoringState_t& eStatus,
        float& temp)
{
    // LogInFile("GetTemperature");

    temp = mWorker->GetTemperature();
    eStatus = mStatus;
}

float TempMonitoring::GetTemperatureTarget()
{
    // LogInFile("GetTemperatureTarget");

    return mCmdSetTemperatureParam.temperature;
}

void TempMonitoring::GetStatus(
        TemperatureMonitoringState_t& eStatus)
{
    // LogInFile("GetStatus");

    eStatus = mStatus;
}

