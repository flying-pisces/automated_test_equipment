#ifndef CONOSCOPE_H
#define CONOSCOPE_H

#include <string>

#include "camera.h"
#include "ConoscopeWorker.h"
#include "conoscopeTypes.h"
#include "ConoscopeConfig.h"
#include "ConoscopeStaticTypes.h"

#include "logger.h"

class Conoscope : public ClassThreadCommon
{
    Q_OBJECT

public:
    enum class Event {
        CmdOpen,
        CmdSetup,
        CmdMeasure,
        CmdExportRaw,
        CmdExportProcessed,
        CmdClose,
        CmdReset,

        CmdCfgFileWrite,
        CmdCfgFileRead,
        CmdCfgFileStatus,

        Error,
    };
    Q_ENUM(Event)

    enum class State {
        Undefined,  /*< worker is not started */
        Idle,       /*< module is started */

        Opened,
        Ready,
        CaptureDone,

        CmdOpenProcessing,
        CmdSetupProcessing,
        CmdMeasureProcessing,
        CmdExportRawProcessing,
        CmdExportProcessedProcessing,
        CmdCloseProcessing,
        CmdResetProcessing,

        CmdCfgFileWriteProcessing,
        CmdCfgFileReadProcessing,

        Error,
    };
    Q_ENUM(State)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = Conoscope::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    Conoscope(ConoscopeConfig *pConfig, QObject *parent = nullptr);

    ~Conoscope();

    void Start();

    ClassCommon::Error Stop();

    void StopThread();

    QString CmdGetPipelineVersion();

    typedef struct
    {
        QString cfgPath;
        QString cameraSn;
        QString cameraVersion;
    } CmdOpenOutput_t;

    ClassCommon::Error CmdOpen(CmdOpenOutput_t& output);
    ClassCommon::Error CmdSetup(SetupConfig_t &config);
    ClassCommon::Error CmdSetupStatus(SetupStatus_t &status);
    ClassCommon::Error CmdMeasure(MeasureConfigWithCropFactor_t &config);

    typedef struct
    {
        QString fileName;

        float        sensorTemperature; // setup: temperature target
        Filter_t     eFilter;           // setup: filter
        Nd_t         eNd;               // setup: nd filter
        IrisIndex_t  eIris;             // setup: iris

        unsigned int exposureTimeUs;    // measurement info:
        unsigned int nbAcquisition;     // measurement info:
        unsigned int height;            // measurement info:
        unsigned int width;             // measurement info:

        int min;
        int max;

#ifdef SATURATION_FLAG_RAW
        bool  saturationFlag;
        float saturationLevel;
#endif
    } CmdExportRawOutput_t;

    typedef struct
    {
        bool bAeEnable;
        int  AEMeasAreaHeight;
        int  AEMeasAreaWidth;
        int  AEMeasAreaX;
        int  AEMeasAreaY;
    } CmdExportAdditionalInfo_t;

    ClassCommon::Error CmdExportRaw(CmdExportRawOutput_t& output);
    ClassCommon::Error CmdExportRaw(std::vector<uint16_t> &buffer, CmdExportRawOutput_t& output, CmdExportAdditionalInfo_t &additionalInfo);

    typedef struct
    {
        QString fileName;

        float        sensorTemperature; // setup: temperature target
        Filter_t     eFilter;           // setup: filter
        Nd_t         eNd;               // setup: nd filter
        IrisIndex_t  eIris;             // setup: iris

        unsigned int exposureTimeUs;    // measurement info:
        unsigned int nbAcquisition;     // measurement info:
        unsigned int height;            // measurement info:
        unsigned int width;             // measurement info:

        ParamData<QString> cameraCfgFileName;         // cfg file: camera
        ParamData<QString> opticalColumnCfgFileName;  // cfg file: optical column
        ParamData<QString> flatFieldFileName;         // file: flat field binary

        ParamData<float>   colorCoefCompX;    // processed: read from configuration
        ParamData<float>   colorCoefCompY;    // processed: read from configuration
        ParamData<float>   colorCoefCompZ;    // processed: read from configuration

        double conversionFactorCompX;         // processed: conversion factor
        double conversionFactorCompY;         // processed: conversion factor
        double conversionFactorCompZ;         // processed: conversion factor

        int min;
        int max;

        bool  saturationFlag;
        float saturationLevel;
    } CmdExportProcessedOutput_t;

    ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config, CmdExportProcessedOutput_t& output);
    ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, CmdExportProcessedOutput_t& output, bool bSaveImage);

    ClassCommon::Error CmdClose();
    ClassCommon::Error CmdReset(QString &cfgPath);

    ClassCommon::Error CmdSetConfig(ConoscopeSettings_t &config);
    ClassCommon::Error CmdGetConfig(ConoscopeSettings_t &config);

    ClassCommon::Error CmdGetCmdConfig(SetupConfig_t &setupConfig,
                                       MeasureConfig_t &measureConfig,
                                       ProcessingConfig_t &processingConfig);

    ClassCommon::Error CmdSetDebugConfig(ConoscopeDebugSettings_t &conoscopeConfig);
    ClassCommon::Error CmdGetDebugConfig(ConoscopeDebugSettings_t &conoscopeConfig);

    ClassCommon::Error CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig);

    ClassCommon::Error CmdGetCaptureSequenceConfig(CaptureSequenceConfig_t& captureSequenceConfig);
    ClassCommon::Error CmdSetCaptureSequenceConfig(CaptureSequenceConfig_t& captureSequenceConfig);

    ClassCommon::Error CmdCfgFileWrite();
    ClassCommon::Error CmdCfgFileRead();
    ClassCommon::Error CmdCfgFileStatus(CfgFileStatus_t &status);

    ClassCommon::Error CmdConvertRaw(ConvertRaw_t &param);

    void GetSomeInfo(SomeInfo_t &info);

private:
    bool SendRequest(ConoscopeWorker::Request event);

    void _SetState(State eState);

    ClassCommon::Error ChangeState(State eState);
    ClassCommon::Error ChangeState(State eState, void* parameter);

    ClassCommon::Error ProcessStateMachine(Event eEvent);
    ClassCommon::Error ProcessStateMachine(Event eEvent, void* parameter);

    State mState;
    State mStatePrevious;

    ConoscopeWorker* mWorker;

    ConoscopeConfig* mConfig;

    ConoscopeBehavior_t mBehaviorConfig;

    void on_conoscopeProcess_Log(QString message);

    Logger* mLogger;

public slots:
    void on_worker_jobDone(int value, int error);

protected:
    void run();

public:
#define SET_CONFIG_IMPLEMENTATION {mConfig->SetConfig(config);}
#define GET_CONFIG_IMPLEMENTATION {mConfig->GetConfig(config);}

    void GetConfig(ConoscopeDebugSettings_t& config)GET_CONFIG_IMPLEMENTATION

    void GetConfig(ConoscopeSettingsI_t& config)GET_CONFIG_IMPLEMENTATION

};

#endif // CONOSCOPE_H
