#include "conoscope.h"

#include <QString>

#include "cameraCmvCxp.h"
#include "QApplication"

#include "ConoscopeProcess.h"

#include "ConoscopeResource.h"

#define DISPLAY_VERSION

#ifdef DISPLAY_VERSION
#include "configuration.h"
#include "toolReturnCode.h"
#endif

#define _Log(a)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

#define LOG_HEADER "[conoscope]"
// #define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | ").arg(LOG_HEADER, -20), text)

#include "toolString.h"
#define RESOURCE ConoscopeResource::Instance()
#define LogInApp(message) RESOURCE->Log(ToolsString::FormatText("Conoscope", message));

Conoscope::Conoscope(ConoscopeConfig* pConfig, QObject *parent) : ClassThreadCommon(parent)
{
    // create a logger
    mLogger = new Logger("LogConoscope.txt");
    RESOURCE->SetLogger(mLogger);

    mState = State::Undefined;

    // create the config file
    mConfig = pConfig;

    mBehaviorConfig.saveParamOnCmd = true;
    mBehaviorConfig.updateCaptureDate = true;

    // retrieve the conoscope settings
    mConfig->GetConfig(ConoscopeProcess::mDebugSettings);
    mConfig->GetConfig(ConoscopeProcess::mSettings);
    mConfig->GetConfig(ConoscopeProcess::mSettingsI);

    // create the worker
    // and all the connections
    mWorker = new ConoscopeWorker();

    // request the worker to do something
    connect(this, &Conoscope::WorkRequest,
            mWorker, &ConoscopeWorker::OnWorkRequest);

    // notification from the worker when job is done
    connect(mWorker, &ConoscopeWorker::WorkDone,
            this, &Conoscope::on_worker_jobDone);

    mWorker->moveToThread(this);

    ConoscopeProcess* ptr = ConoscopeProcess::GetInstance();

    connect(ptr, &ConoscopeProcess::OnLog,
            this, &Conoscope::on_conoscopeProcess_Log);

#ifdef DISPLAY_VERSION
    QString message;

    message.append(QString("  ConoscopeLib  %1 [%2]").arg(VERSION_STR, -10).arg(RELEASE_DATE));

    QString pipelineVersion = CmdGetPipelineVersion();
    ToolReturnCode pipelineError = ToolReturnCode(pipelineVersion);
    QMap<QString, QVariant> pipelineOptions = pipelineError.GetOption();
    message.append(QString("  PipelineLib   %1 [%2]").arg(pipelineOptions["Version"].toString(), -10).arg(pipelineOptions["Date"].toString()));

    LogInFile(message);
#endif
}

void Conoscope::on_conoscopeProcess_Log(QString message)
{
    _Log(message);
}

Conoscope::~Conoscope()
{
    ConoscopeProcess::Delete();

    delete mConfig;
}

void Conoscope::Start()
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

ClassCommon::Error Conoscope::Stop()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // stop temperature monitoring
    ConoscopeProcess::Stop();

    StopThread();

    return eError;
}

void Conoscope::StopThread()
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

bool Conoscope::SendRequest(ConoscopeWorker::Request event)
{
    // indicate that the state has changed
    emit WorkRequest((int) event, nullptr);

    return true;
}

void Conoscope::_SetState(State eState)
{
    mState =  eState;

    _Log(QString("  ** state change to  [%1]")
        .arg(EnumToString("State", (int)mState)));

    emit StateChange((int)eState);
}

ClassCommon::Error Conoscope::ChangeState(State eState)
{
    return ChangeState(eState, nullptr);
}

