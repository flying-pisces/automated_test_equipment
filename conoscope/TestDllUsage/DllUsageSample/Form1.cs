using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System;
using System.Runtime.InteropServices;

using System.Threading;
using Newtonsoft.Json;

namespace TestDllUsage
{
    public partial class Form1 : Form
    {
        private class EnumObject<T>
        {
            public string Name { get; set; }
            public T Value { get; set; }
        }

        public static void PopulateComboBox<T>(ComboBox cboControl, T value, Action<T> action, bool initAction = false)
        {
            int selectedIndex = -1;

            foreach (T mode in Enum.GetValues(typeof(T)))
            {
                // the only way i found to compare values...
                if (string.Format("{0}", mode) == string.Format("{0}", value))
                {
                    selectedIndex = cboControl.Items.Count;
                }

                string ctrlName = string.Format("{0}", mode);
                cboControl.Items.Add(new EnumObject<T>() { Name = ctrlName, Value = mode });
            }

            cboControl.DisplayMember = "Name";
            cboControl.ValueMember = "Value";

            cboControl.DropDownStyle = ComboBoxStyle.DropDownList;

            cboControl.SelectedIndex = selectedIndex;

            cboControl.SelectedValueChanged += new EventHandler((a, b) =>
            {
                EnumObject<T> item = (EnumObject<T>)cboControl.SelectedItem;
                action(item.Value);
            });

            // call the action during comboBox configuration
            if (initAction == true)
            {
                EnumObject<T> item = (EnumObject<T>)cboControl.SelectedItem;
                action(item.Value);
            }
        }

        int processPipelineIndex = 0;
        int processSdkIndex = 0;
        int value = 0;

        public enum SdkApi
        {
            CmdRunApp,
            CmdQuitApp,

            CmdGetVersion,
            CmdOpen,
            CmdSetup,
            CmdSetupStatus,
            CmdMeasure,
            CmdExportRaw,
            CmdExportProcessed,
            CmdClose,
            CmdReset,

            CmdSetConfig,
            CmdGetConfig,
            CmdGetCmdConfig,
            CmdSetDebugConfig,
            CmdGetDebugConfig,


            CmdCaptureSequence
        }

        SdkApi sdkApi;
        Conoscope conoscope;

        public Form1()
        {
            InitializeComponent();
            txtLog.Text = "";

            sdkApi = SdkApi.CmdGetVersion;

            PopulateComboBox<SdkApi>(cboSdk,
               sdkApi,
               (nb) => { sdkApi = nb; }
            );

            btnSdk.Click += (sender, arg) => { ProcessSdk(); };
            btnClear.Click += (sender, arg) => { txtLog.Clear(); };
            btnCapture.Click += (sender, arg) => { ProcessCapture(); };
            btnCaptureSequence.Click += (sender, arg) => { ProcessCaptureSequence(); };
            // create the conoscope instance
            // that wil launch an internal task to run app loop
            conoscope = new Conoscope();

            // connect the log
            conoscope.eventLog += (sender, args) =>
            {
                FormHelper.Async(this, new Action(() => { Logger(args.logString); }));
            };

            this.FormClosing += (sender, arg) => { conoscope.CloseApplication(); };
        }

        public void Logger(string message)
        {
            txtLog.AppendText(message + "\r\n");
        }


        //public void ProcessPipeline()
        //{
        //    string returnValue = "";

        //    processPipelineIndex++;

        //    if (processPipelineIndex == 1)
        //    {
        //        value = DllInterface.CmdTest(value);
        //        Logger("CmdTest -> " + value);
        //    }
        //    else if (processPipelineIndex == 2)
        //    {
        //        returnValue = DllInterface.CmdGetVersion();
        //        Logger("CmdGetVersion -> " + returnValue);
        //    }
        //    else if (processPipelineIndex == 3)
        //    {
        //        returnValue = DllInterface.CmdGetInfo();
        //        Logger("CmdGetInfo -> " + returnValue);
        //    }
        //    else if (processPipelineIndex == 4)
        //    {
        //        returnValue = DllInterface.CmdComputeRawData();
        //        Logger("CmdComputeRawData -> " + returnValue);
        //    }
        //    else if (processPipelineIndex == 5)
        //    {
        //        returnValue = DllInterface.CmdComputeKLibData();
        //        Logger("CmdComputeKLibData -> " + returnValue);

