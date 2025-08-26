#include "ConoscopeResource.h"

#include "toolReturnCode.h"

ConoscopeResource* ConoscopeResource::mInstance;

ConoscopeResource::ConoscopeResource()
{
    mFilterToStringMap[(int)Filter_BK7] = "BK7";
    mFilterToStringMap[(int)Filter_Mirror] = "Mirror";
    mFilterToStringMap[(int)Filter_X]  = "X";
    mFilterToStringMap[(int)Filter_Xz] = "Xz";
    mFilterToStringMap[(int)Filter_Ya] = "Ya";
    mFilterToStringMap[(int)Filter_Yb] = "Yb";
    mFilterToStringMap[(int)Filter_Z]  = "Z";
    mFilterToStringMap[(int)Filter_IrCut] = "IRCut";
    mFilterToStringMap[(int)Filter_Invalid] = "Invalid";

    mNdToStringMap[(int)Nd_0] = "0";
    mNdToStringMap[(int)Nd_1] = "1";
    mNdToStringMap[(int)Nd_2] = "2";
    mNdToStringMap[(int)Nd_3] = "3";
    mNdToStringMap[(int)Nd_4] = "4";
    mNdToStringMap[(int)Nd_Invalid] = "Invalid";

    mIrisIndexToStringMap[(int)IrisIndex_2mm] = "2";
    mIrisIndexToStringMap[(int)IrisIndex_3mm] = "3";
    mIrisIndexToStringMap[(int)IrisIndex_4mm] = "4";
    mIrisIndexToStringMap[(int)IrisIndex_5mm] = "5";
    mIrisIndexToStringMap[(int)IrisIndex_Invalid] = "Invalid";

    mWheelStateIndexToStringMap[(int)WheelState_Idle]= "Idle";
    mWheelStateIndexToStringMap[(int)WheelState_Success]= "Success";
    mWheelStateIndexToStringMap[(int)WheelState_Operating]= "Operating";
    mWheelStateIndexToStringMap[(int)WheelState_Error]= "Error";

    mTemperatureMonitoringToStringMap[(int)TemperatureMonitoringState_NotStarted]= "NotStarted";
    mTemperatureMonitoringToStringMap[(int)TemperatureMonitoringState_Processing]= "Processing";
    mTemperatureMonitoringToStringMap[(int)TemperatureMonitoringState_Locked]= "Locked";
    mTemperatureMonitoringToStringMap[(int)TemperatureMonitoringState_Aborted]= "Aborted";
    mTemperatureMonitoringToStringMap[(int)TemperatureMonitoringState_Error]= "Error";

    for(auto const& pair: mFilterToStringMap)
    {
        mFilterToStringMapRevert[pair.second] = pair.first;
    }

    for(auto const& pair: mNdToStringMap)
    {
        mNdToStringMapRevert[pair.second] = pair.first;
    }

    for(auto const& pair: mIrisIndexToStringMap)
    {
        mIrisIndexToStringMapRevert[pair.second] = pair.first;
    }

    mLogger = nullptr;
    mLogCallback = nullptr;
    mEventCallback = nullptr;
    mWarningCallback = nullptr;
}

ConoscopeResource* ConoscopeResource::GetInstance()
{
    if(mInstance == nullptr)
    {
        mInstance = new ConoscopeResource();
    }

    return mInstance;
}

ConoscopeResource* ConoscopeResource::Instance()
{
    return GetInstance();
}

QString ConoscopeResource::_ToString(std::map<int, QString>&map, int value, bool bReplaceDot)
{
    QString output = "ERROR";

    if(map.count(value) != 0)
    {
        output = map[value];

        if(bReplaceDot == true)
        {
            output = output.replace(".", "");
        }
    }

    return output;
}

int ConoscopeResource::_ToInt(std::map<QString, int>&map, QString value)
{
    if(map.count(value) != 0)
    {
        return map[value];
    }
    else
    {
        return -1;
    }
}

QString ConoscopeResource::ToString(Filter_t value)
{
    return _ToString(mFilterToStringMap, value);
}

QString ConoscopeResource::ToString(Nd_t value, bool bReplaceDot)
{
    return _ToString(mNdToStringMap, value, bReplaceDot);
}

QString ConoscopeResource::ToString(IrisIndex_t value, bool bReplaceDot)
{
    return _ToString(mIrisIndexToStringMap, value, bReplaceDot);
}

QString ConoscopeResource::ToString(WheelState_t value)
{
    return _ToString(mWheelStateIndexToStringMap, value);
}

QString ConoscopeResource::ToString(TemperatureMonitoringState_t value)
{
    return _ToString(mTemperatureMonitoringToStringMap, value);
}

int ConoscopeResource::Convert(ResourceType_t eType, QString value)
{
    int output;

    switch(eType)
    {
    case ResourceType_Filter:
        output = _ToInt(mFilterToStringMapRevert, value);
        break;

    case ResourceType_Nd:
        output = _ToInt(mNdToStringMapRevert, value);
        break;

    case ResourceType_Iris:
        output = _ToInt(mIrisIndexToStringMapRevert, value);
        break;

    default:
        break;
    }

    return output;
}

void ConoscopeResource::SetLogger(Logger* logger)
{
    mLogger = logger;
}

void ConoscopeResource::AppendLog(QString msg)
{
    if(mLogger)
    {
        mLogger->Append(msg);
    }
}

void ConoscopeResource::AppendLog(QString header, QString msg)
{
    if(mLogger)
    {
        mLogger->Append(header, msg);
    }
}

void ConoscopeResource::RegisterLogCallback(void (*callback)(char*))
{
    mLogCallback = callback;
}

void ConoscopeResource::RegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString))
{
    mEventCallback = callback;
}

void ConoscopeResource::RegisterWarningCallback(void (*callback)(QString))
{
    mWarningCallback = callback;
}

static char * cstr = new char [0];

void ConoscopeResource::Log(QString message)
{
    if(mLogCallback != nullptr)
    {
        // convert QString into char array
        std::string str = message.toStdString();
        // fix here
        delete[] cstr;
        cstr = new char [str.length()+1];

        strcpy_s (cstr, str.length()+1, str.c_str());

        // use of calback
        mLogCallback(cstr);
    }
}

void ConoscopeResource::SendEvent(ConoscopeEvent_t event)
{
    if(mEventCallback != nullptr)
    {
        QString description = ToolReturnCode::_mErrorDescription;
        mEventCallback(event, description);
    }
}

void ConoscopeResource::SendWarning(QString description)
{
    if(mWarningCallback != nullptr)
    {
        if(description == "")
        {
            description = ToolReturnCode::_mErrorDescription;
        }

        mWarningCallback(description);
    }
}

void ConoscopeResource::TaktTimeStart()
{
    taktTimer.start();
}

qint64 ConoscopeResource::TaktTimeMs()
{
    return taktTimer.elapsed();
}
