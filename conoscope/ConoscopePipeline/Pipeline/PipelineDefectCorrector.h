#ifndef STORMHOLDDEFECTCORRECTOR_H
#define STORMHOLDDEFECTCORRECTOR_H

#include "logger.h"
#include "imageConfiguration.h"

#include "PipelineTypes.h"
#include "PipelineHistogram.h"

class PipelineDefectCorrector
{
private:
    static PipelineDefectCorrector* m_instance;

    PipelineDefectCorrector();

    static PipelineDefectCorrector* PipelineDefectCorrector::getInstance()
    {
        if(m_instance == NULL)
        {
            m_instance = new PipelineDefectCorrector();
        }

        return m_instance;
    }

    ImageConfiguration m_ImageConfiguration;

public:
    static void SetImageConfiguration(ImageConfiguration &imageConfiguration);

    static void SetLogger(Logger* logger);

    static bool Correct(int16* rawData, const ImageSize &size, const std::vector<Defect> *defectPixels);

// protected:
private:
    Logger*             mLogger;
    int16*              mRawData;
    ImageSize           mSize;
    const std::vector<Defect>*   mSensorDefects_pixels;

    bool _Correct(
            int16* rawData,
            const ImageSize &size,
            const std::vector<Defect> *defectPixels);

    void correctDefectivePixel(int16 *rawData, Point pixel);

    bool getNeighborPixels(
            int neighborDistance,
            int16 iNeighboorPixelValue[9],
            bool bNeighboorPixelValid[9],
            Point pixel);

    bool getPixelValue(
            short x,
            short y,
            int16* value);

    bool isDefective(short x, short y);
};

#endif // STORMHOLDDEFECTCORRECTOR_H
