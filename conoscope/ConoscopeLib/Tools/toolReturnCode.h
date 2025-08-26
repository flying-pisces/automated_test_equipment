#ifndef TOOLLOGGER_H
#define TOOLLOGGER_H

#include <QJsonObject>
#include <QMap>

#include "classcommon.h"

#define ERROR_DESCRIPTION(msg) ToolReturnCode::SetErrorDescription((eError == ClassCommon::Error::Ok) ? "" : msg)

class ToolReturnCode
{
public:
    explicit ToolReturnCode();

    explicit ToolReturnCode(ClassCommon::Error eError);

    explicit ToolReturnCode(QString eError);

    ~ToolReturnCode();

    void SetError(ClassCommon::Error eError);

    void SetError(QString message);

    void SetOption(QString key, QVariant value);

    QString GetJsonCode();

    ClassCommon::Error GetError();

    QString GetMessage();

    QString GetErrorDescription();

    QMap<QString, QVariant> GetOption();

    QVariant GetOption(QString key);

    static void SetErrorDescription(QString description)
    {
        _mErrorDescription = description;
    }

    static QString _mErrorDescription;

private:
    ClassCommon::Error mError;
    QString mMessage;
    QString mErrorDescription;

    QMap<QString, QVariant> mOption;
};

#endif
