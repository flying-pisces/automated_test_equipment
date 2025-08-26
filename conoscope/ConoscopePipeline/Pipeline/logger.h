#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QMutex>

/* Class LOGGER
 * handle the logging mechanism
 *
 */

#define SEPARATOR " | "
#define DATETIME_FORMAT "yyyy/MM/dd hh:mm:ss.zzz"

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(const QString &name, QObject *parent = 0) : QObject(parent)
    {
        logFile = new QFile(name);
    }

    ~Logger()
    {
        delete logFile;
    }

    void Init(const QString text)
    {
        logFile->open(QIODevice::WriteOnly);
        QTextStream ts(logFile);
        ts << QDateTime::currentDateTime().toString(DATETIME_FORMAT) << SEPARATOR << text << endl;
        logFile->close();
    }

    void Append(const QString text)
    {
        mMutex.lock();

        if(logFile->open(QIODevice::Append))
        {
          QTextStream ts(logFile);
          ts << QDateTime::currentDateTime().toString(DATETIME_FORMAT) << SEPARATOR << text << endl;
          logFile->close();
        }

        // qDebug() << text;

        mMutex.unlock();
    }

private:
    QFile* logFile;

    QMutex mMutex;
};

class DebugLogger
{
public:
    static void Print(std::string text)
    {
        qDebug() << text.data();
    }
};

#endif // LOGGER_H
