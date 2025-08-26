#include "CoaXpressFrame.h"

CoaXpressFrame* CoaXpressFrame::mInstance = NULL;

void ImageFrame::SetFeature(ImageFeature& feature)
{
    mFeature = feature;

    mNbBytesPerPixel = 1;
    if(feature.eFormat != PixelFormat_Mono8)
    {
        mNbBytesPerPixel = 2;
    }

    // TODO maybe concider the pixel format (if ever it is more than 8 bits)
    mPixelNumber = mFeature.height * mFeature.width;

    mImageVector.clear();
    mImageVector.resize(mPixelNumber);

    mAccumulatedVector.clear();
    mAccumulatedVector.resize(mPixelNumber);

#ifdef STD_DEV_FILE
    mAccumulatedSquareVector.clear();
    mAccumulatedSquareVector.resize(mPixelNumber);

    mStdVector.clear();
    mStdVector.resize(mPixelNumber);
#endif

    mAccumulatedCount = 0;

    debugStoreTime = 0;
}

#ifdef FRAME_FEATURE
int ImageFrame::Initialise(int height, int width)
{
    mHeight = height;
    mWidth = width;

    int pixelNumber = height * width;

    if(mAccumulatedCount == 0)
    {
        memset(mAccumulatedVector.data(), 0, pixelNumber * sizeof(uint32_t));
    }

    return  mHeight * mWidth;
}
#endif

CoaXpressFrame::CoaXpressFrame()
{
    mFrame.clear();
    mFrame.resize(NUMBER_OF_FRAME);

    mFreeIndex = 0;
}

CoaXpressFrame* CoaXpressFrame::GetInstance()
{
    if(mInstance == NULL)
    {
        mInstance = new CoaXpressFrame();
    }

    return mInstance;
}

void CoaXpressFrame::SetFrameSize(ImageFeature feature)
{
    CoaXpressFrame* instance = GetInstance();

    int index = 0;

    while(index < (int)instance->mFrame.size())
    {
        instance->mFrame.at(index).SetFeature(feature);
        index ++;
    }
}

int CoaXpressFrame::GetImageIndex()
{
    // for now there is only one frame
    return 0;
}

int CoaXpressFrame::StoreImage(ImageBuffer& imageBuffer)
{
    CoaXpressFrame* instance = GetInstance();

    instance->debugTimer.start();

    // uint8_t*  pData8  = (uint8_t*)imageBuffer.ptr;
#ifdef REMOVED
    uint16_t* pData16 = (uint16_t*)imageBuffer.ptr;
#endif

    int bufferSize = (int)instance->mFrame.at(instance->mFreeIndex).mImageVector.size();
    bufferSize *= instance->mFrame.at(instance->mFreeIndex).mNbBytesPerPixel;
    uint8_t* pDst = (uint8_t*)instance->mFrame.at(instance->mFreeIndex).mImageVector.data();
    uint8_t* pSrc = (uint8_t*)imageBuffer.ptr;

    int memoryChunckSize = instance->mFrame.at(instance->mFreeIndex).mFeature.width *
                           instance->mFrame.at(instance->mFreeIndex).mNbBytesPerPixel;

    int remain = bufferSize;

    while(remain > 0)
    {
        memcpy(pDst,
               pSrc,
               memoryChunckSize);

        pDst += memoryChunckSize;
        pSrc += memoryChunckSize;

        remain -= memoryChunckSize;
    }

    instance->mFrame.at(instance->mFreeIndex).debugStoreTime = instance->debugTimer.elapsed();

    return 0;
}

#ifdef STD_DEV_FILE
void CoaXpressFrame::StoreStdDev(int imageIndex, bool bEnable)
{
    CoaXpressFrame* instance = GetInstance();
    instance->mFrame.at(imageIndex).bStoreStdDev = bEnable;
}
#endif

