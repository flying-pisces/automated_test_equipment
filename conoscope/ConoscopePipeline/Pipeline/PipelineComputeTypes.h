#ifndef PIPELINECOMPUTETYPES_H
#define PIPELINECOMPUTETYPES_H

#define COMPUTE_PARAM

#include <stdlib.h>
#include "PipelineHistogram.h"
#include "PipelineDefines.h"

typedef struct {
    float linearizationTableX;
    float linearizationTableY;
    FLAT_FIELD_INVERSE_TYPE flatFieldInverse;
    bool linearizationRadius;
} precomputedData_t;

#endif PIPELINECOMPUTETYPES_H
