#include "toolLogger.h"
#include <QDir>

void ToolLogger::CreateDir(QString & path)
{
    if(m_enable == true)
    {
        QDir dir(path);
        if (!dir.exists())
        {
            dir.mkpath(path);
        }
    }
}

ToolLogger::ToolLogger(QString fileName, QObject *parent) : QObject(parent)
{
    m_showDate = true;
    m_enable = true;

    file = new QFile;

    if (!fileName.isEmpty())
    {
        file->setFileName(fileName);
        file->open(QIODevice::Append | QIODevice::Text);
    }
}

ToolLogger::~ToolLogger()
{
    file->close();
    delete file;
}

void ToolLogger::handleFeatureUpdate()
{
}

void ToolLogger::write(const QString &value)
{
    if((m_enable == true) &&
       (file->isOpen() == true))
    {
        QString text = value + "\n";

        if(m_showDate == true)
        {
            text = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz, ") + text;
        }

        QTextStream out(file);

        out.setCodec("UTF-8");
        out << text;
    }
}

void ToolLogger::setShowDateTime(bool value)
{
    m_showDate = value;
}

bool ToolLogger::SetFileName(QString fileName, QString path)
{
    bool bRes = true;

    file->close();

    if(m_enable == true)
    {
        CreateDir(path);

        if(!fileName.isEmpty())
        {
            // append date to the file name
            QString header = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_");
            QString trailer = QString(".txt");

            file->setFileName(path + "\\"+ header + fileName + trailer);
            bRes = file->open(QIODevice::Append | QIODevice::Text);
        }
        else
        {
            bRes = false;
        }
    }
    else
    {
        bRes = true;
    }

    return bRes;
}

void ToolLogger::Close()
{
    file->close();
}

LocalLogger* LocalLogger::m_instance = NULL;
