#ifndef COAXPRESS_TYPES_H
#define COAXPRESS_TYPES_H

typedef enum
{
    PixelFormat_Mono8,
    PixelFormat_Mono10,
    PixelFormat_Mono12,
    PixelFormat_Unknown,
} PixelFormat_t;

typedef enum
{
    CameraManufacturer_Adimec,
    CameraManufacturer_CriticalLink,
    CameraManufacturer_Unknown
} CameraManufacturer_t;

class ImageFeature
{
public:
    int width;
    int height;
    int offsetX;
    int offsetY;
    PixelFormat_t eFormat;
    CameraManufacturer_t eCameraManufacturer;

    ImageFeature();

    ImageFeature(const ImageFeature& feature);
};

class ImageBuffer
{
public:
    void *ptr;
    size_t bufSize;
    int width;
    int height;

    ImageBuffer();
};

#endif // COAXPRESS_TYPES_H
