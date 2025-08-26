#include "classcommon.h"

#include <QMetaEnum>
#include "toolString.h"

ClassCommon::ClassCommon(QObject *parent) : QObject(parent)
{

}

ClassCommon::~ClassCommon()
{

}

QString ClassCommon::ErrorToString(ClassCommon::Error eError)
{
    const QMetaObject &mo = ClassCommon::staticMetaObject;
    int index = mo.indexOfEnumerator("Error");
    QMetaEnum metaEnum = mo.enumerator(index);
    QString valueString = metaEnum.valueToKey(static_cast<int>(eError));
    return valueString;
}

#ifdef REMOVED
#include <QApplication>

#define TIME_QUANTA_MS 10

void ClassCommon::Wait(int timeMs)
{
    do
    {
        QThread::msleep(TIME_QUANTA_MS);
        QApplication::processEvents(QEventLoop::AllEvents, TIME_QUANTA_MS);

        timeMs -= TIME_QUANTA_MS;
    } while(timeMs > 0);
}
#endif

void ClassCommon::Status(QString log)
{
    emit WriteStatus(log);
}

void ClassCommon::Log(QString log)
{
    emit WriteLog(log);
}

void ClassCommon::Log(QString header, QString log)
{
    emit WriteLog(ToolsString::FormatText(header, log));
}

ClassThreadCommon::ClassThreadCommon(QObject *parent) : QThread(parent)
{

}

ClassThreadCommon::~ClassThreadCommon()
{

}

void ClassThreadCommon::Status(QString log)
{
    emit WriteStatus(log);
}

void ClassThreadCommon::Log(QString log)
{
    emit WriteLog(log);
}

void ClassThreadCommon::Log(QString header, QString log)
{
    emit WriteLog(ToolsString::FormatText(header, log));
}

void AddToAverage(
        double* measureAverage,
        int* sampleCount,
        double measure)
{
    // increment number of sample
    *sampleCount = *sampleCount + 1;
    // calculate cumulative moving average
    double part = measure - *measureAverage;
    part = part / *sampleCount;
    *measureAverage = *measureAverage + part;
}

QString GetRectInfo(QRect rect)
{
    return QString("%1x%2+%3+%4")
                .arg(rect.height())
                .arg(rect.width())
                .arg(rect.left())
                .arg(rect.top());
}