ClassCommon::Error Conoscope::ChangeState(State eState, void* parameter)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mStatePrevious = mState;

    // warning following is true depending on the command
    // with this method the configuration is the parameter of the function
    SetupConfig_t*      pSetupConfig = (SetupConfig_t*)parameter;
    MeasureConfigWithCropFactor_t*    pMeasureConfig = (MeasureConfigWithCropFactor_t*)parameter;
    ProcessingConfig_t* pProcessingConfig = (ProcessingConfig_t*)parameter;

    ConoscopeDebugSettings_t debugConfig;
    mConfig->GetConfig(debugConfig);

    if(debugConfig.debugMode == true)
    {
        ConoscopeProcess::CmdSetupDebug(*pSetupConfig);
    }

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
        eError = ConoscopeProcess::CmdOpen();

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
        eError = ConoscopeProcess::CmdSetup(*pSetupConfig);

        if(eError == ClassCommon::Error::Ok)
        {
#ifdef MULTITHREAD_CAPTURE_SEQUENCE
            // ifever another thread has launched an export during setup execution,
            // keep export state
            if((mState != State::CmdExportRawProcessing) &&
               (mState != State::CmdExportProcessedProcessing))
            {
                _SetState(State::Ready);
            }
#else
            _SetState(State::Ready);
#endif
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdMeasureProcessing:
        LogInFile("CmdMeasureProcessing");

        eError = ConoscopeProcess::CmdMeasure(*pMeasureConfig, mBehaviorConfig.updateCaptureDate);

        LogInFile("CmdMeasureProcessing Done");

        if(eError == ClassCommon::Error::Ok)
        {
            _SetState(State::CaptureDone);
        }
        else if((eError == ClassCommon::Error::InvalidParameter) ||
                (eError == ClassCommon::Error::InvalidConfiguration))
        {
            RESOURCE->SendWarning();

            _SetState(State::Ready);
        }
        else
        {
            _SetState(State::Error);
        }
        break;

    case State::CmdExportRawProcessing:
        eError = ConoscopeProcess::CmdExportRaw();

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
        eError = ConoscopeProcess::CmdExportProcessed(*pProcessingConfig);

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
        eError = ConoscopeProcess::CmdClose();

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
        eError = ConoscopeProcess::CmdReset();

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
        // asynchronous execution
        SendRequest(ConoscopeWorker::Request::CmdCfgFileWriteProcessing);
        break;

    case State::CmdCfgFileReadProcessing:
        // asynchronous execution
        SendRequest(ConoscopeWorker::Request::CmdCfgFileReadProcessing);
        break;

    default:
        // keep that line to explicitelly define all transitions
        // event if there is no actions
        eError = ClassCommon::Error::Failed;
        break;
    }

    return eError;
}

ClassCommon::Error Conoscope::ProcessStateMachine(Event eEvent)
{
    return ProcessStateMachine(eEvent, nullptr);
}

ClassCommon::Error Conoscope::ProcessStateMachine(Event eEvent, void* parameter)
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
            mConfig->GetConfig(config);

            if(config.debugMode == true)
            {
                eError = ChangeState(State::CmdExportProcessedProcessing);
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
        break;

#ifdef MULTITHREAD_CAPTURE_SEQUENCE
    case State::CmdExportRawProcessing:
    case State::CmdExportProcessedProcessing:
        // this is a very specific behavior where it is possible to execute setup while processing data
        // note1: processing must be done in another thread and that there is no check...
        // note2: setup can not be done during Measurement
        if(eEvent == Event::CmdSetup)
        {
            // warning following is true depending on the command
            // with this method the configuration is the parameter of the function
            SetupConfig_t* pSetupConfig = (SetupConfig_t*)parameter;

            eError = ConoscopeProcess::CmdSetup(*pSetupConfig);

            if(eError != ClassCommon::Error::Ok)
            {
                _SetState(State::Error);
            }
        }
        break;
#endif

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

    default:
        break;
    }

    if(eError != ClassCommon::Error::InvalidState)
    {
        _Log(QString("  ** ERROR   event %1 in state %2 not handled")
            .arg(EnumToString("Event", (int)eEvent))
            .arg(EnumToString("State", (int)mState)));
    }

    return eError;
}

void Conoscope::run()
{
    // indicate the thread is started
    ChangeState(State::Idle);

    exec();

    ChangeState(State::Undefined);
}

//void Conoscope::on_worker_jobDone(int value, int error)
void Conoscope::on_worker_jobDone(int value, int)
{
    ConoscopeWorker::Request eRequest = static_cast<ConoscopeWorker::Request>(value);
    // ClassCommon::Error eError = static_cast<ClassCommon::Error>(error);

    switch(eRequest)
    {
    case ConoscopeWorker::Request::CmdCfgFileReadProcessing:
        // don't check error
        ChangeState(mStatePrevious);
        break;

    case ConoscopeWorker::Request::CmdCfgFileWriteProcessing:
        // don't check error
        ChangeState(mStatePrevious);
        break;
    }
}

