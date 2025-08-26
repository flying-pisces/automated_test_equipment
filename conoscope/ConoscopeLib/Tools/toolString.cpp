#include "toolString.h"

QString ToolsString::FormatText(QString header, QString input)
{
    QString value = QString(ROW_LENGTH, ' ');

    value.prepend(header);
    value.truncate(ROW_LENGTH);
    value.append(ROW_SEPARATOR);
    value.append(input);

    return value;
}

QString ToolsString::FixedLength(QString input, int length)
{
    QString value = QString(length, ' ');

    value.prepend(input);
    value.truncate(length);

    return value;
}
