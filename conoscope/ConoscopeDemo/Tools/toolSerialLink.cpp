#include "toolSerialLink.h"
#include <QSerialPort>
#include "picoammeter/picoammeterdata.h"

#define VOID_PORT_NAME "disconnect"

ToolSerialLink::ToolSerialLink(QObject *parent) : ClassCommon(parent)
{
    m_serialPort = new QSerialPort (parent);
    m_baudRate = QSerialPort::Baud57600;

    connect(m_serialPort, &QSerialPort::readyRead, this, &ToolSerialLink::handleReadyRead);

    m_state = State::NotConnected;
}

ToolSerialLink::~ToolSerialLink()
{
    if (m_serialPort->isOpen())
    {
        m_serialPort->close();
    }
}

ClassCommon::Error ToolSerialLink::SetBaudRate(int baudRate)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    const QMetaObject &mo = QSerialPort::staticMetaObject;

    if(IsEnumValidValue(mo, "BaudRate", baudRate))
    {
        m_baudRate = baudRate;
    }
    else
    {
        eError = ClassCommon::Error::InvalidParameter;
    }

    return eError;
}

ToolSerialLink::OpenStatus ToolSerialLink::Open(const QString& portName)
{
    ToolSerialLink::OpenStatus status;

#ifdef TOGGLE_CONNECTION
    if(portName == VOID_PORT_NAME)
    {
        m_serialPort->close();
        status = ToolSerialLink::OpenStatus::NotConnected;
        m_state = State::NotConnected;
    }
    else if((m_serialPort->portName() != portName) ||
            (m_serialPort->isOpen() == false))
#else

    // close current connection if it does match current one
    if((m_serialPort->portName() != portName) &&
       (m_serialPort->isOpen() == true))
    {
        m_serialPort->close();
        m_state = State::NotConnected;
        status = ToolSerialLink::OpenStatus::NotConnected;
    }

    if(m_serialPort->isOpen() == false)
#endif
    {
        // open the port
        m_serialPort->close();
        m_serialPort->setPortName(portName);

        m_serialPort->setBaudRate(m_baudRate);
        m_serialPort->setDataBits(QSerialPort::Data8);
        m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
        m_serialPort->setParity(QSerialPort::NoParity);

        if(m_serialPort->open(QIODevice::ReadWrite))
        {
            status = ToolSerialLink::OpenStatus::Connected;
            m_state = State::Connected;
        }
        else
        {
            // not connected or already connected
            status = ToolSerialLink::OpenStatus::NotConnectedError;
            m_state = State::NotConnected;
        }
    }
#ifdef TOGGLE_CONNECTION
    else
    {
        // already connected - disconnect
        status = ToolSerialLink::OpenStatus::NotConnected;
        m_serialPort->close();
        m_state = State::NotConnected;
    }
#else
    else
    {
        // the connection correctly open
        // else it would have been closed at the beginning of this function
        status = ToolSerialLink::OpenStatus::Connected;
        m_state = State::Connected;
    }
#endif

    return status;
}

ClassCommon::Error ToolSerialLink::Close()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // may also check m_state
    if(m_serialPort->isOpen() == true)
    {
        m_serialPort->close();
        m_state = State::NotConnected;
    }
    else
    {
        eError = ClassCommon::Error::InvalidState;
    }

    return eError;
}

ToolSerialLink::State ToolSerialLink::GetState()
{
    return m_state;
}

void ToolSerialLink::handleReadyRead()
{
    // read serial link data
    QByteArray readData = m_serialPort->readAll();
    emit dataReceived(readData);
}

void ToolSerialLink::handleDataToSend(const QByteArray& data)
{
    m_serialPort->write(data);
}

