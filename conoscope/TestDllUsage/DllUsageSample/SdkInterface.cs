



using System;

using System.Security;
using System.Runtime.InteropServices; // DllImport

static class SdkInterface
{
    const string DLLPATH = @"ConoscopeLib.dll";
    const CallingConvention CALLCONV = CallingConvention.StdCall;

    public enum Filter_t
    {
        Filter_BK7,
        Filter_Mirror,
        Filter_X,
        Filter_Xz,
        Filter_Ya,
        Filter_Yb,
        Filter_Z,
        Filter_IrCut,
        Filter_Invalid,
    }

    public enum Nd_t
    {
        Nd_0,
        Nd_1,
        Nd_2,
        Nd_3,
        Nd_4,
        Nd_Invalid,
    }

    public enum Iris_t
    {
        IrisIndex_2mm,
        IrisIndex_3mm,
        IrisIndex_4mm,
        IrisIndex_5mm,
        IrisIndex_Invalid,
    }
    
    public enum TemperatureMonitoringState_t
    {
        TemperatureMonitoringState_NotStarted,
        TemperatureMonitoringState_Processing,
        TemperatureMonitoringState_Locked,
        TemperatureMonitoringState_Aborted,
        TemperatureMonitoringState_Error,
    }

    public enum WheelState_t
    {
        WheelState_Idle,
        WheelState_Success,
        WheelState_Operating,
        WheelState_Error,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SetupConfig_t
    {
        public float sensorTemperature;   // temperature target
        public SdkInterface.Filter_t filterWheelPosition;   // position of the filter wheel
        public SdkInterface.Nd_t ndWheelPosition;           // position of the nd wheel
        public SdkInterface.Iris_t irisIndex;               // selected iris (this is not a command, this parameter indicates the iris installed on the device)
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SetupStatus_t
    {
        public SdkInterface.TemperatureMonitoringState_t temperatureMonitoringState;
        public float sensorTemperature;     // current temperature target

        public SdkInterface.WheelState_t wheelState;
        public SdkInterface.Filter_t filterWheelPosition;   // position of the filter wheel
        public SdkInterface.Nd_t ndWheelPosition;           // position of the NB wheel
        public SdkInterface.Iris_t irisIndex;               // selected iris (this is not a command, this parameter indicates the iris installed on the device)
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct MeasureConfig_t
    {
        public int exposureTimeUs;        // exposure time in micro seconds
        public int nbAcquisition;         // number of frames acquired (average)
        public int binningFactor;         //
        public bool bTestPattern;          // if set, return a test pattern returned by the sensor
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ProcessingConfig_t
    {
        public bool bBiasCompensation;
        public bool bSensorDefectCorrection;
        public bool bSensorPrnuCorrection;
        public bool bLinearisation;
        public bool bFlatField;
        public bool bAbsolute;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct _ConoscopeSettings_t
    {
        public IntPtr cfgPath;              // location where cfg files are stored
        public IntPtr capturePath;          // location where captures are stored
        public bool autoExposure;         // enable auto exposure (or not)
        public float autoExposurePixelMax; // indicate the max value of a pixel in autoexposure algo
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ConoscopeSettings_t
    {
        public string cfgPath;              // location where cfg files are stored
        public string capturePath;          // location where captures are stored
        public bool autoExposure;         // enable auto exposure (or not)
        public float autoExposurePixelMax; // indicate the max value of a pixel in autoexposure algo
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct _ConoscopeDebugSettings_t
    {
        public bool debugMode;
        public bool emulateCamera;
        public IntPtr dummyRawImagePath; // path of the dymmy image when camera is emulated
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct ConoscopeDebugSettings_t
    {
        public bool debugMode;
        public bool emulateCamera;
        public string dummyRawImagePath; // path of the dymmy image when camera is emulated
    }

    public enum CfgFileState_t
    {
        CfgFileState_NotDone,

        CfgFileState_Reading,
        CfgFileState_Writing,

        CfgFileState_ReadDone,
        CfgFileState_WriteDone,

        CfgFileState_ReadError,
        CfgFileState_WriteError,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CfgFileStatus_t
    {
        public SdkInterface.CfgFileState_t eState;
        public int progress;
        public long elapsedTime;
        public string fileName;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CaptureSequenceConfig_t
    {
        public float sensorTemperature; // temperature target
        public bool bWaitForSensorTemperature;

        public SdkInterface.Nd_t eNd;               // set nd wheel position
        public SdkInterface.Iris_t eIris;

        public int exposureTimeUs;    // exposure time in micro seconds
        public int nbAcquisition;     // number of frames acquired (average)
        public bool bAutoExposure;
        public bool bUseExpoFile;      // exposure time are defined in a json file
    }

    public enum CaptureSequenceState_t
    {
        State_NotStarted,
        State_Setup,
        State_WaitForTemp,
        State_AutoExpo,
        State_Measure,
        State_Process,
        State_Done,
        State_Error,
        State_Cancel,
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CaptureSequenceStatus_t
    {
        public int nbSteps;
        public int currentSteps;

        public Filter_t eFilter;
        public CaptureSequenceState_t eState;
    } 

    [System.Security.SuppressUnmanagedCodeSecurity]
    [DllImport(DLLPATH, EntryPoint = "CmdRunApp", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdRunApp();

    [System.Security.SuppressUnmanagedCodeSecurity]
    [DllImport(DLLPATH, EntryPoint = "CmdQuitApp", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdQuitApp();

    [System.Security.SuppressUnmanagedCodeSecurity]
    [DllImport(DLLPATH, EntryPoint = "CmdGetVersion", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdGetVersion();

    [System.Security.SuppressUnmanagedCodeSecurity]
    [DllImport(DLLPATH, EntryPoint = "CmdOpen", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdOpen();

    [System.Security.SuppressUnmanagedCodeSecurity]
    [DllImport(DLLPATH, EntryPoint = "CmdSetup", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdSetup(ref SetupConfig_t config);

    [System.Security.SuppressUnmanagedCodeSecurity]
    [DllImport(DLLPATH, EntryPoint = "CmdSetupStatus", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdSetupStatus(ref SetupStatus_t setup);

    [DllImport(DLLPATH, EntryPoint = "CmdMeasure", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdMeasure(ref MeasureConfig_t setup);

    [DllImport(DLLPATH, EntryPoint = "CmdExportRaw", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdExportRaw();

    [DllImport(DLLPATH, EntryPoint = "CmdExportProcessed", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdExportProcessed(ref ProcessingConfig_t setup);

    [DllImport(DLLPATH, EntryPoint = "CmdClose", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdClose();

    [DllImport(DLLPATH, EntryPoint = "CmdReset", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdReset();

    [DllImport(DLLPATH, EntryPoint = "CmdSetConfig", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdSetConfig(ref _ConoscopeSettings_t config);

    [DllImport(DLLPATH, EntryPoint = "CmdGetConfig", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdGetConfig(ref _ConoscopeSettings_t config);

    // retrieve last configuration of CmdSetup, CmdMeasure, CmdExportProcess
    [DllImport(DLLPATH, EntryPoint = "CmdGetCmdConfig", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdGetCmdConfig(ref SetupConfig_t setupConfig, ref MeasureConfig_t measureConfig, ref ProcessingConfig_t processingConfig);

    // retrieve debug configuration (debug mode may alter state machine)
    [DllImport(DLLPATH, EntryPoint = "CmdSetDebugConfig", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdSetDebugConfig(ref _ConoscopeDebugSettings_t config); 

    [DllImport(DLLPATH, EntryPoint = "CmdGetDebugConfig", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdGetDebugConfig(ref _ConoscopeDebugSettings_t config);

    [DllImport(DLLPATH, EntryPoint = "CmdCfgFileRead", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdCfgFileRead();

    [DllImport(DLLPATH, EntryPoint = "CmdCfgFileStatus", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdCfgFileStatus(ref CfgFileStatus_t status);

    // higher level commands
    [DllImport(DLLPATH, EntryPoint = "CmdGetCaptureSequence", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdGetCaptureSequence(ref CaptureSequenceConfig_t config);

    [DllImport(DLLPATH, EntryPoint = "CmdCaptureSequence", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdCaptureSequence(ref CaptureSequenceConfig_t config);

    [DllImport(DLLPATH, EntryPoint = "CmdCaptureSequenceCancel", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdCaptureSequenceCancel();

    [DllImport(DLLPATH, EntryPoint = "CmdCaptureSequenceStatus", CallingConvention = CALLCONV)]
    public static extern IntPtr _CmdCaptureSequenceStatus(ref CaptureSequenceStatus_t status);
    
    private static string ConvertResult(IntPtr value)
    {
        return Marshal.PtrToStringAnsi(value);
    }
    
    public static string CmdRunApp()
    {
        return ConvertResult(SdkInterface._CmdRunApp());
    }

    public static string CmdQuitApp()
    {
        return ConvertResult(SdkInterface._CmdQuitApp());
    }

    public static string CmdGetVersion()
    {
        return ConvertResult(SdkInterface._CmdGetVersion());
    }
        
    public static string CmdOpen()
    {
        return ConvertResult(SdkInterface._CmdOpen());
    }

    public static string CmdSetup(ref SetupConfig_t setup)
    {
        return ConvertResult(SdkInterface._CmdSetup(ref setup));
    }

    public static string CmdSetupStatus(ref SetupStatus_t setup)
    {
        return ConvertResult(SdkInterface._CmdSetupStatus(ref setup));
    }

    public static string CmdMeasure(ref MeasureConfig_t setup)
    {
        return ConvertResult(SdkInterface._CmdMeasure(ref setup));
    }

    public static string CmdExportRaw()
    {
        return ConvertResult(SdkInterface._CmdExportRaw());
    }

    public static string CmdExportProcessed(ref ProcessingConfig_t setup)
    {
        return ConvertResult(SdkInterface._CmdExportProcessed(ref setup));
    }

    public static string CmdClose()
    {
        return ConvertResult(SdkInterface._CmdClose());
    }

    public static string CmdReset()
    {
        return ConvertResult(SdkInterface._CmdReset());
    }
    
    public static string CmdSetConfig(ref ConoscopeSettings_t config)
    {
        _ConoscopeSettings_t _config = new _ConoscopeSettings_t();

        _config.cfgPath = (IntPtr)Marshal.StringToHGlobalAnsi(config.cfgPath);
        _config.capturePath = (IntPtr)Marshal.StringToHGlobalAnsi(config.capturePath);
        _config.autoExposure = config.autoExposure;
        _config.autoExposurePixelMax = config.autoExposurePixelMax;

        return ConvertResult(SdkInterface._CmdSetConfig(ref _config));
    }

    public static string CmdGetConfig(ref ConoscopeSettings_t config)
    {
        _ConoscopeSettings_t _config = new _ConoscopeSettings_t();
        string result = ConvertResult(SdkInterface._CmdGetConfig(ref _config));

        config.cfgPath = Marshal.PtrToStringAnsi(_config.cfgPath);
        config.capturePath = Marshal.PtrToStringAnsi(_config.capturePath);
        config.autoExposure = _config.autoExposure;
        config.autoExposurePixelMax = _config.autoExposurePixelMax;

        return result;
    }

    public static string CmdGetCmdConfig(ref SetupConfig_t setupConfig, ref MeasureConfig_t measureConfig, ref ProcessingConfig_t processingConfig)
    {
        return ConvertResult(SdkInterface._CmdGetCmdConfig(ref setupConfig, ref measureConfig, ref processingConfig));
    }

    public static string CmdSetDebugConfig(ref ConoscopeDebugSettings_t config)
    {
        _ConoscopeDebugSettings_t _config = new _ConoscopeDebugSettings_t();

        _config.debugMode = config.debugMode;
        _config.emulateCamera = config.emulateCamera;
        _config.dummyRawImagePath = (IntPtr)Marshal.StringToHGlobalAnsi(config.dummyRawImagePath);

        return ConvertResult(SdkInterface._CmdSetDebugConfig(ref _config));
    }
    
    public static string CmdGetDebugConfig(ref ConoscopeDebugSettings_t config)
    {
        _ConoscopeDebugSettings_t _config = new _ConoscopeDebugSettings_t();
        string result = ConvertResult(SdkInterface._CmdGetDebugConfig(ref _config));
        
        config.debugMode         = _config.debugMode;
        config.emulateCamera     = _config.emulateCamera;
        config.dummyRawImagePath = Marshal.PtrToStringAnsi(_config.dummyRawImagePath);
        
        return result;
    }
    
    public static string CmdCfgFileRead()
    {
        return ConvertResult(SdkInterface._CmdCfgFileRead());
    }

    public static string CmdCfgFileStatus(ref CfgFileStatus_t status)
    {
        return ConvertResult(SdkInterface._CmdCfgFileStatus(ref status));
    }
    
    public static string CmdGetCaptureSequence(ref CaptureSequenceConfig_t config)
    {
        return ConvertResult(SdkInterface._CmdGetCaptureSequence(ref config));
    }
    
    public static string CmdCaptureSequence(ref CaptureSequenceConfig_t config)
    {
        return ConvertResult(SdkInterface._CmdCaptureSequence(ref config));
    }

    public static string CmdCaptureSequenceCancel()
    {
        return ConvertResult(SdkInterface._CmdCaptureSequenceCancel());
    }

    public static string CmdCaptureSequenceStatus(ref CaptureSequenceStatus_t status)
    {
        return ConvertResult(SdkInterface._CmdCaptureSequenceStatus(ref status));
    }
}


