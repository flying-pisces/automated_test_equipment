#include "PipelineDefectCorrector.h"

#define OMP_PARAL

#define NEIGHBOR_DISTANCE 1
#define NEIGHBOR_ARRAY_SIZE(x) (x*2+1)*(x*2+1)

#define _ACTIVE_WIDTH                  m_ImageConfiguration.active_width
#define _ACTIVE_HEIGHT                 m_ImageConfiguration.active_height
#define _ACTIVE_VERTICAL_OFFSET        m_ImageConfiguration.active_vertical_offset
#define _ACTIVE_HORIZONTAL_OFFSET      m_ImageConfiguration.active_horizontal_offset

#define _IMAGE_HEIGHT                           m_ImageConfiguration.image_height
#define _IMAGE_WIDTH                            m_ImageConfiguration.image_width

#define appendLogFile(text) if(mLogger) mLogger->Append(text)

PipelineDefectCorrector* PipelineDefectCorrector::m_instance = NULL;

PipelineDefectCorrector::PipelineDefectCorrector()
{
    mLogger = NULL;
}

#define INSTANCE(instance) PipelineDefectCorrector* instance = PipelineDefectCorrector::getInstance()

void PipelineDefectCorrector::SetImageConfiguration(
       ImageConfiguration& imageConfiguration)
{
    INSTANCE(instance);
    instance->m_ImageConfiguration = imageConfiguration;
}

void PipelineDefectCorrector::SetLogger(Logger* logger)
{
    INSTANCE(instance);
    instance->mLogger = logger;
}

bool PipelineDefectCorrector::Correct(int16* rawData, const ImageSize &size, const std::vector<Defect>* defectPixels)
{
    INSTANCE(instance);
    return instance->_Correct(rawData, size, defectPixels);
}

bool PipelineDefectCorrector::_Correct(
        int16* rawData,
        const ImageSize& size,
        const std::vector<Defect>* defectPixels)
{
    mSize.Set(size);
    mSensorDefects_pixels = defectPixels;

    appendLogFile("Apply Defects Correction");
    //Check list of defective pixels and correct value with neighbourhood
    //To be Done: Need to think about cluster of several pixels

    // get pointer to defectpixels
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif

    int minX = size.offsetX;
    int maxX = size.offsetX + size.width;
    int minY = size.offsetY;
    int maxY = size.offsetY + size.height;

    for(int i=0; i < (int)mSensorDefects_pixels->size(); i++)
    {
        // int iPosition = mSensorDefects->pixels[i].x + (mSensorDefects->pixels[i].y * size.width);
        // debugPrint("Before correction : " + rawData[iPosition]);

        Defect defectPixel = mSensorDefects_pixels->at(i);

        // check whether pixel is in the rawData

        if((defectPixel.coord.x >= minX) && (defectPixel.coord.x < maxX) &&
           (defectPixel.coord.y >= minY) && (defectPixel.coord.y < maxY))
        {
            // change defect coord to match rawData
            defectPixel.coord.x -= size.offsetX;
            defectPixel.coord.y -= size.offsetY;

            int* crashPointer = 0;
            if(defectPixel.coord.x < 0)
            {
                *crashPointer = 0;
            }
            if(defectPixel.coord.y < 0)
            {
                *crashPointer = 0;
            }

            correctDefectivePixel(rawData, defectPixel.coord);

            // debugPrint("After correction : " + rawData[iPosition]);
        }
    }

    return true;
}

