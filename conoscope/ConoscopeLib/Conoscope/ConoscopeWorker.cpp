#include "ConoscopeWorker.h"
#include "ConoscopeProcess.h"
#include "ConoscopeResource.h"

#define LOG_CW(x) Log("              Worker", x)

#define LOG_HEADER "[conoscopeWorker]"
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))

ConoscopeWorker::ConoscopeWorker(QObject *parent) : ClassCommon(parent)
{
}

ConoscopeWorker::~ConoscopeWorker()
{
}

void ConoscopeWorker::OnWorkRequest(int value)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    Request eRequest = static_cast<Request>(value);

    switch(eRequest)
    {
    case Request::CmdCfgFileReadProcessing:
        eError = _CmdCfgFileRead();
        break;

    case Request::CmdCfgFileWriteProcessing:
        eError = _CmdCfgFileWrite();
        break;

    default:
        break;
    }

    LOG_CW(QString("done     request [%1] %2")
        .arg(EnumToString("Request", (int)eRequest))
        .arg((eError == ClassCommon::Error::Ok) ? "Ok" : "Failed"));

    emit WorkDone(value, (int)eError);
}

ClassCommon::Error ConoscopeWorker::_CmdCfgFileRead()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("CmdCfgFileRead");

    eError = ConoscopeProcess::CmdCfgFileRead();

    return eError;
}

ClassCommon::Error ConoscopeWorker::_CmdCfgFileWrite()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("CmdCfgFileWrite");

    eError = ConoscopeProcess::CmdCfgFileWrite();

    return eError;
}

