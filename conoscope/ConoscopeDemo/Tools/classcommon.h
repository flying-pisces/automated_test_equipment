#ifndef CLASSCOMMON_H
#define CLASSCOMMON_H

#include <QObject>
#include <QThread>
#include <QMetaEnum>
#include <QVector>
#include <QRect>

class ClassCommon : public QObject
{
    Q_OBJECT

public:
    enum class Error{
        Ok,
        Failed,
        ViFailed,
        InvalidParameter,
        InvalidState,
        NotImplemented,
        FailedMaxRetry,
        Aborted,
        Timeout,
        InvalidConfiguration
    };
    Q_ENUM(Error)

public:
    explicit ClassCommon(QObject *parent = nullptr);
    ~ClassCommon();

    static QString ErrorToString(Error eError);

#ifdef REMOVED
    static void Wait(int timeMs);
#endif

signals:
    void WriteStatus(QString log);
    void WriteLog(QString log);

protected:
    void Status(QString log);
    void Log(QString log);
    void Log(QString header, QString log);

    static QString GetEnumString(const QMetaObject &mo, const char* enumName, int enumValue)
    {
        int index = mo.indexOfEnumerator(enumName);
        if(index == -1)
        {
            return "error";
        }
        else
        {
            QMetaEnum metaEnum = mo.enumerator(index);
            return metaEnum.valueToKey(static_cast<int>(enumValue));
        }
    }

    /* check whether the value is in the enum list */
    static bool IsEnumValidValue(const QMetaObject &mo, const char* enumName, int enumValue)
    {
        bool isValid = false;

        QString keyString = GetEnumString(mo, enumName, enumValue);

        if(!keyString.isEmpty())
        {
            isValid = true;
        }

        return isValid;
    }
};

class ClassThreadCommon : public QThread
{
    Q_OBJECT

public:
    explicit ClassThreadCommon(QObject *parent = nullptr);
    ~ClassThreadCommon();

signals:
    void WriteStatus(QString log);
    void WriteLog(QString log);

    /* signal sent to the worker to request a job to be done */
    void WorkRequest(int value);
    void WarningMessage(QString message);

    /* signal to indicate the state has changed */
    void StateChange(int state);

protected:
    void Status(QString log);
    void Log(QString log);
    void Log(QString header, QString log);

    static QString GetEnumString(const QMetaObject &mo, const char* enumName, int enumValue)
    {
        int index = mo.indexOfEnumerator(enumName);
        if(index == -1)
        {
            return "error";
        }
        else
        {
            QMetaEnum metaEnum = mo.enumerator(index);
            return metaEnum.valueToKey(static_cast<int>(enumValue));
        }
    }
};

void AddToAverage(
        double* measureAverage,
        int* sampleCount,
        double measure);

QString GetRectInfo (QRect rect);

#endif // CLASSCOMMON_H
