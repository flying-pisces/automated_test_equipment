#ifndef DIALOGSTREAM_H
#define DIALOGSTREAM_H

#include <QDialog>
#include "FrameBuffer.h"

namespace Ui {
class DialogStream;
}

class DialogStream : public QDialog
{
    Q_OBJECT

public:
    explicit DialogStream(QWidget *parent = 0);
    ~DialogStream();

    void Start();

    FrameBuffer* pFrameBuffer;

public slots:
    void onRawBufferReady();

private:
    Ui::DialogStream *ui;

    int mFrameIndex;

};

#endif // DIALOGSTREAM_H
