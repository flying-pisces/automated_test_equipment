#ifndef APPCONTROLLERRESOURCE_H
#define APPCONTROLLERRESOURCE_H

// #include "conoscopeApi.h"
#include "conoscopeLib.h"
#include "conoscopeTypes.h"

#include "appConfig.h"

#define RESOURCES AppResource::Instance()

class AppResource
{

private:
    AppConfig* mConfig;

    AppResource()
    {
        mConfig = new AppConfig();

        mConfig->GetConfig(mAppConfig);

        mTemperatureMonitoringStateMap[(int)TemperatureMonitoringState_NotStarted] = "NotStarted";
        mTemperatureMonitoringStateMap[(int)TemperatureMonitoringState_Processing] = "Processing";
        mTemperatureMonitoringStateMap[(int)TemperatureMonitoringState_Locked] = "Locked";
        mTemperatureMonitoringStateMap[(int)TemperatureMonitoringState_Aborted] = "Aborted";
        mTemperatureMonitoringStateMap[(int)TemperatureMonitoringState_Error] = "Error";

        mWheelStateToStringMap[(int)WheelState_Idle] = "Idle";
        mWheelStateToStringMap[(int)WheelState_Success] = "Success";
        mWheelStateToStringMap[(int)WheelState_Operating] = "Operating";
        mWheelStateToStringMap[(int)WheelState_Error] = "Error";

        mFilterToStringMap[(int)Filter_BK7] = "BK7";
        mFilterToStringMap[(int)Filter_Mirror] = "Mirror";
        mFilterToStringMap[(int)Filter_X]  = "X";
        mFilterToStringMap[(int)Filter_Xz] = "Xz";
        mFilterToStringMap[(int)Filter_Ya] = "Ya";
        mFilterToStringMap[(int)Filter_Yb] = "Yb";
        mFilterToStringMap[(int)Filter_Z]  = "Z";
        mFilterToStringMap[(int)Filter_IrCut] = "IRCut";
        mFilterToStringMap[(int)Filter_Invalid] = "Invalid";

        mNdToStringMap[(int)Nd_0] = "0";
        mNdToStringMap[(int)Nd_1] = "1";
        mNdToStringMap[(int)Nd_2] = "2";
        mNdToStringMap[(int)Nd_3] = "3";
        mNdToStringMap[(int)Nd_4] = "4";
        mNdToStringMap[(int)Nd_Invalid] = "Invalid";

        mIrisIndexToStringMap[(int)IrisIndex_2mm] = "2";
        mIrisIndexToStringMap[(int)IrisIndex_3mm] = "3";
        mIrisIndexToStringMap[(int)IrisIndex_4mm] = "4";
        mIrisIndexToStringMap[(int)IrisIndex_5mm] = "5";
        mIrisIndexToStringMap[(int)IrisIndex_Invalid] = "Invalid";

        mCfgFileStateMap[(int)CfgFileState_NotDone] = "NotDone";
        mCfgFileStateMap[(int)CfgFileState_Reading] = "Reading";
        mCfgFileStateMap[(int)CfgFileState_Writing] = "Writing";
        mCfgFileStateMap[(int)CfgFileState_ReadDone] = "ReadDone";
        mCfgFileStateMap[(int)CfgFileState_WriteDone] = "WriteDone";
        mCfgFileStateMap[(int)CfgFileState_ReadError] = "ReadError";
        mCfgFileStateMap[(int)CfgFileState_WriteError] = "WriteError";

        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_NotStarted]  = "NotStarter";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_Setup]       = "Setup";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_WaitForTemp] = "WaitForTemp";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_AutoExpo]    = "AutoExpo";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_Measure]     = "Measure";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_Process]     = "Process";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_Done]        = "Done";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_Error]       = "Error";
        mCaptureSequenceToStringMap[(int)CaptureSequenceStatus_t::State_t::State_Cancel]      = "Cancel";

        mMeasureStateToStringMap[(int) MeasureStatus_t::State_NotStarted] = "NotStarted";
        mMeasureStateToStringMap[(int) MeasureStatus_t::State_Process]    = "Process   ";
        mMeasureStateToStringMap[(int) MeasureStatus_t::State_Done]       = "Done      ";
        mMeasureStateToStringMap[(int) MeasureStatus_t::State_Error]      = "Error     ";
        mMeasureStateToStringMap[(int) MeasureStatus_t::State_Cancel]     = "Cancel    ";

        mExportFormatToStringMap[(int) ExportFormat_t::ExportFormat_bin]  = "bin";
        mExportFormatToStringMap[(int) ExportFormat_t::ExportFormat_bin_jpg]  = "bin+jpg";
        //mExportFormatToStringMap[(int) ExportFormat_t::ExportFormat_jpg]  = "jpg";
    }

    ~AppResource() {}

    static AppResource* mInstance;

    static AppResource* GetInstance();

public:
    static AppResource* Instance();

    // ConoscopeApi* mConoscopeApi;
    ConoscopeLib* mConoscopeApi;

    std::map<int, QString> mTemperatureMonitoringStateMap; // TemperatureMonitoringState_t
    std::map<int, QString> mWheelStateToStringMap;         // WheelState_t
    std::map<int, QString> mFilterToStringMap;             // Filter_t
    std::map<int, QString> mNdToStringMap;                 // Nd_t
    std::map<int, QString> mIrisIndexToStringMap;          // IrisIndex_t
    std::map<int, QString> mCfgFileStateMap;               // CfgFileState_t
    std::map<int, QString> mCaptureSequenceToStringMap;    // CaptureSequenceStatus_t::State_t
    std::map<int, QString> mMeasureStateToStringMap;       // MeasureStatus_t::State_t
    std::map<int, QString> mExportFormatToStringMap;               // ExportFormat_t;

    AppConfig_t mAppConfig;

    void SaveAppConfig()
    {
        mConfig->SetConfig(mAppConfig);
    }

    bool IsMaskEnable(LogMask_t mask);

    void GetDisplayStreamConfig(ProcessingConfig_t& config);

    void Close();
};

#endif // APPCONTROLLERRESOURCE_H
