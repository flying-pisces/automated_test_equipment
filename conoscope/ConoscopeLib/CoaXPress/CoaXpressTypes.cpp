#include "CoaXpressTypes.h"

ImageFeature::ImageFeature()
{
    width     = 0;
    height    = 0;
    offsetX   = 0;
    offsetY   = 0;
    eFormat   = PixelFormat_Unknown;
    eCameraManufacturer = CameraManufacturer_Unknown;
}

ImageFeature::ImageFeature(const ImageFeature& feature)
{
    width         = feature.width;
    height        = feature.height;
    offsetX       = feature.offsetX;
    offsetY       = feature.offsetY;
    eFormat       = feature.eFormat;
    eCameraManufacturer   = feature.eCameraManufacturer;
}

ImageBuffer::ImageBuffer()
{
    ptr     = nullptr;
    bufSize = 0;
    width   = 0;
    height  = 0;
}

