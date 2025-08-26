#ifndef TOOLLOGGER_H
#define TOOLLOGGER_H

#include <QJsonObject>
#include <QMap>

#include "classcommon.h"

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

    QMap<QString, QVariant> GetOption();

    QVariant GetOption(QString key);

private:
    ClassCommon::Error mError;
    QString mMessage;
    QMap<QString, QVariant> mOption;
};

#endif
