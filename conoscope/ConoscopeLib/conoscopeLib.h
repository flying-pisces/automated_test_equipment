#ifndef CONOSCOPELIB_H
#define CONOSCOPELIB_H

#include "conoscopeLib_global.h"
#include <string>
#include <vector>

#include "conoscopeTypes.h"

extern "C" CONOSCOPELIBSHARED_EXPORT const char* CmdRunApp();
extern "C" CONOSCOPELIBSHARED_EXPORT const char* CmdQuitApp();

// retrieve version of library (and version of pipeline library)
extern "C" CONOSCOPELIBSHARED_EXPORT const char* CmdGetVersion();

extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdOpen();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdSetup(SetupConfig_t& config);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdSetupStatus(SetupStatus_t& status);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdMeasure(MeasureConfig_t& config);

extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdExportRaw();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdExportRawBuffer(std::vector<uint16_t>& buffer);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdExportProcessed(ProcessingConfig_t& config);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdExportProcessedBuffer(ProcessingConfig_t& config, std::vector<int16_t> &buffer);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdClose();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdReset();

// set some configuration of the lib
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdSetConfig(ConoscopeSettings2_t &config);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdGetConfig(ConoscopeSettings2_t &config);

// retrieve last configuration of CmdSetup, CmdMeasure, CmdExportProcess
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdGetCmdConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig);

// retrieve debug configuration (debug mode may alter state machine)
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdSetDebugConfig(ConoscopeDebugSettings2_t &conoscopeConfig);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdGetDebugConfig(ConoscopeDebugSettings2_t &conoscopeConfig);

extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdSetBehaviorConfig(ConoscopeBehavior_t &behaviorConfig);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdRegisterLogCallback(void (*callback)(char*));
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdRegisterEventCallback(void (*callback)(ConoscopeEvent_t, QString));
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdRegisterWarningCallback(void (*callback)(QString));

// write/read data in camera memory (parameter set with CmdSetConfig)
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdCfgFileWrite();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdCfgFileRead();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdCfgFileStatus(CfgFileStatus_t& status);

// higher level commands
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdGetCaptureSequence(CaptureSequenceConfig_t& config);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdCaptureSequence(CaptureSequenceConfig_t& config);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdCaptureSequenceCancel();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdCaptureSequenceStatus(CaptureSequenceStatus_t &status);

extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdMeasureAE(MeasureConfig_t& config);
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdMeasureAECancel();
extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdMeasureAEStatus(MeasureStatus_t& status);

extern "C" CONOSCOPELIBSHARED_EXPORT const char *CmdConvertRaw(ConvertRaw_t& param);

// terminate the dll
extern "C" CONOSCOPELIBSHARED_EXPORT const char* CmdTerminate();

#endif // CONOSCOPELIB_H
