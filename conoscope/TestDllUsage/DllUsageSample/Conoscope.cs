using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Threading;
using System.Threading.Tasks;

using Newtonsoft.Json;

namespace TestDllUsage
{
    public class OnLogEventArgs : EventArgs
    {
        public OnLogEventArgs(string log)
        {
            logString = log;
        }

        public string logString { get; set; }
    }

    class Conoscope
    {
        public event EventHandler<OnLogEventArgs> eventLog;

        private void Logger(string message)
        {
            eventLog?.Invoke(this, new OnLogEventArgs(message));
        }

        public class Result
        {
            public int Error { get; set; }
            public string Message { get; set; }
        }

        public class Version
        {
            public int Error { get; set; }
            public string Message { get; set; }

            public string Lib_Date { get; set; }
            public string Lib_Name { get; set; }
            public string Lib_Version { get; set; }

            public string Pipeline_Date { get; set; }
            public string Pipeline_Name { get; set; }
            public string Pipeline_Version { get; set; }
        }
        public void LogReturnValue(string api, string returnValue)
        {
            Result value = JsonConvert.DeserializeObject<Result>(returnValue);
            string apiName = "" + api + "";

            Logger(string.Format("{0,-20} {1} {2}", apiName, value.Error, value.Message));
        }

        Task taskApp;

        public Conoscope()
        {
            Thread.CurrentThread.Name = "Main";

            // Create a task and supply a user delegate by using a lambda expression. 
            taskApp = new Task(() => this.AppTask());
            // Start the task.
            taskApp.Start();

            Thread.Sleep(1000);

            string returnValue = "";
            string returnInfo = "";

            // get dll version
            returnValue = SdkInterface.CmdGetVersion();

            LogReturnValue("CmdGetVersion", returnValue);
            Version version = JsonConvert.DeserializeObject<Version>(returnValue);

            Logger(string.Format("  {0,-15} {1,-10} {2}", version.Lib_Name, version.Lib_Version, version.Lib_Date));
            Logger(string.Format("  {0,-15} {1,-10} {2}", version.Pipeline_Name, version.Pipeline_Version, version.Pipeline_Date));
        }

        ~Conoscope()
        {
            CloseApplication();
        }

        void AppTask()
        {
            string returnValue;
            returnValue = SdkInterface.CmdRunApp();
        }

        public string CmdGetVersion()
        {
            return SdkInterface.CmdGetVersion();
        }

        public string CmdOpen()
        {
            return SdkInterface.CmdOpen();
        }

        public string CmdSetup(ref SdkInterface.SetupConfig_t config)
        {
            return SdkInterface.CmdSetup(ref config);
        }
        
        public string CmdSetupStatus(ref SdkInterface.SetupStatus_t setupStatus)
        {
            return SdkInterface.CmdSetupStatus(ref setupStatus);
        }

        public string CmdMeasure(ref SdkInterface.MeasureConfig_t config)
        {
            return SdkInterface.CmdMeasure(ref config);
        }

        public string CmdExportRaw()
        {
            return SdkInterface.CmdExportRaw();
        }

        public string CmdExportProcessed(ref SdkInterface.ProcessingConfig_t config)
        {
            return SdkInterface.CmdExportProcessed(ref config);
        }

        public string CmdClose()
        {
            return SdkInterface.CmdClose();
        }

        public string CmdReset()
        {
            return SdkInterface.CmdReset();
        }

        public string CmdSetConfig(ref SdkInterface.ConoscopeSettings_t config)
        {
            return SdkInterface.CmdSetConfig(ref config);
        }

        public string CmdGetConfig(ref SdkInterface.ConoscopeSettings_t config)
        {
            return SdkInterface.CmdGetConfig(ref config);
        }

        public string CmdGetCmdConfig(ref SdkInterface.SetupConfig_t setupConfig,
                                      ref SdkInterface.MeasureConfig_t measureConfig,
                                      ref SdkInterface.ProcessingConfig_t processingConfig)
        {
            return SdkInterface.CmdGetCmdConfig(
            ref setupConfig,
            ref measureConfig,
            ref processingConfig);
        }
    
        public string CmdSetDebugConfig(ref SdkInterface.ConoscopeDebugSettings_t config)
        {
            return SdkInterface.CmdSetDebugConfig(ref config);
        }

        public string CmdGetDebugConfig(ref SdkInterface.ConoscopeDebugSettings_t config)
        {
            return SdkInterface.CmdGetDebugConfig(ref config);
        }

        public string CloseApplication()
        {
            string result = SdkInterface.CmdQuitApp();

            taskApp.Wait();

            return result;
        }

        public string CmdCfgFileRead()
        {
            return SdkInterface.CmdCfgFileRead();
        }

        public string CmdCfgFileStatus(ref SdkInterface.CfgFileStatus_t status)
        {
            return SdkInterface.CmdCfgFileStatus(ref status);
        }

        public string CmdGetCaptureSequence(ref SdkInterface.CaptureSequenceConfig_t config)
        {
            return SdkInterface.CmdGetCaptureSequence(ref config);
        }

        public string CmdCaptureSequence(ref SdkInterface.CaptureSequenceConfig_t config)
        {
            return SdkInterface.CmdCaptureSequence(ref config);
        }

        public string CmdCaptureSequenceCancel()
        {
            return SdkInterface.CmdCaptureSequenceCancel();
        }

        public string CmdCaptureSequenceStatus(ref SdkInterface.CaptureSequenceStatus_t status)
        {
            return SdkInterface.CmdCaptureSequenceStatus(ref status);
        }
    }
}
