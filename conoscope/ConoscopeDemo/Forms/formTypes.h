#ifndef FORMTYPES_H
#define FORMTYPES_H

#include "conoscopeTypes.h"

typedef struct
{
    SetupConfig_t         cmdSetupConfig;
    MeasureConfig_t       cmdMeasureConfig;
    ProcessingConfig_t    cmdProcessingConfig;

    ConoscopeSettings_t      config;
    ConoscopeDebugSettings_t debugConfig;

    CaptureSequenceConfig_t cmdCaptureSequenceConfig;

    ConvertRaw_t cmdConvertRaw;

    struct ApplicationSettings_t
    {
        bool    bStreamProcessedData;
        bool    autoExposure;         // enable auto exposure (or not)
    } applicationConfig;

    struct ApplicationState_t
    {
        bool cfgPathEnable;
        bool emulatedCameraEnable;
    } applicationState;

} CmdParameters_t;

#endif // FORMTYPES_H
