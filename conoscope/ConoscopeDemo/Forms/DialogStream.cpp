#include "DialogStream.h"
#include "ui_DialogStream.h"

DialogStream::DialogStream(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogStream)
{
    ui->setupUi(this);

    mFrameIndex = 0;
}

DialogStream::~DialogStream()
{
    delete ui;
}

void DialogStream::Start()
{
    ClearScreen();
}

void DialogStream::onRawBufferReady(int width, int height, int whiteLevel)
{
    QString part2 = "";

    if(pFrameBuffer->mMessage.isEmpty() == false)
    {
        part2 = QString(" [%1]").arg(pFrameBuffer->mMessage);
    }

    ui->lblInfo->setText(QString ("frame %1%2").arg(mFrameIndex ++).arg(part2));

    ui->imgWidget->SetWhiteLevel(whiteLevel);

    ui->imgWidget->SetAeRoi(mAEMeasAreaWidth,
                            mAEMeasAreaHeight,
                            mAEMeasAreaX,
                            mAEMeasAreaY);

    ui->imgWidget->SetRawData(pFrameBuffer->data, width, height);

    pFrameBuffer->Release();
}

void DialogStream::onRawBufferAeRoi(int AEMeasAreaWidth,
                                    int AEMeasAreaHeight,
                                    int AEMeasAreaX,
                                    int AEMeasAreaY)
{
    mAEMeasAreaWidth  = AEMeasAreaWidth;
    mAEMeasAreaHeight = AEMeasAreaHeight;
    mAEMeasAreaX      = AEMeasAreaX;
    mAEMeasAreaY      = AEMeasAreaY;
}

void DialogStream::ClearScreen()
{
    mFrameIndex = 0;

    ui->lblInfo->setText(QString ("Waiting..."));

    int height = 10;
    int width = 10;

    QVector<uint16_t> source;
    source.resize(width * height);

    memset(source.data(), 0, width * height * sizeof(uint16_t));

    ui->imgWidget->SetRawData(source, width, height, true);
}
