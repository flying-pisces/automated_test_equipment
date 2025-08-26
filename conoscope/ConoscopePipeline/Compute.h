#ifndef COMPUTE_H
#define COMPUTE_H

#include "logger.h"
#include "imageConfiguration.h"
#include "PipelineTypes.h"

#include "PipelineDefines.h"

class Compute
{

private:
    static Compute* m_instance;
    static Compute* getInstance();

    Compute();

    ~Compute();

    ImageConfiguration mImageConfiguration;
    Logger* mLogger;

    QByteArray mImgData;

public:
    static Error_t ComputeRawData(
            int16 *inputData,
            Pipeline_RawDataParam *param,
            Pipeline_ResultRawDataParam &resultParam);

    static Error_t ComputeKLibData(int16 *inputData,
            Pipeline_KLibDataParam &param,
            Pipeline_CalibrationParam *calibration,
            int16 *klibData);

private:
    Error_t _ComputeRawData(
            int16 *inputData,
            Pipeline_RawDataParam *param,
            Pipeline_ResultRawDataParam &resultParam);

    Error_t _ComputeKLibData(int16 *inputData,
            Pipeline_KLibDataParam &param,
            Pipeline_CalibrationParam *calibration,
            int16 *klibData);

};

#endif // COMPUTE_H
