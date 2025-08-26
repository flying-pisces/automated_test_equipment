#ifndef PIPELINELIB_H
#define PIPELINELIB_H

#include "PipelineLib_global.h"
#include <string>
#include "Types.h"

#include "PipelineTypes.h"

extern "C" PIPELINELIBSHARED_EXPORT const char* CmdGetVersion();
extern "C" PIPELINELIBSHARED_EXPORT const char *CmdComputeRawData(int16 *inputData, Pipeline_RawDataParam* param, Pipeline_ResultRawDataParam &resultParam);
extern "C" PIPELINELIBSHARED_EXPORT const char *CmdComputeKLibData(int16* inputData, Pipeline_KLibDataParam &param, Pipeline_CalibrationParam *calibration, int16 *klibData);

#endif // PIPELINELIB_H
