#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class FrameBuffer : public QObject
{
    Q_OBJECT

public:
    enum class State {
        Free,
        Filling,
        ReadyToDisplay,
        Displaying
    };
    Q_ENUM(State)

public:
    explicit FrameBuffer(QObject *parent = nullptr);
    ~FrameBuffer();

    void Fill(std::vector<int16_t> *pData, int whiteLevel);

    void Release();

    State mState;
    QVector<uint16_t> data;
    QString mMessage;
};

#endif /* FRAMEBUFFER_H */