QString Conoscope::CmdGetPipelineVersion()
{
    QString message;
    message = ConoscopeProcess::CmdGetPipelineVersion();
    return message;
}

ClassCommon::Error Conoscope::CmdOpen(CmdOpenOutput_t &output)
{
    ClassCommon::Error eError;
    eError = ProcessStateMachine(Event::CmdOpen);

    if(eError == ClassCommon::Error::Ok)
    {
        output.cfgPath       = ConoscopeProcess::mInfo.cfgPath;

        output.cameraSn      = ConoscopeProcess::mInfo.cameraSerialNumber;
        output.cameraVersion = ConoscopeProcess::mInfo.cameraVersion;
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdSetup(SetupConfig_t &config)
{
    /*
    LogInFile("> CmdSetup");

    LogInFile(QString ("  temp   %1").arg(config.sensorTemperature));
    LogInFile(QString ("  filter %1").arg(RESOURCE->ToString(config.eFilter)));
    LogInFile(QString ("  nd     %1").arg(RESOURCE->ToString(config.eNd)));
    LogInFile(QString ("  iris   %1").arg(RESOURCE->ToString(config.eIris)));
    */

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // check parameters
    if(config.eIris >= IrisIndex_Invalid)
    {
        _Log(QString("CmdSetup invalid parameter:irisIndex %1 > %2").arg(config.eIris).arg(IrisIndex_Invalid));
        eError = ClassCommon::Error::InvalidParameter;
    }

    if(config.eFilter >= Filter_Invalid)
    {
        _Log(QString("CmdSetup invalid parameter: filterIndex %1 > %2").arg(config.eFilter).arg(Filter_Invalid));
        eError = ClassCommon::Error::InvalidParameter;
    }

    if(config.eNd > Nd_Invalid)
    {
        _Log(QString("CmdSetup invalid parameter: nbIndex %1 > %2").arg(config.eNd).arg(Nd_Invalid));
        eError = ClassCommon::Error::InvalidParameter;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // store config in json file
        mConfig->SetConfig(config);

        eError = ProcessStateMachine(Event::CmdSetup, &config);
    }

    if(eError != ClassCommon::Error::Ok)
    {
        LogInFile(QString("< CmdSetup - %1").arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdSetupStatus(SetupStatus_t &status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // LogInFile("> CmdSetupStatus");

    // this API does not go into the state machine because it is only retrieving state
    if((mState == State::Opened) ||
       (mState == State::Ready) ||
       (mState == State::CaptureDone) ||
       (mState == State::CmdSetupProcessing) ||
       (mState == State::CmdMeasureProcessing) ||
       (mState == State::CmdExportRawProcessing) ||
       (mState == State::CmdExportProcessedProcessing))
    {
        eError = ConoscopeProcess::CmdSetupStatus(status);
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdMeasure(MeasureConfigWithCropFactor_t &config)
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
        // store config in json file
        mConfig->SetConfig(config);

        eError = ProcessStateMachine(Event::CmdMeasure, &config);
    }

    LogInFile(QString("< CmdMeasure - %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error Conoscope::CmdExportRaw(CmdExportRawOutput_t &output)
{
    ClassCommon::Error eError;

    LogInFile("> CmdExportRaw");

    eError = ProcessStateMachine(Event::CmdExportRaw);

    if(eError == ClassCommon::Error::Ok)
    {
        output.fileName          = ConoscopeProcess::mInfo.captureFileName;

        /* Setup */
        output.sensorTemperature = ConoscopeProcess::mInfo.sensorTemperature;
        output.eFilter           = ConoscopeProcess::mInfo.eFilter;
        output.eNd               = ConoscopeProcess::mInfo.eNd;
        output.eIris             = ConoscopeProcess::mInfo.eIris;

        output.exposureTimeUs    = ConoscopeProcess::mInfo.exposureTimeUs;
        output.nbAcquisition     = ConoscopeProcess::mInfo.nbAcquisition;
        output.height            = ConoscopeProcess::mInfo.height;
        output.width             = ConoscopeProcess::mInfo.width;

#ifdef SATURATION_FLAG_RAW
        output.saturationFlag    = ConoscopeProcess::mInfo.saturationFlag;
        output.saturationLevel   = ConoscopeProcess::mInfo.saturationLevel;
#endif
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdExportRaw(std::vector<uint16_t> &buffer, CmdExportRawOutput_t& output, CmdExportAdditionalInfo_t& additionalInfo)
{
    ClassCommon::Error eError;

    LogInFile("> CmdExportRaw");

    if(mState == State::CaptureDone)
    {
        eError = ConoscopeProcess::CmdExportRaw(buffer);

        /* Setup */
        output.sensorTemperature = ConoscopeProcess::mInfo.sensorTemperature;
        output.eFilter           = ConoscopeProcess::mInfo.eFilter;
        output.eNd               = ConoscopeProcess::mInfo.eNd;
        output.eIris             = ConoscopeProcess::mInfo.eIris;

        output.exposureTimeUs    = ConoscopeProcess::mInfo.exposureTimeUs;
        output.nbAcquisition     = ConoscopeProcess::mInfo.nbAcquisition;
        output.height            = ConoscopeProcess::mInfo.height;
        output.width             = ConoscopeProcess::mInfo.width;

        output.min               = ConoscopeProcess::mInfo.min;
        output.max               = ConoscopeProcess::mInfo.max;

        /* additional information about measurement (AE) */
        additionalInfo.bAeEnable        = ConoscopeProcess::mAdditionalInfo.bAeEnable;
        additionalInfo.AEMeasAreaHeight = ConoscopeProcess::mAdditionalInfo.AEMeasAreaHeight;
        additionalInfo.AEMeasAreaWidth  = ConoscopeProcess::mAdditionalInfo.AEMeasAreaWidth;
        additionalInfo.AEMeasAreaX      = ConoscopeProcess::mAdditionalInfo.AEMeasAreaX;
        additionalInfo.AEMeasAreaY      = ConoscopeProcess::mAdditionalInfo.AEMeasAreaY;
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdExportProcessed(ProcessingConfig_t& config, CmdExportProcessedOutput_t &output)
{
    LogInFile("> CmdExportProcessed");

    // store config in json file
    mConfig->SetConfig(config);

    ClassCommon::Error eError;
    eError = ProcessStateMachine(Event::CmdExportProcessed, &config);

    // Setup
    output.sensorTemperature = ConoscopeProcess::mInfo.sensorTemperature;
    output.eFilter           = ConoscopeProcess::mInfo.eFilter;
    output.eNd               = ConoscopeProcess::mInfo.eNd;
    output.eIris             = ConoscopeProcess::mInfo.eIris;

    // camera cfg file
    output.cameraCfgFileName         = ConoscopeProcess::mInfo.cameraCfgFileName;
    output.opticalColumnCfgFileName  = ConoscopeProcess::mInfo.opticalColumnCfgFileName;
    output.flatFieldFileName         = ConoscopeProcess::mInfo.flatFieldFileName;
    output.colorCoefCompX = ConoscopeProcess::mInfo.colorCoefCompX;
    output.colorCoefCompY = ConoscopeProcess::mInfo.colorCoefCompY;
    output.colorCoefCompZ = ConoscopeProcess::mInfo.colorCoefCompZ;

    if(eError == ClassCommon::Error::Ok)
    {
        output.fileName = ConoscopeProcess::mInfo.captureFileName;
    }
    else
    {
        output.fileName = "NA: Failed";
    }

    output.exposureTimeUs = ConoscopeProcess::mInfo.exposureTimeUs;
    output.nbAcquisition  = ConoscopeProcess::mInfo.nbAcquisition;
    output.height         = ConoscopeProcess::mInfo.height;
    output.width          = ConoscopeProcess::mInfo.width;

    output.conversionFactorCompX = ConoscopeProcess::mInfo.conversionFactorCompX;
    output.conversionFactorCompY = ConoscopeProcess::mInfo.conversionFactorCompY;
    output.conversionFactorCompZ = ConoscopeProcess::mInfo.conversionFactorCompZ;

    output.saturationFlag  = ConoscopeProcess::mInfo.saturationFlag;
    output.saturationLevel = ConoscopeProcess::mInfo.saturationLevel;

    return eError;
}

ClassCommon::Error Conoscope::CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, CmdExportProcessedOutput_t& output, bool bSaveImage)
{
    LogInFile("> CmdExportProcessed");

    // store config in json file
    mConfig->SetConfig(config);

    ClassCommon::Error eError;

#ifndef MULTITHREAD_CAPTURE_SEQUENCE
    if(mState == State::CaptureDone)
#else
    if((mState == State::CaptureDone) ||
       (mState == State::CmdSetupProcessing))
#endif
    {
        // change the state here
        // it is don't respect the state machine mechanism
        _SetState(State::CmdExportProcessedProcessing);

        eError = ConoscopeProcess::CmdExportProcessed(config, buffer, bSaveImage);

        // Setup
        output.sensorTemperature = ConoscopeProcess::mInfo.sensorTemperature;
        output.eFilter           = ConoscopeProcess::mInfo.eFilter;
        output.eNd               = ConoscopeProcess::mInfo.eNd;
        output.eIris             = ConoscopeProcess::mInfo.eIris;

        // camera cfg file
        output.cameraCfgFileName         = ConoscopeProcess::mInfo.cameraCfgFileName;
        output.opticalColumnCfgFileName  = ConoscopeProcess::mInfo.opticalColumnCfgFileName;
        output.flatFieldFileName         = ConoscopeProcess::mInfo.flatFieldFileName;
        output.colorCoefCompX            = ConoscopeProcess::mInfo.colorCoefCompX;
        output.colorCoefCompY            = ConoscopeProcess::mInfo.colorCoefCompY;
        output.colorCoefCompZ            = ConoscopeProcess::mInfo.colorCoefCompZ;

        if(eError == ClassCommon::Error::Ok)
        {
            output.fileName = ConoscopeProcess::mInfo.captureFileName;
        }
        else
        {
            output.fileName = "NA: Failed";
        }

        output.exposureTimeUs = ConoscopeProcess::mInfo.exposureTimeUs;
        output.nbAcquisition  = ConoscopeProcess::mInfo.nbAcquisition;
        output.height         = ConoscopeProcess::mInfo.height;
        output.width          = ConoscopeProcess::mInfo.width;

        output.conversionFactorCompX = ConoscopeProcess::mInfo.conversionFactorCompX;
        output.conversionFactorCompY = ConoscopeProcess::mInfo.conversionFactorCompY;
        output.conversionFactorCompZ = ConoscopeProcess::mInfo.conversionFactorCompZ;

        output.min = ConoscopeProcess::mInfo.min;
        output.max  = ConoscopeProcess::mInfo.max;

        output.saturationFlag  = ConoscopeProcess::mInfo.saturationFlag;
        output.saturationLevel = ConoscopeProcess::mInfo.saturationLevel;

        _SetState(State::CaptureDone);
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdClose()
{
    ClassCommon::Error eError;

    LogInFile("> CmdClose");

    eError = ProcessStateMachine(Event::CmdClose);

    return eError;
}

ClassCommon::Error Conoscope::CmdReset(QString &cfgPath)
{
    ClassCommon::Error eError;

    LogInFile("> CmdReset");

    eError = ProcessStateMachine(Event::CmdReset);

    if(eError == ClassCommon::Error::Ok)
    {
        cfgPath = ConoscopeProcess::mInfo.cfgPath;
    }

    return eError;
}

ClassCommon::Error Conoscope::CmdSetConfig(ConoscopeSettings_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetConfig");

    LogInFile(QString("    cfgPath       %1").arg(CONVERT_TO_QSTRING(config.cfgPath)));
    LogInFile(QString("    capturePath   %1").arg(CONVERT_TO_QSTRING(config.capturePath)));

    mConfig->SetConfig(config);
    mConfig->GetConfig(ConoscopeProcess::mSettings);

    return eError;
}

ClassCommon::Error Conoscope::CmdGetConfig(ConoscopeSettings_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mConfig->GetConfig(config);

/*
    QString message;

    message.append("> CmdGetConfig\n");
    message.append(QString("    cfgPath       %1\n").arg(CONVERT_TO_QSTRING(config.cfgPath)));
    message.append(QString("    capturePath   %2\n").arg(CONVERT_TO_QSTRING(config.capturePath)));
    message.append(QString("    autoExposure  %1 (max = %2)").arg(config.autoExposure).arg(config.autoExposurePixelMax));

    LogInFile(message);
*/

    return eError;
}

ClassCommon::Error Conoscope::CmdGetCmdConfig(
        SetupConfig_t &setupConfig,
        MeasureConfig_t &measureConfig,
        ProcessingConfig_t &processingConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // LogInFile("> CmdGetCmdConfig");

    mConfig->GetConfig(setupConfig);
    mConfig->GetConfig(measureConfig);
    mConfig->GetConfig(processingConfig);

    return eError;
}

ClassCommon::Error Conoscope::CmdSetDebugConfig(
        ConoscopeDebugSettings_t& conoscopeConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetDebugConfig");

    LogInFile(QString("    debugMode      %1").arg(conoscopeConfig.debugMode));
    LogInFile(QString("    emulateCamera  %1").arg(conoscopeConfig.emulateCamera));
    LogInFile(QString("    imagePath      %1").arg(CONVERT_TO_QSTRING(conoscopeConfig.dummyRawImagePath)));
    LogInFile(QString("    emulateWheel   %1").arg(conoscopeConfig.emulateWheel));

    mConfig->SetConfig(conoscopeConfig);

    // update conoscope settings
    mConfig->GetConfig(ConoscopeProcess::mDebugSettings);

    return eError;
}

ClassCommon::Error Conoscope::CmdGetDebugConfig(
        ConoscopeDebugSettings_t& conoscopeConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    mConfig->GetConfig(conoscopeConfig);

/*
    QString message;

    message.append("> CmdGetDebugConfig\n");
    message.append(QString("    debugMode      %1\n").arg(conoscopeConfig.debugMode));
    message.append(QString("    emulateCamera  %1\n").arg(conoscopeConfig.emulateCamera));
    message.append(QString("    imagePath      %1").arg(CONVERT_TO_QSTRING(conoscopeConfig.dummyRawImagePath)));

    LogInFile(message);
*/

    return eError;
}

ClassCommon::Error Conoscope::CmdSetBehaviorConfig(
        ConoscopeBehavior_t &behaviorConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetBehaviorConfig");

    LogInFile(QString("    saveParamOnCmd    %1").arg(behaviorConfig.saveParamOnCmd == true ? "True" : "False"));
    LogInFile(QString("    updateCaptureDate %1").arg(behaviorConfig.updateCaptureDate == true ? "True" : "False"));

    mBehaviorConfig.saveParamOnCmd = (bool) behaviorConfig.saveParamOnCmd;
    mBehaviorConfig.updateCaptureDate = (bool) behaviorConfig.updateCaptureDate;

    // if(mBehaviorConfig.saveParamOnCmd == true)
    mConfig->bSaveConfig = behaviorConfig.saveParamOnCmd;

    return eError;
}

ClassCommon::Error Conoscope::CmdGetCaptureSequenceConfig(CaptureSequenceConfig_t& captureSequenceConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // LogInFile("> CmdGetCaptureSequenceConfig");

    mConfig->GetConfig(captureSequenceConfig);

    return eError;
}

ClassCommon::Error Conoscope::CmdSetCaptureSequenceConfig(CaptureSequenceConfig_t& captureSequenceConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdSetCaptureSequenceConfig");

    mConfig->SetConfig(captureSequenceConfig);

    return eError;
}

ClassCommon::Error Conoscope::CmdCfgFileWrite()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCfgFileWrite");

    eError = ProcessStateMachine(Event::CmdCfgFileWrite);

    return eError;
}

ClassCommon::Error Conoscope::CmdCfgFileRead()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCfgFileRead");

    eError = ProcessStateMachine(Event::CmdCfgFileRead);

    return eError;
}

ClassCommon::Error Conoscope::CmdCfgFileStatus(CfgFileStatus_t& status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("> CmdCfgFileStatus");

    eError = ConoscopeProcess::CmdCfgFileStatus(status);

    return eError;
}

ClassCommon::Error Conoscope::CmdConvertRaw(ConvertRaw_t &param)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = ConoscopeProcess::CmdConvertRaw(param);

    return eError;
}

void Conoscope::GetSomeInfo(SomeInfo_t& info)
{
    ConoscopeProcess::GetSomeInfo(info);
}
