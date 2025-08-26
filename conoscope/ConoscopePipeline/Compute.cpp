#include "Compute.h"

#include "PipelineCompute.h"

#define INSTANCE(instance) Compute* instance = getInstance()

Compute* Compute::m_instance = NULL;

Compute::Compute()
{
    // create a logger
    mLogger = new Logger("LogPipeline.txt");

    // and assocaite it
    PipelineCompute::SetLogger(mLogger);

    // define image dimension
    mImageConfiguration.image_horizontal_offset      = 0;
    mImageConfiguration.image_vertical_offset        = 0;
    mImageConfiguration.image_width                  = 7920;
    mImageConfiguration.image_height                 = 6004;

    mImageConfiguration.active_horizontal_offset     = 0;
    mImageConfiguration.active_vertical_offset       = 0;
    mImageConfiguration.active_width                 = 7920;
    mImageConfiguration.active_height                = 6004;

    mImageConfiguration.bias_corner_area_width       = 32;
    mImageConfiguration.bias_corner_area_height      = 72;

    mImageConfiguration.active_central_area_horizontal_offset = 492;
    mImageConfiguration.active_central_area_vertical_offset   = 0;

    mImageConfiguration.UpdateSettings();

    PipelineCompute::SetImageConfiguration(mImageConfiguration);
}

Compute::~Compute()
{
    // it is for development purpose
    delete mLogger;
}

Compute* Compute::getInstance()
{
    if(m_instance == NULL)
    {
        m_instance = new Compute();
    }

    return m_instance;
}

Error_t Compute::ComputeRawData(
        int16* inputData,
        Pipeline_RawDataParam* param,
        Pipeline_ResultRawDataParam& resultParam)
{
    INSTANCE(instance);
    return instance->_ComputeRawData(
                inputData,
                param,
                resultParam);
}

Error_t Compute::ComputeKLibData(
        int16* inputData,
        Pipeline_KLibDataParam &param,
        Pipeline_CalibrationParam *calibration,
        int16* klibData)
{
    INSTANCE(instance);
    return instance->_ComputeKLibData(
                inputData,
                param,
                calibration,
                klibData);
}

Error_t Compute::_ComputeRawData(
        int16* inputData,
        Pipeline_RawDataParam *param,
        Pipeline_ResultRawDataParam& resultParam)
{
    Error_t res = Error_Ok;

    // TODO remove this line
    int16* rawData = inputData;
    Histogram* pHistogram = NULL;

    res = PipelineCompute::ComputeRawData(
                rawData,
                param,
                pHistogram,
                resultParam);

    return res;
}

Error_t Compute::_ComputeKLibData(
        int16*                     inputData,
        Pipeline_KLibDataParam     &param,
        Pipeline_CalibrationParam  *calibration,
        int16*                     klibData)
{
    Error_t res = Error_Ok;

    Pipeline_DataIn   rawData;
    Pipeline_DataOut  calibDataV2Out;
    std::vector<char> calibratedData;

    rawData.pData = inputData;
    rawData.dataSize = param.imageSize.nbPixels * sizeof(int16);

    // Set size of output buffer
    int calibratedDataSize = (2 * calibration->calibratedDataRadius + 1) * (2 * calibration->calibratedDataRadius + 1);
    calibratedData.resize(calibratedDataSize * sizeof(int16));

    res = PipelineCompute::ComputeKLibData(
                param,
                calibration,
                &rawData,
                &calibDataV2Out,
                // pCalibratedData);
                klibData);

    return res;
}
