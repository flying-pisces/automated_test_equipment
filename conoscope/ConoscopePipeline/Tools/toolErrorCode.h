#ifndef TOOLLOGGER_H
#define TOOLLOGGER_H

#include <QJsonObject>
#include <QMap>

#include "classcommon.h"

class ToolErrorCode
{
public:
    explicit ToolErrorCode();

    explicit ToolErrorCode(ClassCommon::Error eError);

    explicit ToolErrorCode(QString eError);

    ~ToolErrorCode();

    void SetError(ClassCommon::Error eError);

    void SetError(QString message);

    void SetOption(QString key, QVariant value);

    QString GetJsonCode();

    ClassCommon::Error GetError();

    QString GetMessage();

    QMap<QString, QVariant> GetOption();

private:
    ClassCommon::Error mError;
    QString mMessage;
    QMap<QString, QVariant> mOption;
};

#endif
