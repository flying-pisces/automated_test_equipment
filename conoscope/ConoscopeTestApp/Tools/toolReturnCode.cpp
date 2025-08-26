#include "toolReturnCode.h"

#include <QJsonDocument>
#include <QJsonArray>

#include "conoscopeTypes.h"

ToolReturnCode::ToolReturnCode()
{
    SetError(ClassCommon::Error::Ok);
    SetError("N.A.");

    mOption.clear();
}

ToolReturnCode::ToolReturnCode(ClassCommon::Error eError)
{
    SetError(eError);

    mOption.clear();
}

ToolReturnCode::ToolReturnCode(QString value)
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

        if(itemName == RETURN_ITEM_ERROR)
        {
            mError = (ClassCommon::Error)itemValue.toInt();
        }
        else if(itemName == RETURN_ITEM_MESSAGE)
        {
            mMessage = itemValue.toString();
        }
        else
        {
            mOption[itemName] = itemValue;
        }
    }
}

ToolReturnCode::~ToolReturnCode()
{

}

void ToolReturnCode::SetError(ClassCommon::Error eError)
{
    mError = eError;
    mMessage = ClassCommon::ErrorToString(eError);
}

void ToolReturnCode::SetError(QString message)
{
    mMessage = message;
}

void ToolReturnCode::SetOption(QString key, QVariant value)
{
    mOption[key] = value;
}

QString ToolReturnCode::GetJsonCode()
{
    QJsonObject content;

    content.insert(RETURN_ITEM_ERROR, (int)mError);
    content.insert(RETURN_ITEM_MESSAGE, mMessage);

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

ClassCommon::Error ToolReturnCode::GetError()
{
    return mError;
}

QString ToolReturnCode::GetMessage()
{
    return mMessage;
}

QMap<QString, QVariant> ToolReturnCode::GetOption()
{
    return mOption;
}

QVariant ToolReturnCode::GetOption(QString key)
{
    QVariant value;

    if(mOption.contains(key))
    {
        value = mOption[key];
    }

    return value;
}