void PipelineDefectCorrector::correctDefectivePixel(int16* rawData, Point pixel)
{
    mRawData = rawData;

    int32 iSum = 0.0;
    int   nbGoodValues = 0;

    int16 iNeighborPixelValue[NEIGHBOR_ARRAY_SIZE(NEIGHBOR_DISTANCE)] = {0};
    bool  bNeighborPixelValid[NEIGHBOR_ARRAY_SIZE(NEIGHBOR_DISTANCE)] = {false};

    // Get value and validity of each neighboor pixels
    getNeighborPixels(NEIGHBOR_DISTANCE, iNeighborPixelValue, bNeighborPixelValid, pixel);

    // [0][1][2]
    // [3][4][5]
    // [6][7][8]
    //
    // false = invalid
    // true  = valid
    //
    // +---------------------------+
    // |                           |
    // |   +---------A---------+   |
    // |   |                   |   |
    // |   |                   |   |
    // |   |                   |   |
    // |   B                   C   |
    // |   |                   |   |
    // |   |                   |   |
    // |   |                   |   |
    // +---+-------------------+---+

    /*
    debugPrint("x=" + x + ";y=" + y);
    debugPrint("Initial value");
    debugPrint("---------");
    debugPrint("[" + iNeighboorPixelValue[0] + "][" + iNeighboorPixelValue[1] + "][" + iNeighboorPixelValue[2] + "]");
    debugPrint("[" + iNeighboorPixelValue[3] + "][" + iNeighboorPixelValue[4] + "][" + iNeighboorPixelValue[5] + "]");
    debugPrint("[" + iNeighboorPixelValue[6] + "][" + iNeighboorPixelValue[7] + "][" + iNeighboorPixelValue[8] + "]");
    debugPrint("---------");
    debugPrint("Before");
    debugPrint("[" + bNeighboorPixelValid[0] ? "o" : "#" + "][" + bNeighboorPixelValid[1] ? "o" : "#" + "][" + bNeighboorPixelValid[2] ? "o" : "#" + "]");
    debugPrint("[" + bNeighboorPixelValid[3] ? "o" : "#" + "][" + bNeighboorPixelValid[4] ? "o" : "#" + "][" + bNeighboorPixelValid[5] ? "o" : "#" + "]");
    debugPrint("[" + bNeighboorPixelValid[6] ? "o" : "#" + "][" + bNeighboorPixelValid[7] ? "o" : "#" + "][" + bNeighboorPixelValid[8] ? "o" : "#" + "]");
    debugPrint("---------");
    */

#ifdef DEFECT_ON_ACTIVE_AREA
    // desactive some pixels depending on the location of the pixel
    if((pixel.y == _ACTIVE_VERTICAL_OFFSET) &&
       (pixel.x >= _ACTIVE_HORIZONTAL_OFFSET) &&
       (pixel.x < _ACTIVE_HORIZONTAL_OFFSET + _ACTIVE_WIDTH))
    {
        // A segment
        bNeighborPixelValid[0] = false;
        bNeighborPixelValid[1] = false;
        bNeighborPixelValid[2] = false;
    }

    if((pixel.x == _ACTIVE_HORIZONTAL_OFFSET) &&
       (pixel.y >= _ACTIVE_VERTICAL_OFFSET))
    {
        // B segment
        bNeighborPixelValid[0] = false;
        bNeighborPixelValid[3] = false;
        bNeighborPixelValid[6] = false;
    }
    else if((pixel.x == _ACTIVE_HORIZONTAL_OFFSET + _ACTIVE_WIDTH - 1) &&
            (pixel.y >= _ACTIVE_VERTICAL_OFFSET))
    {
        // C segment
        bNeighborPixelValid[2] = false;
        bNeighborPixelValid[5] = false;
        bNeighborPixelValid[8] = false;
    }
#else
    // desactive some pixels depending on the location of the pixel
    if((pixel.y == 0) &&
       (pixel.x >= 0) &&
       (pixel.x < _IMAGE_WIDTH))
    {
        // A segment
        bNeighborPixelValid[0] = false;
        bNeighborPixelValid[1] = false;
        bNeighborPixelValid[2] = false;
    }

    if((pixel.x == 0) &&
       (pixel.y >= 0))
    {
        // B segment
        bNeighborPixelValid[0] = false;
        bNeighborPixelValid[3] = false;
        bNeighborPixelValid[6] = false;
    }
    else if((pixel.x == _IMAGE_WIDTH - 1) &&
            (pixel.y >= 0))
    {
        // C segment
        bNeighborPixelValid[2] = false;
        bNeighborPixelValid[5] = false;
        bNeighborPixelValid[8] = false;
    }
#endif

    /*
    debugPrint("---------");
    debugPrint("After");
    debugPrint("[" + bNeighboorPixelValid[0] ? "o" : "#" + "][" + bNeighboorPixelValid[1] ? "o" : "#" + "][" + bNeighboorPixelValid[2] ? "o" : "#" + "]");
    debugPrint("[" + bNeighboorPixelValid[3] ? "o" : "#" + "][" + bNeighboorPixelValid[4] ? "o" : "#" + "][" + bNeighboorPixelValid[5] ? "o" : "#" + "]");
    debugPrint("[" + bNeighboorPixelValid[6] ? "o" : "#" + "][" + bNeighboorPixelValid[7] ? "o" : "#" + "][" + bNeighboorPixelValid[8] ? "o" : "#" + "]");
    debugPrint("---------");
    */

    // For each neighbor pixels
    for(int i = 0; i < NEIGHBOR_ARRAY_SIZE(NEIGHBOR_DISTANCE); i++)
    {
        // If this pixel is good
        if (bNeighborPixelValid[i] == true)
        {
            iSum += iNeighborPixelValue[i];
            nbGoodValues++;
        }
    }

    // To avoid division by zero
    if (nbGoodValues > 0)
    {
        // Average of good values
        int iPosition = (int)pixel.y * mSize.width + (int)pixel.x;

        mRawData[iPosition] = (int16)(iSum / nbGoodValues);

        /*
        debugPrint("Sum :" + iSum  + "| Good values :" + nbGoodValues);
        debugPrint("Final result");
        debugPrint("---------");
        debugPrint("[" + iNeighboorPixelValue[0] + "][" + iNeighboorPixelValue[1] + "][" + iNeighboorPixelValue[2] + "]");
        debugPrint("[" + iNeighboorPixelValue[3] + "][" + iNeighboorPixelValue[4] + "][" + iNeighboorPixelValue[5] + "]");
        debugPrint("[" + iNeighboorPixelValue[6] + "][" + iNeighboorPixelValue[7] + "][" + iNeighboorPixelValue[8] + "]");
        debugPrint("---------");
        */
    }
}

