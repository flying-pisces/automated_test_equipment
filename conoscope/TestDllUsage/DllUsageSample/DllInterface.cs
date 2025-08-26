using System;

using System.Security;
using System.Runtime.InteropServices; // DllImport


//static class DllInterface
//{

//    //const string DLLPATH = @"E:\clementine\git_wa\ConoscopeSDK_TestUsage\TestUsage\PipelineLib.dll";
//    const string DLLPATH = @"PipelineLib.dll";

//    const CallingConvention CALLCONV = CallingConvention.StdCall;

//    //[System.Security.SuppressUnmanagedCodeSecurity]
//    //[DllImport(@"E:\clementine\git_wa\ConoscopeSDK\TestUsage\PipelineLib.dll", EntryPoint = "CmdTest", CallingConvention = CALLCONV)]
//    //public static extern int _CmdTest(int value);

//    //[System.Security.SuppressUnmanagedCodeSecurity]
//    //[DllImport(@"E:\clementine\git_wa\ConoscopeSDK\TestUsage\PipelineLib.dll", EntryPoint = "CmdGetVersion", CallingConvention = CALLCONV)]
//    //public static extern IntPtr _CmdGetVersion();

//    [System.Security.SuppressUnmanagedCodeSecurity]
//    [DllImport(DLLPATH, EntryPoint = "CmdTest", CallingConvention = CALLCONV)]
//    public static extern int _CmdTest(int value);

//    // extern "C" PIPELINELIBSHARED_EXPORT const char* CmdGetVersion();
//    [System.Security.SuppressUnmanagedCodeSecurity]
//    [DllImport(DLLPATH, EntryPoint = "CmdGetVersion", CallingConvention = CALLCONV)]
//    public static extern IntPtr _CmdGetVersion();

//    // extern "C" PIPELINELIBSHARED_EXPORT const char* CmdGetInfo();
//    [System.Security.SuppressUnmanagedCodeSecurity]
//    [DllImport(DLLPATH, EntryPoint = "CmdGetInfo", CallingConvention = CALLCONV)]
//    public static extern IntPtr _CmdGetInfo();
    
//    // extern "C" PIPELINELIBSHARED_EXPORT const char* CmdComputeRawData(int16 * inputData, Pipeline_RawDataParam * param, Pipeline_ResultRawDataParam & resultParam);
//    [System.Security.SuppressUnmanagedCodeSecurity]
//    [DllImport(DLLPATH, EntryPoint = "CmdComputeRawData", CallingConvention = CALLCONV)]
//    public static extern IntPtr _CmdComputeRawData();

//    // extern "C" PIPELINELIBSHARED_EXPORT const char* CmdComputeKLibData(int16 * inputData, Pipeline_KLibDataParam & param, Pipeline_CalibrationParam * calibration, int16 * klibData);
//    [System.Security.SuppressUnmanagedCodeSecurity]
//    [DllImport(DLLPATH, EntryPoint = "CmdComputeKLibData", CallingConvention = CALLCONV)]
//    public static extern IntPtr _CmdComputeKLibData();
    
//    private static string ConvertResult(IntPtr value)
//    {
//        return Marshal.PtrToStringAnsi(value);
//    }

//    public static int CmdTest(int value)
//    {
//        int res = 0;
//        res = DllInterface._CmdTest(value);
//        return res;
//    }

//    public static string CmdGetVersion()
//    {
//        return ConvertResult(DllInterface._CmdGetVersion());
//    }

//    public static string CmdGetInfo()
//    {
//        return ConvertResult(DllInterface._CmdGetInfo());
//    }
    
//    public static string CmdComputeRawData()
//    {
//        return ConvertResult(DllInterface._CmdComputeRawData());
//    }

//    public static string CmdComputeKLibData()
//    {
//        return ConvertResult(DllInterface._CmdComputeKLibData());
//    }
//}
