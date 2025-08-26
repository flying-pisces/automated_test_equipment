#include "ConoscopeAppProcess.h"
#include "cameraCmvCxp.h"
#include "cameraDummy.h"

#include "imageConfiguration.h"

#include <QDir>
#include "configuration.h"

#include <QJsonObject>
#include <QJsonDocument>

#include "PipelineLib.h"

#include <QProcess>

#include "ConoscopeResource.h"

#define RAW_FILE_NAME "%1_raw"
#define PROCESSED_FILE_NAME "%1_proc"

#define FILE_NAME "%1_filt_%2_nd_%3_iris_%4%5"
#define FILE_NAME_OPTION "_src_%1"

#define CAPTURE_EXTENSION "bin"

#define INSTANCE ConoscopeAppProcess* instance = _GetInstance(); return instance

#define MAX_ATTEMPTS    2000
#define SLEEP_TIME_MS   100
#define TIME_QUANTA_MS  10

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

// #define CHECK_POSITION
#define LOG_HEADER "[conoscopeAppProcess]"
#define LogInFile(text) RESOURCE->AppendLog(QString("%1| ").arg(LOG_HEADER, -20), text)

#define CONOSCOPE ConoscopeAppProcess::mConoscope

ConoscopeAppProcess* ConoscopeAppProcess::mInstance = NULL;
Conoscope*           ConoscopeAppProcess::mConoscope = NULL;

Conoscope::CmdOpenOutput_t             ConoscopeAppProcess::cmdOpenOutput;
Conoscope::CmdExportRawOutput_t        ConoscopeAppProcess::cmdExportRawOutput;
Conoscope::CmdExportProcessedOutput_t  ConoscopeAppProcess::cmdExportProcessedOutput;
Conoscope::CmdExportAdditionalInfo_t   ConoscopeAppProcess::cmdExportAdditionalInfo;
CfgOutput                              ConoscopeAppProcess::readCfgCameraPipelineOutput;

ConoscopeAppProcess::ConoscopeAppProcess(QObject *parent) : ClassCommon(parent)
{
}

ConoscopeAppProcess::~ConoscopeAppProcess()
{
}

ConoscopeAppProcess* ConoscopeAppProcess::_GetInstance()
{
    if(mInstance == NULL)
    {
        mInstance = new ConoscopeAppProcess();
    }

    return mInstance;
}

void ConoscopeAppProcess::SetConoscope(Conoscope* conoscope)
{
    ConoscopeAppProcess::mConoscope = conoscope;
}

void ConoscopeAppProcess::Delete()
{
    if(mInstance != NULL)
    {
        delete(mInstance);
        mInstance = NULL;
    }
}

QString ConoscopeAppProcess::CmdGetPipelineVersion()
{
    INSTANCE->_CmdGetPipelineVersion();
}

ClassCommon::Error ConoscopeAppProcess::CmdOpen()
{
    INSTANCE->_CmdOpen();
}

ClassCommon::Error ConoscopeAppProcess::CmdSetup(SetupConfig_t &config)
{
    INSTANCE->_CmdSetup(config);
}

ClassCommon::Error ConoscopeAppProcess::CmdSetupStatus(SetupStatus_t& status)
{
    INSTANCE->_CmdSetupStatus(status);
}

ClassCommon::Error ConoscopeAppProcess::CmdMeasure(MeasureConfigWithCropFactor_t &config)
{
    INSTANCE->_CmdMeasure(config);
}

ClassCommon::Error ConoscopeAppProcess::CmdExportRaw()
{
    INSTANCE->_CmdExportRaw();
}

ClassCommon::Error ConoscopeAppProcess::CmdExportRaw(std::vector<uint16_t> &buffer)
{
    INSTANCE->_CmdExportRaw(buffer);
}

ClassCommon::Error ConoscopeAppProcess::CmdExportProcessed(ProcessingConfig_t& config)
{
    INSTANCE->_CmdExportProcessed(config);
}

ClassCommon::Error ConoscopeAppProcess::CmdExportProcessed(ProcessingConfig_t& config, std::vector<int16_t> &buffer, bool bSaveImage)
{
    INSTANCE->_CmdExportProcessed(config, buffer, bSaveImage);
}

ClassCommon::Error ConoscopeAppProcess::CmdClose()
{
    INSTANCE->_CmdClose();
}

ClassCommon::Error ConoscopeAppProcess::CmdReset()
{
    INSTANCE->_CmdReset();
}

ClassCommon::Error ConoscopeAppProcess::CmdCfgFileWrite()
{
    INSTANCE->_CmdCfgFileWrite();
}

ClassCommon::Error ConoscopeAppProcess::CmdCfgFileRead()
{
    INSTANCE->_CmdCfgFileRead();
}

ClassCommon::Error ConoscopeAppProcess::CmdCfgFileStatus(CfgFileStatus_t& status)
{
    INSTANCE->_CmdCfgFileStatus(status);
}

