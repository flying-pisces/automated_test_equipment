#ifndef TOOLSERIALLINK_H
#define TOOLSERIALLINK_H

#include <QObject>

#include <QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "classcommon.h"

class ToolSerialLink : public ClassCommon
{
    Q_OBJECT
public:
    enum class OpenStatus{
        Connected,
        NotConnected,
        NotConnectedError
    };
    Q_ENUM(OpenStatus)

    enum class State{
        Connected,
        NotConnected
    };
    Q_ENUM(State)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = ToolSerialLink::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

    explicit ToolSerialLink(QObject *parent = nullptr);

    ~ToolSerialLink();

    ClassCommon::Error SetBaudRate(int baudRate);

    ToolSerialLink::OpenStatus Open(const QString &portName);

    ClassCommon::Error Close();

    ToolSerialLink::State GetState();

signals:
    void dataReceived(QByteArray data);

private slots:
    void handleReadyRead();

public slots:
    void handleDataToSend(const QByteArray &data);

private:
    QSerialPort* m_serialPort;
    State        m_state;
    qint32       m_baudRate;
};

#endif // TOOLSERIALLINK_H