void CoaXpressFrame::AppendImage(int imageIndex, ImageBuffer& imageBuffer)
{
    CoaXpressFrame* instance = GetInstance();

    instance->debugTimer.start();

    uint16_t* pData16 = (uint16_t*)imageBuffer.ptr;

    int pixelNumber = instance->mFrame.at(imageIndex).mPixelNumber;

    int bufSize = instance->mFrame.at(imageIndex).mPixelNumber *
                  instance->mFrame.at(imageIndex).mNbBytesPerPixel;

    // check that image buffer fit into the frameBuffer
    if((int)imageBuffer.bufSize > bufSize)
    {
        // invalid size
        return;
    }

#ifdef FRAME_FEATURE
    instance->mFrame.at(imageIndex).mFeature.width = imageBuffer.width;
    instance->mFrame.at(imageIndex).mFeature.height = imageBuffer.height;
    // offset is not correct
    instance->mFrame.at(imageIndex).mFeature.offsetX = 0;
    instance->mFrame.at(imageIndex).mFeature.offsetY = 0;

    pixelNumber = instance->mFrame.at(imageIndex).Initialise(imageBuffer.height, imageBuffer.width);
#else
    if(instance->mFrame.at(imageIndex).mAccumulatedCount == 0)
    {
        memset(instance->mFrame.at(imageIndex).mAccumulatedVector.data(), 0, pixelNumber * sizeof(uint32_t));
    }
#endif

    uint32_t* pDataAverage = (uint32_t*)instance->mFrame.at(imageIndex).mAccumulatedVector.data();

#ifdef STD_DEV_FILE
    uint32_t* pDataStdDev = (uint32_t*)instance->mFrame.at(imageIndex).mAccumulatedSquareVector.data();
#endif

    if(instance->mFrame.at(imageIndex).mAccumulatedCount == 0)
    {
#pragma omp parallel for num_threads(4)
        for(int index = 0; index < pixelNumber; index ++)
        {
            pDataAverage[index] = pData16[index];
        }
    }
    else
    {
#pragma omp parallel for num_threads(4)
        for(int index = 0; index < pixelNumber; index ++)
        {
            pDataAverage[index] += pData16[index];
        }
    }

#ifdef STD_DEV_FILE
    if(instance->mFrame.at(imageIndex).bStoreStdDev == true)
    {
#pragma omp parallel for num_threads(4)
        for(int index = 0; index < pixelNumber; index ++)
        {
            // accumulate data for standard deviation
            // max number of sample is 256
            pDataStdDev[index] += pData16[index] * pData16[index];
        }
    }
#endif

    instance->mFrame.at(imageIndex).mAccumulatedCount ++;
}

ImageFrame* CoaXpressFrame::GetImage(int imageIndex)
{
    CoaXpressFrame* instance = GetInstance();

    if(instance->mFrame.at(imageIndex).mAccumulatedCount != 0)
    {
        int pixelNumber = instance->mFrame.at(imageIndex).mPixelNumber;
        int mAccumulatedCount = instance->mFrame.at(imageIndex).mAccumulatedCount;

        uint16_t* pDst = (uint16_t*)instance->mFrame.at(imageIndex).mImageVector.data();
        uint32_t* pAccumulated = (uint32_t*)instance->mFrame.at(imageIndex).mAccumulatedVector.data();

#pragma omp parallel for num_threads(4)
        for(int index = 0; index < pixelNumber; index ++)
        {
            pDst[index] = pAccumulated[index] / mAccumulatedCount;
        }

#ifdef STD_DEV_FILE
        // process the standard deviation
        uint32_t* pStdDev = (uint32_t*)instance->mFrame.at(imageIndex).mAccumulatedSquareVector.data();

#ifndef STD_DEV_FLOAT
        uint16_t* pDstStd = (uint16_t*)instance->mFrame.at(imageIndex).mStdVector.data();
#else
        float* pDstStd = (float*)instance->mFrame.at(imageIndex).mStdVector.data();
#endif

        if(instance->mFrame.at(imageIndex).bStoreStdDev == true)
        {
            double stdDev = 0;
            double avg = 0;

            double correctionFacteur = sqrt((double)mAccumulatedCount/((double)mAccumulatedCount - 1.));

            for(int index = 0; index < pixelNumber; index ++)
            {
                stdDev = pStdDev[index] / mAccumulatedCount;
                avg = pDst[index];
                stdDev = stdDev - (avg * avg);
                stdDev = sqrt(stdDev);

                // standard deviation on n-1 instead of n
                stdDev = stdDev * correctionFacteur;

                pDstStd[index] = stdDev;
            }
        }
#endif
    }

    instance->mFrame.at(imageIndex).mAccumulatedCount = 0;

    if(imageIndex < (int)instance->mFrame.size())
    {
        return &instance->mFrame.at(imageIndex);
    }
    else
    {
        return NULL;
    }
}