        //        processPipelineIndex = 0;
        //    }
        //}

        public void SetSdkIndex(int value)
        {
            processSdkIndex = value;
        }

        public void ProcessSdk()
        {
            string returnValue = "";
            string returnInfo = "";

            switch (sdkApi)
            {
                case SdkApi.CmdGetVersion:
                    returnValue = conoscope.CmdGetVersion();
                    break;

                case SdkApi.CmdOpen:
                    returnValue = conoscope.CmdOpen();
                    break;

                case SdkApi.CmdSetup:
                    SdkInterface.SetupConfig_t setupConfig = new SdkInterface.SetupConfig_t();

                    setupConfig.sensorTemperature = 25.5F;                                   // temperature target
                    setupConfig.filterWheelPosition = SdkInterface.Filter_t.Filter_Ya;    // position of the filter wheel - TBD replace by an enum
                    setupConfig.ndWheelPosition = SdkInterface.Nd_t.Nd_2;             // position of the nd wheel - TBD replace by an enum
                    setupConfig.irisIndex = SdkInterface.Iris_t.IrisIndex_4mm;  // selected iris (this is not a command)

                    returnValue = conoscope.CmdSetup(ref setupConfig);
                    break;

                case SdkApi.CmdSetupStatus:
                    SdkInterface.SetupStatus_t setupStatus = new SdkInterface.SetupStatus_t();
                    returnValue = conoscope.CmdSetupStatus(ref setupStatus);

                    returnInfo += "\r\n";
                    returnInfo += "\r\n    sensorTemperature   " + setupStatus.sensorTemperature;
                    returnInfo += "\r\n    filterWheelPosition " + setupStatus.filterWheelPosition;
                    returnInfo += "\r\n    ndWheelPosition     " + setupStatus.ndWheelPosition;
                    returnInfo += "\r\n    irisIndex           " + setupStatus.irisIndex;

                    break;

                case SdkApi.CmdMeasure:
                    SdkInterface.MeasureConfig_t measureConfig = new SdkInterface.MeasureConfig_t();

                    measureConfig.exposureTimeUs = 50505;    // exposure time in micro seconds
                    measureConfig.nbAcquisition = 1;         // number of frames acquired (average)
                    measureConfig.binningFactor = 1;         //
                    measureConfig.bTestPattern = false;      // if set, return a test pattern returned by the sensor

                    returnValue = conoscope.CmdMeasure(ref measureConfig);
                    break;

                case SdkApi.CmdExportRaw:
                    returnValue = conoscope.CmdExportRaw();
                    break;

                case SdkApi.CmdExportProcessed:
                    SdkInterface.ProcessingConfig_t exportProcessConfig = new SdkInterface.ProcessingConfig_t();

                    exportProcessConfig.bBiasCompensation = false;
                    exportProcessConfig.bSensorDefectCorrection = false;
                    exportProcessConfig.bSensorPrnuCorrection = false;
                    exportProcessConfig.bLinearisation = false;
                    exportProcessConfig.bFlatField = false;
                    exportProcessConfig.bAbsolute = false;

                    returnValue = conoscope.CmdExportProcessed(ref exportProcessConfig);
                    break;

                case SdkApi.CmdClose:
                    returnValue = conoscope.CmdClose();
                    break;

                case SdkApi.CmdReset:
                    returnValue = conoscope.CmdReset();
                    break;

                case SdkApi.CmdSetConfig:
                    SdkInterface.ConoscopeSettings_t configSet = new SdkInterface.ConoscopeSettings_t();

                    configSet.cfgPath = ".\\CfgPathCheck";
                    configSet.capturePath = ".\\CapturePathCheck";
                    configSet.autoExposure = false;
                    configSet.autoExposurePixelMax = 80;

                    returnValue = conoscope.CmdSetConfig(ref configSet);
                    break;

                case SdkApi.CmdGetConfig:
                    SdkInterface.ConoscopeSettings_t configGet = new SdkInterface.ConoscopeSettings_t();
                    returnValue = conoscope.CmdGetConfig(ref configGet);

                    returnInfo += "  \r\n  CmdGetConfig";
                    returnInfo += "  \r\n    cfgPath               " + configGet.cfgPath;
                    returnInfo += "  \r\n    capturePath           " + configGet.capturePath;
                    returnInfo += "  \r\n    autoExposure          " + configGet.autoExposure;
                    returnInfo += "  \r\n    autoExposurePixelMax  " + configGet.autoExposurePixelMax;
                    returnInfo += "\r\n";
                    break;

                case SdkApi.CmdGetCmdConfig:
                    SdkInterface.SetupConfig_t setupConfigGet = new SdkInterface.SetupConfig_t();
                    SdkInterface.MeasureConfig_t measureConfigGet = new SdkInterface.MeasureConfig_t();
                    SdkInterface.ProcessingConfig_t processingConfigGet = new SdkInterface.ProcessingConfig_t();

                    returnValue = conoscope.CmdGetCmdConfig(
                        ref setupConfigGet,
                        ref measureConfigGet,
                        ref processingConfigGet);

                    returnInfo = "";
                    returnInfo += "  setupConfig";
                    returnInfo += "  \r\n    sensorTemperature            " + setupConfigGet.sensorTemperature;
                    returnInfo += "  \r\n    filterWheelPosition          " + setupConfigGet.filterWheelPosition;
                    returnInfo += "  \r\n    ndWheelPosition              " + setupConfigGet.ndWheelPosition;
                    returnInfo += "  \r\n    irisIndex                    " + setupConfigGet.irisIndex;

                    returnInfo += "  \r\n  measureConfig";
                    returnInfo += "  \r\n    exposureTimeUs               " + measureConfigGet.exposureTimeUs;
                    returnInfo += "  \r\n    nbAcquisition                " + measureConfigGet.nbAcquisition;
                    returnInfo += "  \r\n    binningFactor                " + measureConfigGet.binningFactor;
                    returnInfo += "  \r\n    bTestPattern                 " + measureConfigGet.bTestPattern;

                    returnInfo += "  \r\n  processingConfig";
                    returnInfo += "  \r\n    bBiasCompensation            " + processingConfigGet.bBiasCompensation;
                    returnInfo += "  \r\n    bSensorDefectCorrection      " + processingConfigGet.bSensorDefectCorrection;
                    returnInfo += "  \r\n    bSensorPrnuCorrection        " + processingConfigGet.bSensorPrnuCorrection;
                    returnInfo += "  \r\n    bLinearisation               " + processingConfigGet.bLinearisation;
                    returnInfo += "  \r\n    bFlatField                   " + processingConfigGet.bFlatField;
                    returnInfo += "  \r\n    bAbsolute                    " + processingConfigGet.bAbsolute;
                    returnInfo += "\r\n";
                    break;

                case SdkApi.CmdSetDebugConfig:
                    SdkInterface.ConoscopeDebugSettings_t debugConfigSet = new SdkInterface.ConoscopeDebugSettings_t();

                    debugConfigSet.debugMode = false;
                    debugConfigSet.emulateCamera = true;
                    debugConfigSet.dummyRawImagePath = "testFolder\\testCapture.bin";

                    returnValue = conoscope.CmdSetDebugConfig(ref debugConfigSet);
                    break;

                case SdkApi.CmdGetDebugConfig:
                    SdkInterface.ConoscopeDebugSettings_t debugConfigGet = new SdkInterface.ConoscopeDebugSettings_t();
                    returnValue = conoscope.CmdGetDebugConfig(ref debugConfigGet);

                    returnInfo += "  \r\n  CmdGetDebugConfig";
                    returnInfo += "  \r\n    debugMode            " + debugConfigGet.debugMode;
                    returnInfo += "  \r\n    emulateCamera        " + debugConfigGet.emulateCamera;
                    returnInfo += "  \r\n    dummyRawImagePath    " + debugConfigGet.dummyRawImagePath;
                    returnInfo += "\r\n";
                    break;

                default:
                    break;
            }

            Logger("" + sdkApi + "\r\n  " + returnValue + "\r\n");

            if (returnInfo.Length != 0)
            {
                Logger("" + returnInfo);
            }
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

        public void LogReturnValue(SdkApi api, string returnValue)
        {
            Result value = JsonConvert.DeserializeObject<Result>(returnValue);
            string apiName = "" + api + "";

            Logger(string.Format("{0,-20} {1} {2}", apiName, value.Error, value.Message));
        }

        public void ProcessCapture()
        {
            string returnValue = "";
            string returnInfo = "";

            // get dll version
            returnValue = conoscope.CmdGetVersion();
            LogReturnValue(SdkApi.CmdGetVersion, returnValue);
            Version version = JsonConvert.DeserializeObject<Version>(returnValue);

            Logger(string.Format("  {0,-15} {1,-10} {2}", version.Lib_Name, version.Lib_Version, version.Lib_Date));
            Logger(string.Format("  {0,-15} {1,-10} {2}", version.Pipeline_Name, version.Pipeline_Version, version.Pipeline_Date));

            // retrieve current configuration
            SdkInterface.SetupConfig_t setupConfig = new SdkInterface.SetupConfig_t();
            SdkInterface.MeasureConfig_t measureConfig = new SdkInterface.MeasureConfig_t();
            SdkInterface.ProcessingConfig_t processingConfig = new SdkInterface.ProcessingConfig_t();
            SdkInterface.ConoscopeSettings_t settings = new SdkInterface.ConoscopeSettings_t();
            SdkInterface.ConoscopeDebugSettings_t debugSettings = new SdkInterface.ConoscopeDebugSettings_t();

            returnValue = conoscope.CmdGetCmdConfig(ref setupConfig, ref measureConfig, ref processingConfig);
            LogReturnValue(SdkApi.CmdGetCmdConfig, returnValue);
            
            returnValue = conoscope.CmdGetConfig(ref settings);
            LogReturnValue(SdkApi.CmdGetConfig, returnValue);

            returnValue = conoscope.CmdGetDebugConfig(ref debugSettings);
            LogReturnValue(SdkApi.CmdGetDebugConfig, returnValue);

            // start the app (only once)
            // returnValue = SdkInterface.CmdRunApp();
            // LogReturnValue(SdkApi.CmdRunApp, returnValue);

            // set mode
            debugSettings.debugMode = false;
            debugSettings.emulateCamera = false;

            returnValue = conoscope.CmdSetDebugConfig(ref debugSettings);
            LogReturnValue(SdkApi.CmdSetDebugConfig, returnValue);

            // set path
            settings.cfgPath = "..\\_Cfg";
            settings.capturePath = "..\\_Capture";

            settings.autoExposure = false;
            settings.autoExposurePixelMax = 80;

            returnValue = conoscope.CmdSetConfig(ref settings);
            LogReturnValue(SdkApi.CmdSetConfig, returnValue);

            // open
            returnValue = conoscope.CmdOpen();
            LogReturnValue(SdkApi.CmdOpen, returnValue);

            // setup
            setupConfig.sensorTemperature = 25.5F;
            setupConfig.filterWheelPosition = SdkInterface.Filter_t.Filter_Ya;
            setupConfig.ndWheelPosition = SdkInterface.Nd_t.Nd_2;
            setupConfig.irisIndex = SdkInterface.Iris_t.IrisIndex_4mm;

            returnValue = conoscope.CmdSetup(ref setupConfig);
            LogReturnValue(SdkApi.CmdSetup, returnValue);

            // check setup
            //returnValue = SdkInterface.CmdGetCmdConfig(ref setupConfig, ref measureConfig, ref processingConfig);
            //LogReturnValue(SdkApi.CmdGetCmdConfig, returnValue);

            // capture
            measureConfig.exposureTimeUs = 50000;
            measureConfig.nbAcquisition = 1;
            measureConfig.bTestPattern = false;
            measureConfig.binningFactor = 1;
            
            returnValue = conoscope.CmdMeasure(ref measureConfig);
            LogReturnValue(SdkApi.CmdMeasure, returnValue);

            returnValue = conoscope.CmdExportRaw();
            LogReturnValue(SdkApi.CmdExportRaw, returnValue);

            conoscope.CmdClose();
            LogReturnValue(SdkApi.CmdClose, returnValue);

            // don't forget to call CloseApplication once every thing is finished
        }

        public void ProcessCaptureSequence()
        {
            string returnValue;

            SdkInterface.SetupConfig_t setupConfig = new SdkInterface.SetupConfig_t();
            SdkInterface.MeasureConfig_t measureConfig = new SdkInterface.MeasureConfig_t();
            SdkInterface.ProcessingConfig_t processingConfig = new SdkInterface.ProcessingConfig_t();
            SdkInterface.ConoscopeSettings_t settings = new SdkInterface.ConoscopeSettings_t();
            SdkInterface.ConoscopeDebugSettings_t debugSettings = new SdkInterface.ConoscopeDebugSettings_t();
            
            // set mode
            debugSettings.debugMode = false;
            debugSettings.emulateCamera = false;
            debugSettings.dummyRawImagePath = "";

            returnValue = conoscope.CmdSetDebugConfig(ref debugSettings);
            LogReturnValue(SdkApi.CmdSetDebugConfig, returnValue);

            // set path
            settings.cfgPath = "..\\_Cfg";
            settings.capturePath = "..\\_Capture";

            settings.autoExposure = false;
            settings.autoExposurePixelMax = 80;

            returnValue = conoscope.CmdSetConfig(ref settings);
            LogReturnValue(SdkApi.CmdSetConfig, returnValue);

            // open
            returnValue = conoscope.CmdOpen();
            LogReturnValue(SdkApi.CmdOpen, returnValue);

            // launch capture sequence
            SdkInterface.CaptureSequenceConfig_t captureSequenceConfig = new SdkInterface.CaptureSequenceConfig_t();

            captureSequenceConfig.sensorTemperature = 25.0F;
            captureSequenceConfig.bWaitForSensorTemperature = false;

            captureSequenceConfig.eNd = SdkInterface.Nd_t.Nd_0;
            captureSequenceConfig.eIris = SdkInterface.Iris_t.IrisIndex_2mm;

            captureSequenceConfig.exposureTimeUs = 100000;
            captureSequenceConfig.nbAcquisition = 1;   
            captureSequenceConfig.bAutoExposure = false;
            captureSequenceConfig.bUseExpoFile = false;
            
            conoscope.CmdCaptureSequence(ref captureSequenceConfig);
            LogReturnValue(SdkApi.CmdCaptureSequence, returnValue);

            bool bDone = false;
            SdkInterface.CaptureSequenceStatus_t captureSequenceStatus = new SdkInterface.CaptureSequenceStatus_t();

                        SdkInterface.Filter_t eFilterPrev = SdkInterface.Filter_t.Filter_Invalid;
            SdkInterface.CaptureSequenceState_t eState = SdkInterface.CaptureSequenceState_t.State_Done;

            do
            {
                Thread.Sleep(5000);

                conoscope.CmdCaptureSequenceStatus(ref captureSequenceStatus);

                if((captureSequenceStatus.eFilter != eFilterPrev) || (captureSequenceStatus.eState != eState))
                {
                    eFilterPrev = captureSequenceStatus.eFilter;
                    eState = captureSequenceStatus.eState;

                    //    ic int nbSteps;
                    //public int currentSteps;
                    string filterString = "" + captureSequenceStatus.eFilter;
                    string stateString = "" + captureSequenceStatus.eState;

                    Logger(string.Format("  {0,-15} {1,-15} step {2}/{3}", stateString, filterString,
                        captureSequenceStatus.currentSteps, captureSequenceStatus.nbSteps));

                }

                if((captureSequenceStatus.eState == SdkInterface.CaptureSequenceState_t.State_Done) ||
                    (captureSequenceStatus.eState == SdkInterface.CaptureSequenceState_t.State_Error))
                {
                    bDone = true;
                }

            } while (bDone == false);
        }
    }
}
