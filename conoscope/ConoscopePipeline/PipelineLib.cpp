#include "PipelineLib.h"

#include <QString>

#include "configuration.h"

#include "Compute.h"

#include "toolErrorCode.h"

#define RETURN  return _GetReturn
// #define RETURN_ERROR(err)  return _GetReturn(ToolErrorCode(err).GetJsonCode())

static char * cstr = new char [0];

static const char* _GetReturn(QString message)
{
    std::string str = message.toStdString();

    delete[] cstr;
    cstr = new char [str.length()+1];

    strcpy_s (cstr, str.length()+1, str.c_str());

    return cstr;
}

const char* CmdGetVersion()
{
    ToolErrorCode eError = ToolErrorCode(ClassCommon::Error::Ok);

    eError.SetOption("Date", RELEASE_DATE);
    eError.SetOption("Version", VERSION_STR);
    eError.SetOption("Name", APPLICATION_NAME);

    RETURN(eError.GetJsonCode());
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define CASE(x) case x: message = TOSTRING(x); break
#define DEFAULT() default: message = ""; break

QString GetErrorMessage(Error_t error)
{
    QString message = "";

    switch(error)
    {
        CASE(Error_Ok);
        CASE(Error_FlatFieldNull);
        CASE(Error_FlatFieldDataNull);
        CASE(Error_InputDataNull);
        CASE(Error_InputDataNone);
        CASE(Error_CalibrationNull);
        CASE(Error_CaptureAreaNull);
        CASE(Error_OpticalAxisNull);
        CASE(Error_LinearizationCoefNull);
        CASE(Error_PrecomputedData);
        CASE(Error_SensorTemperatureDependanceNull);
        CASE(Error_ConversionFactorNull);
        CASE(Error_SensorTemperatureNull);
        CASE(Error_DataSensorTemperatureNull);
        CASE(Error_OpticalAxisOutOfActiveArea);
        DEFAULT();
    }

    return message;
}

const char* CmdComputeRawData(
        int16* inputData,
        Pipeline_RawDataParam* param,
        Pipeline_ResultRawDataParam &resultParam)
{
    Error_t res;
    res = Compute::ComputeRawData(inputData, param, resultParam);

    ClassCommon::Error eError = ClassCommon::Error::Ok;
    if(res != Error_Ok)
    {
        eError = ClassCommon::Error::Failed;
    }

    ToolErrorCode jsonError = ToolErrorCode(eError);

    jsonError.SetOption(RETURN_ITEM_OPTION_ERROR_DESC, GetErrorMessage(res));

    RETURN(jsonError.GetJsonCode());
}

const char* CmdComputeKLibData(
        int16* inputData,
        Pipeline_KLibDataParam &param,
        Pipeline_CalibrationParam *calibration,
        int16* klibData)
{
    Error_t res;
    res = Compute::ComputeKLibData(inputData, param, calibration, klibData);

    ClassCommon::Error eError = ClassCommon::Error::Ok;
    if(res != Error_Ok)
    {
        eError = ClassCommon::Error::Failed;
    }

    ToolErrorCode jsonError = ToolErrorCode(eError);

    jsonError.SetOption(RETURN_ITEM_OPTION_ERROR_DESC, GetErrorMessage(res));

    RETURN(jsonError.GetJsonCode());
}
