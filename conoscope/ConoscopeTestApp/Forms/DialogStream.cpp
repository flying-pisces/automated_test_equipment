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
    mFrameIndex = 0;
}

void DialogStream::onRawBufferReady()
{
    ui->lblInfo->setText(QString ("frame %1").arg(mFrameIndex ++));
    ui->imgWidget->SetRawData(pFrameBuffer->data, 7920, 6004);

    pFrameBuffer->Release();
}
