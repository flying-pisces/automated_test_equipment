#ifndef TOOLLOGGER_H
#define TOOLLOGGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class ToolLogger : public QObject
{
    Q_OBJECT
public:
    explicit ToolLogger(QString fileName, QObject *parent = nullptr);
    ~ToolLogger();

    void setShowDateTime(bool value);

    void Enable(bool enable)
    {
        m_enable = enable;
    }

    bool SetFileName(QString fileName, QString path = "");

    void Close();

    void CreateDir(QString& path);

signals:

public slots:
    void handleFeatureUpdate();

public slots:
    void write(const QString &value);

private:
    QFile *file;
    bool m_showDate;
    bool m_enable;
};


#include "toolString.h"

class LocalLogger: public QObject
{
    Q_OBJECT

private:
    static LocalLogger* m_instance;

    explicit LocalLogger(QObject *parent = nullptr) : QObject(parent)
    {
    }

    ~LocalLogger()
    {
    }

    void _Print(QString log)
    {
        emit WriteLog(log);
    }

    static LocalLogger* _GetInstance()
    {
        return m_instance;
    }

signals:
    void WriteStatus(QString log);
    void WriteLog(QString log);

public:
    static LocalLogger* CreateInstance(QObject *parent = nullptr)
    {
        if(m_instance == NULL)
        {
            m_instance = new LocalLogger(parent);
        }

        return m_instance;
    }

    static void Print(QString str1)
    {
        _GetInstance()->_Print(str1);
    }

    static void Print(QString str1, QString str2)
    {
        _GetInstance()->_Print(ToolsString::FormatText(str1, str2));
    }
};

#endif // TOOLLOGGER_H
