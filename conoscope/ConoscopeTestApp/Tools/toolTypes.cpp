#include "toolTypes.h"

CameraSettingItem::CameraSettingItem (QString _type, QString _name, float _value, QString _unit)
{
    type = _type;
    name = _name;
    value = _value;
    unit = _unit;
}

CameraSettings::CameraSettings()
{
    mSettings.clear();
}

void CameraSettings::Copy (CameraSettings item)
{
    mSettings.clear();

    int length = item.Length();

    for(int index = 0; index < length; index ++)
    {
        CameraSettingItem *ptr = item.Get(index);

        mSettings.append(*ptr);
    }
}

void CameraSettings::Clear()
{
    mSettings.clear();
}

void CameraSettings::Append(CameraSettingItem setting)
{
    mSettings.append(setting);
}

float CameraSettings::GetValue(QString name)
{
    float output = -1.0;
    int index = 0;

    while(index < mSettings.length())
    {
        if(mSettings[index].name == name)
        {
            output = mSettings[index].value;
            index = mSettings.length();
        }

        index ++;
    }

    return output;
}

CameraSettingItem *CameraSettings::Get(int index)
{
    if(index < mSettings.length())
    {
        return &mSettings[index];
    }

    return nullptr;
}

int CameraSettings::Length()
{
    return mSettings.length();
}