ClassCommon::Error ConoscopeAppProcess::SetConfig(CaptureSequenceConfig_t& config)
{
    INSTANCE->_SetConfig(config);
}

ClassCommon::Error ConoscopeAppProcess::SetBehaviorConfig(ConoscopeBehavior_t& config)
{
    INSTANCE->_SetConfig(config);
}

void ConoscopeAppProcess::GetSomeInfo(SomeInfo_t& info)
{
    INSTANCE->_GetSomeInfo(info);
}

ConoscopeAppProcess* ConoscopeAppProcess::GetInstance()
{
    return ConoscopeAppProcess::_GetInstance();
}

QString ConoscopeAppProcess::_CmdGetPipelineVersion()
{
    QString message;

    message = CONOSCOPE->CmdGetPipelineVersion();

    return message;
}

ClassCommon::Error ConoscopeAppProcess::_CmdOpen()
{
    ClassCommon::Error eError;

    eError = CONOSCOPE->CmdOpen(ConoscopeAppProcess::cmdOpenOutput);

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdSetup(SetupConfig_t &config)
{
    QString message = "_CmdSetup\n";
    message.append(QString("    Filter   %1\n").arg(RESOURCE->ToString(config.eFilter)));
    message.append(QString("    Nd       %1\n").arg(RESOURCE->ToString(config.eNd)));
    message.append(QString("    Iris     %1\n").arg(RESOURCE->ToString(config.eIris)));
    message.append(QString("    Temp     %1").arg(config.sensorTemperature));
    LogInFile(message);

    ClassCommon::Error eError = ClassCommon::Error::Failed;

    eError = CONOSCOPE->CmdSetup(config);

    if(eError != ClassCommon::Error::Ok)
    {
        LogInFile(QString("_CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdSetupStatus(SetupStatus_t& status)
{
    LogInFile("_CmdSetupStatus");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdSetupStatus(status);

    LogInFile(QString("_CmdSetupStatus %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdMeasure(MeasureConfigWithCropFactor_t &config)
{
    // LogInFile("_CmdMeasure");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdMeasure(config);

    if(eError != ClassCommon::Error::Ok)
    {
        LogInFile(QString("_CmdMeasure %1").arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdExportRaw()
{
    LogInFile("_CmdExportRaw");

    ClassCommon::Error eError = ClassCommon::Error::Failed;

    eError = CONOSCOPE->CmdExportRaw(ConoscopeAppProcess::cmdExportRawOutput);

    LogInFile(QString("_CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdExportRaw(std::vector<uint16_t> &buffer)
{
    LogInFile("_CmdExportRaw");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdExportRaw(buffer,
                                     ConoscopeAppProcess::cmdExportRawOutput,
                                     ConoscopeAppProcess::cmdExportAdditionalInfo);

    LogInFile(QString("_CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdExportProcessed(ProcessingConfig_t &config)
{
    LogInFile("_CmdExportProcessed");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdExportProcessed(config, ConoscopeAppProcess::cmdExportProcessedOutput);

    LogInFile(QString("_CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, bool bSaveImage)
{
    LogInFile("_CmdExportProcessed");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdExportProcessed(config, buffer, ConoscopeAppProcess::cmdExportProcessedOutput, bSaveImage);

    LogInFile(QString("_CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdClose()
{
    LogInFile("_CmdClose");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdClose();

    LogInFile(QString("_CmdClose %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdReset()
{
    LogInFile("_CmdReset");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    QString cfgPath;

    eError = CONOSCOPE->CmdReset(cfgPath);

    ConoscopeAppProcess::cmdOpenOutput.cfgPath = cfgPath;

    LogInFile(QString("_CmdReset %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdCfgFileWrite()
{
    LogInFile("_CmdCfgFileWrite");

    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = CONOSCOPE->CmdCfgFileWrite();

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdCfgFileRead()
{
    LogInFile("_CmdCfgFileRead");

    ClassCommon::Error eError = ClassCommon::Error::Ok;
    eError = CONOSCOPE->CmdCfgFileRead();

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_CmdCfgFileStatus(CfgFileStatus_t &status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = CONOSCOPE->CmdCfgFileStatus(status);

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_SetConfig(CaptureSequenceConfig_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = mConoscope->CmdSetCaptureSequenceConfig(config);

    return eError;
}

ClassCommon::Error ConoscopeAppProcess::_SetConfig(ConoscopeBehavior_t& config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = mConoscope->CmdSetBehaviorConfig(config);

    return eError;
}

void ConoscopeAppProcess::_GetSomeInfo(SomeInfo_t& info)
{
    mConoscope->GetSomeInfo(info);
}

void ConoscopeAppProcess::_Log(QString message)
{
    if(!message.isEmpty())
    {
        // emit OnLog(message);
    }
}

