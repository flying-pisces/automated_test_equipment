#include "PipelineLib.h"

#include <QLibrary>
#include "toolReturnCode.h"

#define ErrorMessage_AlreadyInstanciated "Error: Already instanciated"
#define ErrorMessage_LoadingDll          "Error: Loading Dll"
#define ErrorMessage_ResolvingApi        "Error: Resolving Api"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define RESOLVE(a) Lib##a = (f_##a)pipelinelib.resolve(TOSTRING(a)); if(!Lib##a) {std::string message = ErrorMessage_ResolvingApi; message.append(" "); message.append(TOSTRING(a)); throw std::exception(message.c_str()); }

PipelineLib::PipelineLib(QObject *parent) : ClassCommon(parent)
{
    _Load();
}

PipelineLib::~PipelineLib()
{
}

#define LIB_EXECUTE(fct) QString::fromStdString(std::string(fct))

QString PipelineLib::GetVersion()
{
    QString message = LIB_EXECUTE(LibCmdGetVersion());
    return message;
}

ClassCommon::Error PipelineLib::CmdComputeRawData(
        int16* inputData,
        Pipeline_RawDataParam* param,
        Pipeline_ResultRawDataParam &resultParam)
{
    QString error = LIB_EXECUTE(LibCmdComputeRawData(inputData, param, resultParam));

    ToolReturnCode returnCode = ToolReturnCode(error);
    mErrorDescription = returnCode.GetOption("ErrorDescription").toString();

    return returnCode.GetError();
}

ClassCommon::Error PipelineLib::CmdComputeKLibData(
        int16* inputData,
        Pipeline_KLibDataParam &param,
        Pipeline_CalibrationParam *calibration,
        int16* klibData)
{
    QString error = LIB_EXECUTE(LibCmdComputeKLibData(inputData, param, calibration, klibData));

    ToolReturnCode returnCode = ToolReturnCode(error);
    mErrorDescription = returnCode.GetOption("ErrorDescription").toString();

    ToolReturnCode::SetErrorDescription(mErrorDescription);

    return returnCode.GetError();
}

void PipelineLib::_Load()
{
    Log("loading DLL...");

    QLibrary pipelinelib("PipelineLib");

    if(!pipelinelib.load())
    {
        throw std::exception(ErrorMessage_LoadingDll);
    }

    Log("DLL loaded");

    RESOLVE(CmdGetVersion);
    RESOLVE(CmdComputeRawData);
    RESOLVE(CmdComputeKLibData);

    Log("API resolved");
}


