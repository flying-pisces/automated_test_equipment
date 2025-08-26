#include "toolErrorCode.h"

#include <QJsonDocument>
#include <QJsonArray>

#define ERROR_ITEM "Error"
#define MESSAGE_ITEM "Message"

ToolErrorCode::ToolErrorCode()
{
    SetError(ClassCommon::Error::Ok);
    SetError("N.A.");

    mOption.clear();
}

ToolErrorCode::ToolErrorCode(ClassCommon::Error eError)
{
    SetError(eError);

    mOption.clear();
}

ToolErrorCode::ToolErrorCode(QString value)
{
    QJsonDocument jsonResponse = QJsonDocument::fromJson(value.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    //convert the json object to variantmap
    QVariantMap jsonObjectMap = jsonObject.toVariantMap();

    QMapIterator<QString, QVariant> iter(jsonObjectMap);
    while (iter.hasNext())
    {
        iter.next();

        QString itemName = iter.key();
        QVariant itemValue = iter.value();
        // QVariant::Type itemValueType = itemValue.type();

        if(itemName == ERROR_ITEM)
        {
            mError = (ClassCommon::Error)itemValue.toInt();
        }
        else if(itemName == MESSAGE_ITEM)
        {
            mMessage = itemValue.toString();
        }
        else
        {
            mOption[itemName] = itemValue;
        }
    }
}

ToolErrorCode::~ToolErrorCode()
{

}

void ToolErrorCode::SetError(ClassCommon::Error eError)
{
    mError = eError;
    mMessage = ClassCommon::ErrorToString(eError);
}

void ToolErrorCode::SetError(QString message)
{
    mMessage = message;
}

void ToolErrorCode::SetOption(QString key, QVariant value)
{
    mOption[key] = value;
}

QString ToolErrorCode::GetJsonCode()
{
    QJsonObject content;

    content.insert(ERROR_ITEM, (int)mError);
    content.insert(MESSAGE_ITEM, mMessage);

    QMapIterator<QString, QVariant> it(mOption);
    while (it.hasNext())
    {
        it.next();
        QString valueStr = it.value().toString();
        content.insert(it.key(), valueStr);
    }

    QJsonDocument doc(content);
    return doc.toJson(QJsonDocument::Compact);
}

ClassCommon::Error ToolErrorCode::GetError()
{
    return mError;
}

QString ToolErrorCode::GetMessage()
{
    return mMessage;
}

QMap<QString, QVariant> ToolErrorCode::GetOption()
{
    return mOption;
}



