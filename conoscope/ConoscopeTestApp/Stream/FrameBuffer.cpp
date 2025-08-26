#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(QObject *parent) : QObject(parent)
{
    mState = State::Free;
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::Fill(std::vector<int16_t>* pData)
{
    if(mState == State::Free)
    {
        mState = State::Filling;

        int size = (int)pData->size();

        int vectorSize = size;
        data.resize(vectorSize);

        memcpy(&data[0], pData->data(), size * sizeof(uint16_t));

        mState = State::ReadyToDisplay;
    }
}

void FrameBuffer::Release()
{
    mState = State::Free;
}
