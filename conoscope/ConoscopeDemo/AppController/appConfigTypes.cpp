#include "appConfigTypes.h"

#include "appResource.h"

std::map<LogMask_t, QString> LogMask::mLogMaskToStringMap = {
    {LogMask_State, "State"},
    {LogMask_StateMachine, "StateMachine"},
    {LogMask_Worker, "Worker"}};

std::map<QString, LogMask_t> LogMask::mLogMaskToStringRevMap = {
    {"State",        LogMask_State},
    {"StateMachine", LogMask_StateMachine},
    {"Worker",       LogMask_Worker}};

LogMask::LogMask()
{

}

LogMask::LogMask(QList<LogMask_t> input)
{
    mLogMasks = input;
}

LogMask::LogMask(QJsonArray input)
{
    mLogMasks.clear();

    for(int index = 0; index < input.count(); index ++)
    {
        QString key = input[index].toString();

        if(LogMask::mLogMaskToStringRevMap.count(key))
        {
            int value = LogMask::mLogMaskToStringRevMap[key];
            mLogMasks.append((LogMask_t) value);
        }
    }
}

QJsonArray LogMask::GetJson()
{
     QJsonArray array;

    for(int index = 0; index < mLogMasks.count(); index ++)
    {
        QString maskName = LogMask::mLogMaskToStringMap[mLogMasks[index]];
        array.append(maskName);
    }

     return array;
}

bool LogMask::IsPresent(LogMask_t mask)
{
    bool res = true;

    if((mask != LogMask_Any) && (mask != LogMask_Error))
    {
        res = mLogMasks.contains(mask);
    }

    return res;
}

