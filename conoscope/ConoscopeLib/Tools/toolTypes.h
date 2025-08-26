#ifndef TOOL_TYPES_H
#define TOOL_TYPES_H

#include <QString>
#include <QList>

template <class T>
class ParamData
{
public:
    T data;
    bool valid;

    ParamData()
    {
        valid = false;
    }

    ParamData<T> &operator=(const T &dataValue)
    {
        data = dataValue;
        valid = true;

        return *this;
    }

    ParamData( const ParamData& value)
    {
        data  = value.data;
        valid = value.valid;
    }

    // new interface
    void SetValue(T value, bool bValid = true)
    {
        data = value;
        valid = bValid;
    }

    void SetValid(bool bValid)
    {
        valid = bValid;
    }

    T GetValue()
    {
        return data;
    }

    bool GetValid()
    {
        return valid;
    }
};

class CameraSettingItem
{
public:
    QString type;
    QString name;
    float value;
    QString unit;

    CameraSettingItem (QString _type, QString _name, float _value, QString _unit);
};

class CameraSettings
{

public:
    CameraSettings();

    void Copy (CameraSettings item);

    void Clear();

    void Append(CameraSettingItem setting);

    float GetValue(QString name);

    CameraSettingItem* Get(int index);

    int Length();

private:
    QList<CameraSettingItem> mSettings;
};

#endif // TOOL_TYPES_H
