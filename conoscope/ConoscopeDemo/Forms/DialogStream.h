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

    void ClearScreen();

public slots:
    void onRawBufferReady(int width, int height, int whiteLevel);

    void onRawBufferAeRoi(int AEMeasAreaWidth,
                          int AEMeasAreaHeight,
                          int AEMeasAreaX,
                          int AEMeasAreaY);

private:
    Ui::DialogStream *ui;

    int mFrameIndex;

    int mAEMeasAreaWidth;
    int mAEMeasAreaHeight;
    int mAEMeasAreaX;
    int mAEMeasAreaY;
};

#endif // DIALOGSTREAM_H
