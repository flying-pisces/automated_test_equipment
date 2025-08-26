#ifndef COAXPRESSFRAME_H
#define COAXPRESSFRAME_H

#include "CoaXpressConfiguration.h"
#include "CoaXpressTypes.h"

#include <vector>
#include <QElapsedTimer>

#define FRAME_FEATURE

class ImageFrame
{
public:
    ImageFeature mFeature;
    int mNbBytesPerPixel;

    void* srcBuf;
    int mPixelNumber;
    std::vector<uint16_t> mImageVector;
    int mAccumulatedCount;
    std::vector<uint32_t> mAccumulatedVector;

#ifdef STD_DEV_FILE
    bool bStoreStdDev;
    std::vector<uint32_t> mAccumulatedSquareVector;

#ifndef STD_DEV_FLOAT
    std::vector<uint16_t> mStdVector;
#else
    std::vector<float> mStdVector;
#endif

#endif

    int debugStoreTime;

    ImageFrame()
    {
    }

    void SetFeature(ImageFeature& feature);

#ifdef FRAME_FEATURE
    int Initialise(int height, int width);

    int mHeight;
    int mWidth;
#endif
};

class CoaXpressFrame
{
private:
    static CoaXpressFrame* mInstance;

    CoaXpressFrame();

    static CoaXpressFrame* GetInstance();

protected:
    std::vector<ImageFrame> mFrame;
    int   mFreeIndex; // currently not used
    QElapsedTimer debugTimer; // for debug purpose

public:

    static void SetFrameSize(ImageFeature feature);

    static int GetImageIndex();

    static int StoreImage(ImageBuffer& imageBuffer);

#ifdef STD_DEV_FILE
    static void StoreStdDev(int imageIndex, bool bEnable);
#endif

    static void AppendImage(int imageIndex, ImageBuffer& imageBuffer);

    static ImageFrame* GetImage(int imageIndex);
};

#endif // COAXPRESSFRAME_H
