#ifndef FORMTYPES_H
#define FORMTYPES_H

#include "conoscopeTypes.h"

typedef struct
{
    SetupConfig_t       cmdSetupConfig;
    MeasureConfig_t     cmdMeasureConfig;
    ProcessingConfig_t  cmdProcessingConfig;
    ConoscopeSettings_t cmdSetConfig;

#ifdef ENABLE_DEBUG_INTERFACE
    DebugMode_t debugModeConfig;
#endif
} CmdParameters_t;

#endif // FORMTYPES_H
