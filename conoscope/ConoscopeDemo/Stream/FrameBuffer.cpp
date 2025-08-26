#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(QObject *parent) : QObject(parent)
{
    mState = State::Free;
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::Fill(std::vector<int16_t>* pData, int whiteLevel)
{
    if(mState == State::Free)
    {
        mState = State::Filling;

        int size = (int)pData->size();

        int vectorSize = size;
        data.resize(vectorSize);

        // all values must be positives
#pragma omp parallel for num_threads(4)
        for(int index = 0; index < (int)pData->size(); index ++)
        {
            if(pData->at(index) < 0)
            {
                pData->at(index) = 0;
            }
            else if(pData->at(index) > whiteLevel)
            {
                pData->at(index) = whiteLevel;
            }
        }

        memcpy(&data[0], pData->data(), size * sizeof(uint16_t));

        mState = State::ReadyToDisplay;
    }
}

void FrameBuffer::Release()
{
    mState = State::Free;
}