bool PipelineDefectCorrector::getNeighborPixels(
        int    neighborDistance,
        int16* iPixelValue,
        bool*  bPixelValid,
        Point  pixel)
{
    // Set all pixels to zero and invalid
    std::memset(iPixelValue, 0, NEIGHBOR_ARRAY_SIZE(neighborDistance) * sizeof(int16));
    std::memset(bPixelValid, 0, NEIGHBOR_ARRAY_SIZE(neighborDistance) * sizeof(bool));

    int xMin = pixel.x - neighborDistance;
    int xMax = pixel.x + neighborDistance;

    int yMin = pixel.y - neighborDistance;
    int yMax = pixel.y + neighborDistance;

    int index = 0;

    for(int yIndex = yMin; yIndex <= yMax; yIndex ++)
    {
        for(int xIndex = xMin; xIndex <= xMax; xIndex ++)
        {
            bPixelValid[index] = getPixelValue(xIndex, yIndex, &iPixelValue[index]);
            index ++;
        }
    }

    return true;
}

bool PipelineDefectCorrector::getPixelValue(short x, short y, int16* value)
{
    bool res = true;

    // If this pixel does not exist
    if((x < 0) || (x >= _IMAGE_WIDTH) ||
       (y < 0) || (y >= _IMAGE_HEIGHT))
    {
        *value = 0;
        res = false;
    }
    else
    {
        // Get the value of this pixel
        int iPosition = (int)y * _IMAGE_WIDTH + (int)x;

        *value = mRawData[iPosition];

        res = !this->isDefective(x, y);
    }

    return res;
}

bool PipelineDefectCorrector::isDefective(short x, short y)
{
    Defect defectPixel;

    // For each defective pixel
    for(int i = 0; i < (int)mSensorDefects_pixels->size(); i++)
    {
        defectPixel = mSensorDefects_pixels->at(i);

        // If it is the requested pixel
        if((defectPixel.coord.x == x) &&
           (defectPixel.coord.y == y))
        {
            return true;
        }
    }

    return false;
}
