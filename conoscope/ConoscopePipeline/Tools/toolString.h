#ifndef TOOL_STRING_H
#define TOOL_STRING_H

#include <QObject>
#include <QString>

#define ROW_LENGTH 30
#define ROW_SEPARATOR ": "

class ToolsString : public QObject
{
    // Q_OBJECT

public:
    static QString FormatText(QString header, QString input);

    static QString FixedLength(QString input, int length);
};

#endif // TOOL_STRING_H
