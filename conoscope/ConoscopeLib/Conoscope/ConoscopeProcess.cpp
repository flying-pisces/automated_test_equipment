#include "ConoscopeProcess.h"
#include "conoscopeTypes.h"
#include "ConoscopeConfig.h"

#include "cameraCmvCxp.h"
#include "cameraDummy.h"

#include "imageConfiguration.h"

#include <QDir>
#include "configuration.h"

#include <QJsonObject>
#include <QJsonDocument>

#include "PipelineLib.h"

#include <QProcess>

#include "ConoscopeResource.h"

#include "toolString.h"
#include "toolReturnCode.h"

#include <QElapsedTimer>

#define RAW_FILE_NAME "%1_raw"
#define PROCESSED_FILE_NAME "%1_proc"

#define FILE_NAME "%1_filt_%2_nd_%3_iris_%4"
#define FILE_NAME_OPTION "_src_%1"

#define CAPTURE_EXTENSION "bin"

#define INSTANCE ConoscopeProcess* instance = _GetInstance(); return instance

#define MAX_ATTEMPTS    2000
#define SLEEP_TIME_MS   100
#define TIME_QUANTA_MS  10

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

// #define CHECK_POSITION
#define LOG_HEADER "[ConoscopeProcess]"
// #define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | ").arg(LOG_HEADER, -20), text)

#define LOG_APP_HEADER "ConoscopeProcess"
#define LogInApp(text) RESOURCE->Log(ToolsString::FormatText(LOG_APP_HEADER, text));

#define IMAGE_INFO_EXTENSION ".json"
#define IMAGE_JPG_EXTENSION ".jpg"

ConoscopeProcess* ConoscopeProcess::mInstance = NULL;

ConoscopeDebugSettings_t ConoscopeProcess::mDebugSettings;
ConoscopeSettings_t      ConoscopeProcess::mSettings;
ConoscopeSettingsI_t     ConoscopeProcess::mSettingsI;
Info_t                   ConoscopeProcess::mInfo;
MeasurementAdditionalInfo_t ConoscopeProcess::mAdditionalInfo;

ConoscopeProcess::ConoscopeProcess(QObject *parent) : ClassCommon(parent)
{
    mInfo.cfgFileName = CONVERT_TO_QSTRING(mSettingsI.cfgFileName);

    // initialise CfgFile status
    mCfgFileStatus.eState      = CfgFileState_NotDone;
    mCfgFileStatus.progress    = 0;
    mCfgFileStatus.elapsedTime = 0;
    mCfgFileStatus.fileName    = std::string("");

    // set some constants depending on HW configuration
    _ConfigureHal();

    // create the pipeline
    mPipelineLib = new PipelineLib(parent);

    mCamera = nullptr;
    mDevices = nullptr;
    mTempMonitor = nullptr;

#ifndef CREATE_CAMERA_DURING_OPEN
    // create the camera and all the devices
    _CreateCamera();
#endif
}

ConoscopeProcess::~ConoscopeProcess()
{
}

void ConoscopeProcess::_ConfigureHal()
{
    // map filter settings to wheel index
    NdWheelMap[Nd_0] = 8;
    NdWheelMap[Nd_1] = 1;
    NdWheelMap[Nd_2] = 3;
    NdWheelMap[Nd_3] = 5;
    NdWheelMap[Nd_4] = 7;

    for(std::map<Nd_t,int>::iterator it = NdWheelMap.begin(); it != NdWheelMap.end(); ++it)
    {
        NdWheelRevertMap[it->second] = it->first;
    }

    FilterWheelMap[Filter_BK7]    = 2;
    FilterWheelMap[Filter_Mirror] = 1;
    FilterWheelMap[Filter_X]      = 8;
    FilterWheelMap[Filter_Xz]     = 5;
    FilterWheelMap[Filter_Ya]     = 7;
    FilterWheelMap[Filter_Yb]     = 6;
    FilterWheelMap[Filter_Z]      = 4;
    FilterWheelMap[Filter_IrCut]  = 3;

    for(std::map<Filter_t,int>::iterator it = FilterWheelMap.begin(); it != FilterWheelMap.end(); ++it)
    {
        FilterWheelRevertMap[it->second] = it->first;
    }

    WheelStatusMap[WheelStatus_Idle]       = WheelState_Idle;
    WheelStatusMap[WheelStatus_Success]    = WheelState_Success;
    WheelStatusMap[WheelStatus_Operating]  = WheelState_Operating;
    WheelStatusMap[WheelStatus_Error]      = WheelState_Error;
    WheelStatusMap[WheelStatus_Bits]       = WheelState_Error;

    WheelTypeIndexMap[WheelType_Filter] = 1;
    WheelTypeIndexMap[WheelType_Nd] = 0;
}

void ConoscopeProcess::_CreateCamera()
{
    if(mCamera == nullptr)
    {
        // create the camera
        if(mDebugSettings.emulateCamera == true)
        {
            CameraDummy* cameraDummy;
            cameraDummy = new CameraDummy();

            mCamera = cameraDummy;
        }
        else
        {
            CameraCmvCxp* cameraCmvCxp;
            cameraCmvCxp = new CameraCmvCxp();
            cameraCmvCxp->SetScriptStatement(":/config_Adimec_S50_FS.js");

            mCamera = cameraCmvCxp;
        }

        connect(mCamera, &Camera::LogInFile,
                this, &ConoscopeProcess::OnCameraLogInFile);

        mDevices = new CDevices (this,mCamera);

        // temperature monitoring is asynchronous
        mTempMonitor = new TempMonitoring(mCamera);
        mTempMonitor->Start();
    }
}

void ConoscopeProcess::_DeleteCamera()
{
    // stop temperature monitoring
    if(mTempMonitor != nullptr)
    {
        mTempMonitor->Stop();

        delete mTempMonitor;
    }

    if(mDevices != nullptr)
    {
        delete mDevices;
    }

    if(mCamera != nullptr)
    {
        // delete mCamera;
    }
}

ConoscopeProcess* ConoscopeProcess::_GetInstance()
{
    if(mInstance == NULL)
    {
        mInstance = new ConoscopeProcess();
    }

    return mInstance;
}
void ConoscopeProcess::Stop()
{
    // stop temperature monitoring
    if(mInstance != NULL)
    {
        mInstance->_DeleteCamera();
    }
}

void ConoscopeProcess::Delete()
{
    if(mInstance != NULL)
    {
        delete(mInstance);
        mInstance = NULL;
    }
}

QString ConoscopeProcess::CmdGetPipelineVersion()
{
    INSTANCE->_CmdGetPipelineVersion();
}

ClassCommon::Error ConoscopeProcess::CmdOpen()
{
    INSTANCE->_CmdOpen();
}

ClassCommon::Error ConoscopeProcess::CmdSetup(SetupConfig_t &config)
{
    INSTANCE->_CmdSetup(config);
}

ClassCommon::Error ConoscopeProcess::CmdSetupStatus(SetupStatus_t& status)
{
    INSTANCE->_CmdSetupStatus(status);
}

ClassCommon::Error ConoscopeProcess::CmdMeasure(MeasureConfigWithCropFactor_t &config, bool updateCaptureDate)
{
    INSTANCE->_CmdMeasure(config, updateCaptureDate);
}

ClassCommon::Error ConoscopeProcess::CmdExportRaw()
{
    INSTANCE->_CmdExportRaw();
}

ClassCommon::Error ConoscopeProcess::CmdExportRaw(std::vector<uint16_t> &buffer)
{
    INSTANCE->_CmdExportRaw(buffer);
}

ClassCommon::Error ConoscopeProcess::CmdExportProcessed(ProcessingConfig_t& config)
{
    INSTANCE->_CmdExportProcessed(config);
}

ClassCommon::Error ConoscopeProcess::CmdExportProcessed(ProcessingConfig_t& config, std::vector<int16_t> &buffer, bool bSaveImage)
{
    INSTANCE->_CmdExportProcessed(config, buffer, bSaveImage);
}

ClassCommon::Error ConoscopeProcess::CmdClose()
{
    INSTANCE->_CmdClose();
}

ClassCommon::Error ConoscopeProcess::CmdReset()
{
    INSTANCE->_CmdReset();
}

ClassCommon::Error ConoscopeProcess::CmdSetupDebug(SetupConfig_t &config)
{
    INSTANCE->_CmdSetupDebug(config);
}

ClassCommon::Error ConoscopeProcess::CmdCfgFileWrite()
{
    INSTANCE->_CmdCfgFileWrite();
}

ClassCommon::Error ConoscopeProcess::CmdCfgFileRead()
{
    INSTANCE->_CmdCfgFileRead();
}

ClassCommon::Error ConoscopeProcess::CmdCfgFileStatus(CfgFileStatus_t& status)
{
    INSTANCE->_CmdCfgFileStatus(status);
}

ClassCommon::Error ConoscopeProcess::CmdConvertRaw(ConvertRaw_t &param)
{
    INSTANCE->_CmdConvertRaw(param);
}

void ConoscopeProcess::GetSomeInfo(SomeInfo_t& info)
{
    INSTANCE->_GetSomeInfo(info);
}

ConoscopeProcess* ConoscopeProcess::GetInstance()
{
    return ConoscopeProcess::_GetInstance();
}

QString ConoscopeProcess::_CmdGetPipelineVersion()
{
    QString message;
    message = mPipelineLib->GetVersion();
    return message;
}

ClassCommon::Error ConoscopeProcess::_CmdOpen()
{
    LogInFile("_CmdOpen");

    ClassCommon::Error eError;

    // initialise the setup status flag
    mSetupWheelFailure = SetupWheelStatus_Unknown;

#ifdef CREATE_CAMERA_DURING_OPEN
    // create the camera and all the devices
    _CreateCamera();
#endif

    // connect the camera
    QString cameraSerialNumber = "SN_0";
    QString cameraBoardSerialNumber = "CriticalLink_0";

    QString ipAddr = "TBD_ipAddr";
    QString port = "TBD_port";
    QString connectionConfig = "CXP6_X2";

    _Log("  Register");
    eError = mCamera->Register(cameraSerialNumber, cameraBoardSerialNumber);

    if(eError == ClassCommon::Error::Ok)
    {
        _Log("  ConfigureConnection");
        eError = mCamera->ConfigureConnection(ipAddr, port, connectionConfig);
    }

    // connect the camera
    if(eError == ClassCommon::Error::Ok)
    {
        _Log("  Connect");
        eError = mCamera->Connect();

        ERROR_DESCRIPTION("Problem during camera connect");
    }

    if(eError == ClassCommon::Error::Ok)
    {
        _GetCameraInfo();
    }

#ifdef SET_TEMPERATURE
    _WaitForSetupIsDone();

    mTempMonitor->CmdReset();
#endif

    LogInFile(QString("_CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdSetup(SetupConfig_t &config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    bool bWheelError = false;

    LogInFile("_CmdSetup");

    if(mDebugSettings.emulateCamera == true)
    {
        mDebugSettings.emulateWheel = true;
    }

    _setupConfig = config;

    // initialise the flag
    mSetupWheelFailure = SetupWheelStatus_Success;

    _Log("Set wheel position");
    int retryCount = 3;

    int nbWheelIndex = NdWheelMap[config.eNd];
    int filterWheelIndex = FilterWheelMap[config.eFilter];

    // wheel index in range [1, 8]
    CDevices::Status_t eStatus;
    unsigned int waitDelayMs;

#ifdef CHECK_WHEEL_INTEGRITY
    LogInApp (QString("CheckWheel - ND     : %1").arg(RESOURCE->ToString(config.eNd)));
    LogInFile(QString("CheckWheel - ND     : %1").arg(RESOURCE->ToString(config.eNd)));

    LogInApp (QString("CheckWheel - ND     : wheel = %2 index = %3").arg(WheelTypeIndexMap[WheelType_Nd]).arg(nbWheelIndex));
    LogInFile(QString("CheckWheel - ND     : wheel = %2 index = %3").arg(WheelTypeIndexMap[WheelType_Nd]).arg(nbWheelIndex));

    LogInApp (QString("CheckWheel - filter : %1").arg(RESOURCE->ToString(config.eFilter)));
    LogInFile(QString("CheckWheel - filter : %1").arg(RESOURCE->ToString(config.eFilter)));

    LogInApp (QString("CheckWheel - filter : wheel = %2 index = %3").arg(WheelTypeIndexMap[WheelType_Filter]).arg(filterWheelIndex));
    LogInFile(QString("CheckWheel - filter : wheel = %2 index = %3").arg(WheelTypeIndexMap[WheelType_Filter]).arg(filterWheelIndex));


    LogInApp (QString("CheckWheel - emulate camera %1, emulate wheel %2").arg(mDebugSettings.emulateCamera).arg(mDebugSettings.emulateWheel));
    LogInFile(QString("CheckWheel - emulate camera %1, emulate wheel %2").arg(mDebugSettings.emulateCamera).arg(mDebugSettings.emulateWheel));
#endif

    if(mDebugSettings.emulateWheel == false)
    {
#ifdef CHECK_WHEEL_INTEGRITY
        bool bIntegrity = true;

        if(mDevices->BiWheelIsPresent() == true)
        {
            LogInApp ("CheckWheel - BiWheelIsPresent ok");
            LogInFile("CheckWheel - BiWheelIsPresent ok");
        }
        else
        {
            LogInApp ("CheckWheel - BiWheelIsPresent FAILED");
            LogInFile("CheckWheel - BiWheelIsPresent FAILED");

            bIntegrity = false;
        }

        if(bIntegrity == true)
        {
            unsigned char motorStatus = mDevices->GetMotorStatus();

            QString motorStatusMessage = QString("CheckWheel - MotorStatus = %1 ").arg(motorStatus);

            switch(motorStatus)
            {
            case MOTORIdle:
                motorStatusMessage += "Idle";
                break;
            case MOTORSuccess:
                motorStatusMessage += "Success";
                break;
            case MOTOROperating:
                motorStatusMessage += "Operating";
                break;
            case MOTORError:
                motorStatusMessage += "Error";
                break;
            case MOTORStatusBits:
                motorStatusMessage += "StatusBits";
                break;
            case MOTORRetriesBits:
                motorStatusMessage += "RetriesBits";
                break;
            default:
                motorStatusMessage += "Unknown";
                break;
           }

            if((motorStatus == MOTOROperating) ||
               (motorStatus == MOTORRetriesBits) ||
               (motorStatus == MOTORStatusBits))
            {
                motorStatusMessage += " ERROR";
                bIntegrity = false;
            }

            LogInApp (motorStatusMessage);
            LogInFile(motorStatusMessage);
        }

#ifdef REMOVED
        if((ucIndex < 1) || (ucIndex > BiWheelNbOfPositions))
        {
            qDebug() << QString("    BiWheelGoto  ERROR 2");
            return(CDevices::Status_t::Status_Error);
        }

        mintTargetBWPosition = ucIndex;
        mintCurrentBWWheel = ucMotorNumber;

        intCurrentPosition =  BiWheelPosition(mintCurrentBWWheel) ;

        if (mintTargetBWPosition == intCurrentPosition)
        {
            qDebug() << QString("    BiWheelGoto  Done");
            return(CDevices::Status_t::Status_Done);
        }

        BiWheelSelect (ucMotorNumber);
        mMotor->SetEnabled(true);
        mMotor->SetEventsSinkAddress(0x00);

        // select direction
        intDistance = mintTargetBWPosition - intCurrentPosition ;

        // calculate the delay required to move the wheel
        waitDelayMs = WAIT_DELAY_UNIT_MS * ((intDistance > 0) ? intDistance : -intDistance);
        // qDebug() << QString(" >> BiWheelGoTo distance %1 -> %2").arg(intDistance).arg(waitDelayMs);

        if (intDistance > 0)
        {
            if (intDistance <= (BiWheelNbOfPositions / 2))
            {
                mMotor->SetDirection (PositiveDirection) ;
            }
            else
            {
                mMotor->SetDirection (NegativeDirection) ;
            }
        }
        else
        {
            if (intDistance >= -(BiWheelNbOfPositions / 2))
            {
                mMotor->SetDirection (NegativeDirection) ;
            }
            else
            {
                mMotor->SetDirection (PositiveDirection) ;
            }
        }

        mMotor->SetStartupStepPeriod (10100) ;
        mMotor->SetMinimumStepPeriod (1100) ;
        mMotor->SetRampUpIncrement (1000) ;

        mMotor->SetCount(4000) ;
        mMotor->SetStatus(MOTORIdle) ;
        mMotor->SetPosition(ucIndex) ;

        mMotor->Search();

        // qDebug() << QString("  BiWheelGoto  Processing");
        return(CDevices::Status_t::Status_Processing);

#endif


#endif

        // ND wheel
        if(eError == ClassCommon::Error::Ok)
        {
            _Log(QString("Set wheel position ND    : wheel = %1 index = %2").arg(WheelTypeIndexMap[WheelType_Nd]).arg(nbWheelIndex));

            eStatus = mDevices->BiWheelGoto(WheelTypeIndexMap[WheelType_Nd], nbWheelIndex, waitDelayMs);

            if(eStatus == CDevices::Status_t::Status_Processing)
            {
                // wait for the end of processing
                bWheelError = mDevices->BiWheelWaitForReady(retryCount, waitDelayMs);

                if(bWheelError == false)
                {
                    LogInApp("_CmdSetup ERROR      ND wheel failure");
                    LogInFile("_CmdSetup ERROR  ND wheel failure");

                    mSetupWheelFailure = SetupWheelStatus_Failure;
                    eError = ClassCommon::Error::Failed;

                    ERROR_DESCRIPTION("ND wheel failure");
                }
            }
            else if((eStatus == CDevices::Status_t::Status_Error) || (eStatus == CDevices::Status_t::Status_Unknown))
            {
                mSetupWheelFailure = SetupWheelStatus_Failure;
                eError = ClassCommon::Error::Failed;

                ERROR_DESCRIPTION("ND wheel not processing");
            }
        }

        // filter wheel
        if(eError == ClassCommon::Error::Ok)
        {
            _Log(QString("Set wheel position filter: wheel = %1 index = %2").arg(WheelTypeIndexMap[WheelType_Filter]).arg(filterWheelIndex));

            // wheel index in range [1, 8]
            eStatus = mDevices->BiWheelGoto(WheelTypeIndexMap[WheelType_Filter], filterWheelIndex, waitDelayMs);

            if(eStatus == CDevices::Status_t::Status_Processing)
            {
                bWheelError = mDevices->BiWheelWaitForReady(retryCount, waitDelayMs);

                if(bWheelError == false)
                {
                    LogInApp("_CmdSetup ERROR  filter wheel failure");
                    LogInFile("_CmdSetup ERROR  filter wheel failure");

                    mSetupWheelFailure = SetupWheelStatus_Failure;
                    eError = ClassCommon::Error::Failed;

                    ERROR_DESCRIPTION("filter wheel failure");
                }
            }
            else if((eStatus == CDevices::Status_t::Status_Error) || (eStatus == CDevices::Status_t::Status_Unknown))
            {
                mSetupWheelFailure = SetupWheelStatus_Failure;
                eError = ClassCommon::Error::Failed;

                ERROR_DESCRIPTION("filter wheel not processing");
            }
        }
    }

    // Set temperature
#ifdef SET_TEMPERATURE

    if(eError == ClassCommon::Error::Ok)
    {
        _Log("  SetTemperature");

        // check whether temperature target has changed
        bool changeSetup = _HasSetupChanged(config.sensorTemperature);

        if(changeSetup == true)
        {
            // check whether temperature monitoring is on going
            _WaitForSetupIsDone();

            int setTemperatureTimeout = 60000;
            eError = mTempMonitor->CmdSetTemperature(config.sensorTemperature, setTemperatureTimeout);

            if(eError != ClassCommon::Error::Ok)
            {
                _Log("  ERROR Temperature monitoring");
            }

            ERROR_DESCRIPTION("ERROR Temperature monitoring");
        }
        else
        {
            _Log("  No need to change temperature monitoring target");
            eError = ClassCommon::Error::Ok;
        }
    }
#else
    eError = ClassCommon::Error::Ok;
#endif

    if(eError == ClassCommon::Error::Ok)
    {
        LogInFile(QString("_CmdSetup done"));
    }
    else
    {
        LogInFile(QString("_CmdSetup error (%1)").arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdSetupStatus(SetupStatus_t& status)
{
    LogInFile("_CmdSetupStatus");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // retrieve wheel status
    if(mSetupWheelFailure == SetupWheelStatus_Success)
    {
        WheelStatus_t eWheelStatus = _GetWheelStatus(status.eNd, status.eFilter);

        if(WheelStatusMap.count(eWheelStatus) != 0)
        {
            status.eWheelStatus = WheelStatusMap[eWheelStatus];
        }
        else
        {
            status.eWheelStatus = WheelState_Error;
        }
    }
    else
    {
        status.eWheelStatus = WheelState_Error;
    }

    status.eIris   = _setupConfig.eIris;

    if(mDebugSettings.emulateWheel == true)
    {
        // indicate that wheel configuration has succeed anyway
        status.eWheelStatus = WheelState_t::WheelState_Success;
    }

    // retrieve temperature
    TemperatureMonitoringState_t eTemperatureMonitoringState;
    float temp;

    mTempMonitor->GetTemperature(eTemperatureMonitoringState, temp);

    status.eTemperatureMonitoringState = eTemperatureMonitoringState;
    status.sensorTemperature = temp;

    LogInFile(QString("_CmdSetupStatus %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdMeasure(MeasureConfigWithCropFactor_t &config, bool updateCaptureDate)
{
    QString message;

    message.append("_CmdMeasure\n");

    message.append(QString("    exposureTimeUs %1\n").arg(config.exposureTimeUs));
    message.append(QString("    nbAcquisition  %1\n").arg(config.nbAcquisition));
    message.append(QString("    binningFactor  %1\n").arg(config.binningFactor));
    message.append(QString("    bTestPattern   %1").arg(config.bTestPattern));

    message.append((QString("\n    crop         X %1\n").arg(config.cropArea.x())));
    message.append((QString("    crop         Y %1\n").arg(config.cropArea.y())));
    message.append((QString("    crop     width %1\n").arg(config.cropArea.width())));
    message.append((QString("    crop    height %1").arg(config.cropArea.height())));

    LogInFile(message);

    ClassCommon::Error eError = ClassCommon::Error::Ok;
    bool bDone = false;
    int numAttempts = 0;

    if(mDebugSettings.emulateCamera == true)
    {
        CameraDummy* cameraDummy = (CameraDummy*) mCamera;

        QMap<QString, QVariant> settings;

        QString dummyRawImagePath = CONVERT_TO_QSTRING(mDebugSettings.dummyRawImagePath);
        eError = cameraDummy->LoadRawImage(dummyRawImagePath, settings);

        ERROR_DESCRIPTION("ERROR can not load data");

        if(eError == ClassCommon::Error::Ok)
        {
            _GetCameraInfo();
        }

        if(eError == ClassCommon::Error::Ok)
        {
            // retrieve the configuration of the capture
            config.exposureTimeUs = settings["exposureTimeUs"].toInt();
            config.nbAcquisition  = settings["nbAcquisition"].toInt();
            config.binningFactor  = settings["binningFactor"].toInt();
            config.bTestPattern   = settings["bTestPattern"].toBool();

            // retrieve setup matching the capture
            _setupConfig.sensorTemperature = settings["setupSensorTemperature"].toFloat();
            _setupConfig.eFilter           = (Filter_t)settings["setupFilter"].toInt();
            _setupConfig.eNd               = (Nd_t)settings["setupNd"].toInt();
            _setupConfig.eIris             = (IrisIndex_t)settings["setupIris"].toInt();

            // retrieve information from camera
            _OpeningInfo();
        }

        if(eError == ClassCommon::Error::Ok)
        {
            _captureInfo.timeStampString = settings["timeStampString"].toString();
            _timeStampString_test = settings["timeStampString"].toString();
            _captureInfo.timeStampDate = settings["timeStampDate"].toString();
            _captureInfo.timeStampTime = settings["timeStampTime"].toString();
        }
    }

    // store configuration
    mInfo.exposureTimeUs = config.exposureTimeUs;
    mInfo.nbAcquisition  = config.nbAcquisition;
    mInfo.binningFactor  = config.binningFactor;
    mInfo.bTestPattern   = config.bTestPattern;

    mInfo.AeEnable                = config.AeEnable;
    mInfo.AeExpoTimeGranularityUs = config.AeExpoTimeGranularityUs;

    // store additional info
    mAdditionalInfo.bAeEnable        = config.bAeEnable;
    mAdditionalInfo.AEMeasAreaHeight = config.AEMeasAreaHeight;
    mAdditionalInfo.AEMeasAreaWidth  = config.AEMeasAreaWidth;
    mAdditionalInfo.AEMeasAreaX      = config.AEMeasAreaX;
    mAdditionalInfo.AEMeasAreaY      = config.AEMeasAreaY;

    Camera::Status eStatus;
    Camera::CaptureConfig cameraConfig;

    ImageConfiguration* imageConfiguration = ImageConfiguration::Get();

    cameraConfig.mnExposureMicros  = config.exposureTimeUs;
    cameraConfig.mnNumImages       = config.nbAcquisition;
    cameraConfig.mnVBin            = config.binningFactor;    // not used

#ifdef AE_ROI
    if((config.cropArea.height() == 0) || (config.cropArea.width() == 0))
    {
        cameraConfig.mcDimensions  = QRect(0, 0, imageConfiguration->image_width, imageConfiguration->image_height);
    }
    else
    {
        cameraConfig.mcDimensions  = QRect(config.cropArea.x(),
                                           config.cropArea.y(),
                                           config.cropArea.width(),
                                           config.cropArea.height());
    }
#else
    cameraConfig.mcDimensions  = QRect(0, 0, imageConfiguration->image_width, imageConfiguration->image_height);
#endif

    cameraConfig.mbExtTrig         = false;                   // not used
    cameraConfig.mbTestPattern     = config.bTestPattern;
    cameraConfig.mnTrigDelayMicros = 0;                       // not used
    cameraConfig.bStoreStdDev      = false;

    _measurementConfig = _setupConfig;

    if(_measurementConfig.eFilter == Filter_Mirror)
    {
        LogInFile("  ERROR invalid setup: Mirror");
        eError = ClassCommon::Error::InvalidConfiguration;

        ERROR_DESCRIPTION("ERROR invalid setup: Mirror");
    }

    if(eError == ClassCommon::Error::Ok)
    {
        _Log("Configure");
        LogInFile("mCamera->Configure");
        eError = mCamera->Configure(&cameraConfig);

        // update exposure time with the time set (due to exposure time granularity of the camera)
        config.exposureTimeUs = cameraConfig.mnExposureMicros;
        mInfo.exposureTimeUs  = cameraConfig.mnExposureMicros;

        ERROR_DESCRIPTION("ERROR configure");
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // check if the camera has not been changed since open
        // it may not have been detected
        eError = mCamera->CheckConnection();
        ERROR_DESCRIPTION("ERROR camera serial number has changed");
    }

    if(eError == ClassCommon::Error::Ok)
    {
        _Log("StartMeasurement");
        LogInFile("mCamera->StartMeasurement");
        eError = mCamera->StartMeasurement();

        ERROR_DESCRIPTION("ERROR measurement");
    }

    _Log("wait Measurement");

    // wait for the end of the measurement
    if(eError == ClassCommon::Error::Ok)
    {
        do {
            mCamera->GetStatus(eStatus);

            if(eError == ClassCommon::Error::Ok)
            {
                if(eStatus == Camera::Status::MeasurementPending)
                {
                    bDone = false;

                    if(numAttempts++ >= MAX_ATTEMPTS)
                    {
                         // Log("", "GetStatus FAILED after retry");
                         eError = ClassCommon::Error::FailedMaxRetry;

                         ERROR_DESCRIPTION("ERROR measurement timeout");
                    }
                    else
                    {
                        QThread::msleep(SLEEP_TIME_MS);
                        int timeMs = SLEEP_TIME_MS;

                        do
                        {
                            QThread::msleep(TIME_QUANTA_MS);
                            QApplication::processEvents(QEventLoop::AllEvents, TIME_QUANTA_MS);
                            timeMs -= TIME_QUANTA_MS;
                        } while(timeMs > 0);
                    }
                }
                else
                {
                    bDone = true;
                }
            }
        } while((eError == ClassCommon::Error::Ok) && (bDone == false)) ;

        if(eError == ClassCommon::Error::Ok)
        {
            _Log("Capture Done");
        }
        else
        {
            _Log("Capture Failed");
        }
    }

    // then retrieve the raw data
    if(eError == ClassCommon::Error::Ok)
    {
        /* raw data info comming from camera */
        struct Camera::RawDataInfo rawDataInfo;

        rawDataInfo.miCols = imageConfiguration->image_width;
        rawDataInfo.miLines = imageConfiguration->image_height;

        rawDataInfo.cropArea = config.cropArea;

        LogInFile("mCamera->GetRawData");
        eError = mCamera->GetRawData(rawDataInfo, _rawData, _rawDataStdDev);

        ERROR_DESCRIPTION("ERROR getRawData");

        //rawDataInfo.settings.Append(CameraSettingItem("exposure time", "ExposureUs", config.exposureTimeUs, "us"));

        // update capture info
        _captureInfo.imageHeight            = rawDataInfo.miLines;
        _captureInfo.imageWidth             = rawDataInfo.miCols;
        _captureInfo.imageOffsetX           = rawDataInfo.cropArea.x();
        _captureInfo.imageOffsetY           = rawDataInfo.cropArea.y();
        _captureInfo.exposureUs             = config.exposureTimeUs;
        _captureInfo.temperatureCpu         = rawDataInfo.settings.GetValue("Sensor");
        _captureInfo.temperatureMainBoard   = rawDataInfo.settings.GetValue("MainBoard");
        _captureInfo.temperatureSensor      = rawDataInfo.settings.GetValue("Cpu");

        // store information about the capture
        if(mDebugSettings.emulateCamera == false)
        {
            if(updateCaptureDate == true)
            {
                QDateTime timeStamp = QDateTime::currentDateTime();

                // _captureInfo.timeStamp = timeStamp;
                _captureInfo.timeStampDate = timeStamp.toString("dd/MM/yyyy");
                _captureInfo.timeStampTime = timeStamp.toString("hh:mm:ss");

                _captureInfo.timeStampString = timeStamp.toString("yyyyMMdd_hhmmss");
                _timeStampString_test = timeStamp.toString("yyyyMMdd_hhmmss");
            }
        }

        // retrieve current temperature
        LogInFile("mTempMonitor->GetTemperature");
        mTempMonitor->GetTemperature(_captureInfo.temperature);
    }

    LogInFile(QString("_CmdMeasure %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdExportRaw()
{
    LogInFile("_CmdExportRaw");

    ClassCommon::Error eError = ClassCommon::Error::Failed;

    // check that capture folder exists
    mInfo.capturePath = _CreateFolder(ConoscopeProcess::mSettings.capturePath);

    QString name = QString(FILE_NAME).arg(_timeStampString_test)
                                     .arg(RESOURCE->ToString(_measurementConfig.eFilter))
                                     .arg(RESOURCE->ToString(_measurementConfig.eNd))
                                     .arg(RESOURCE->ToString(_measurementConfig.eIris, true));

#ifdef FILE_NAME_FORMAT
    QMap<FileFormatKey_t, QString> fileNameFormatParams;

    fileNameFormatParams[FileFormatKey_t::TimeStamp] = _timeStampString_test;
    fileNameFormatParams[FileFormatKey_t::Filter]    = RESOURCE->ToString(_measurementConfig.eFilter);
    fileNameFormatParams[FileFormatKey_t::Nd]        = RESOURCE->ToString(_measurementConfig.eNd);
    fileNameFormatParams[FileFormatKey_t::Iris]      = RESOURCE->ToString(_measurementConfig.eIris, true);
    fileNameFormatParams[FileFormatKey_t::ExpoTime]  = QString("%1").arg(mInfo.exposureTimeUs);
    fileNameFormatParams[FileFormatKey_t::NbAcq]     = QString("%1").arg(mInfo.nbAcquisition);
    fileNameFormatParams[FileFormatKey_t::Height]    = QString("%1").arg(_captureInfo.imageHeight);
    fileNameFormatParams[FileFormatKey_t::Width]     = QString("%1").arg(_captureInfo.imageWidth);
    fileNameFormatParams[FileFormatKey_t::SatLevel]  = "";
    fileNameFormatParams[FileFormatKey_t::SatFlag]   = "";

    if(mInfo.AeEnable == true)
    {
        fileNameFormatParams[FileFormatKey_t::AeExpoGran] = QString("%1").arg(mInfo.AeExpoTimeGranularityUs);
    }

    name = FormatFileName(fileNameFormatParams);

    if(name == "")
    {
        name = QString(FILE_NAME).arg(_timeStampString_test)
                                 .arg(RESOURCE->ToString(_measurementConfig.eFilter))
                                 .arg(RESOURCE->ToString(_measurementConfig.eNd))
                                 .arg(RESOURCE->ToString(_measurementConfig.eIris, true));
    }
#endif

    // prepend and append part
    name = QString("%1%2%3").arg(CONVERT_TO_QSTRING(ConoscopeProcess::mSettings.fileNamePrepend))
                                     .arg(name)
                                     .arg(CONVERT_TO_QSTRING(ConoscopeProcess::mSettings.fileNameAppend));

    QString fileName = QString(RAW_FILE_NAME).arg(name);

    QString fileName_2 = _GetFileName(fileName, mInfo.capturePath, "jpg");

    fileName = _GetFileName(fileName, mInfo.capturePath, CAPTURE_EXTENSION);

    _CleanFileName(fileName);

    // output data
    _FillInfo(_measurementConfig, fileName);

    // json data

    // create object for json file
    QMap<QString, QMap<QString, QVariant>> settings;

    settings["Setup"]["sensorTemperature"] = (double)_measurementConfig.sensorTemperature;
#ifdef STORE_INDEX
    settings["Setup"]["filter"] = _measurementConfig.eFilter;
    settings["Setup"]["nd"]     = _measurementConfig.eNd;
    settings["Setup"]["iris"]   = _measurementConfig.eIris;
#else
    settings["Setup"]["filter"] = RESOURCE->ToString(_measurementConfig.eFilter);
    settings["Setup"]["nd"]     = RESOURCE->ToString(_measurementConfig.eNd);
    settings["Setup"]["iris"]   = RESOURCE->ToString(_measurementConfig.eIris);
#endif

    settings["Measure"]["ExposureTimeUs"] = mInfo.exposureTimeUs;
    settings["Measure"]["NbAcquisition"]  = mInfo.nbAcquisition;
    settings["Measure"]["BinningFactor"]  = mInfo.binningFactor;
    settings["Measure"]["TestPattern"]    = mInfo.bTestPattern;

#ifdef SATURATION_FLAG_RAW
    uint16_t saturationValue = 4095;
    int nbPixels = mInfo.height * mInfo.width;

    _GetSaturationFlag(saturationValue, nbPixels, (uint16_t*)_rawData.data(), mInfo.saturationFlag, mInfo.saturationLevel);

    settings["Measure"]["SaturationFlag"] = mInfo.saturationFlag;
    settings["Measure"]["SaturationLevel"] = mInfo.saturationLevel;
#endif

    if(mInfo.AeEnable == true)
    {
        settings["Measure"]["AeExposureTimeGranularityUs"] = mInfo.AeExpoTimeGranularityUs;
    }

    // save captured image
    if(_rawData.length() != 0)
    {
        char* pImage = (char*)_rawData.data();

        if((ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin) ||
           (ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin_jpg) )
        {
            eError = _WriteImageFile(fileName, pImage, _rawData.size(), _captureInfo, settings);
            _Log(QString("  store image in %1  %2").arg(fileName).arg(ClassCommon::ErrorToString(eError)));
        }

        if(ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin_jpg)
        {
            _SaveImage<uint16_t>(fileName_2, (uint16_t*)pImage, mInfo.height, mInfo.width);
        }
    }
    else
    {
        _Log(QString("  Error no data captured"));
    }

    LogInFile(QString("_CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdExportRaw(std::vector<uint16_t> &buffer)
{
    LogInFile("_CmdExportRaw");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // output data
    _FillInfo(_measurementConfig);

    // copy the data into the buffer
    int size = _rawData.size();
    buffer.resize(size / sizeof(uint16_t));
    memcpy(buffer.data(), _rawData.data(), size);

    if(eError == ClassCommon::Error::Ok)
    {
        // analyse data
        _AnalyseData<uint16_t>(_captureInfo.imageWidth, _captureInfo.imageHeight, buffer, mInfo.max);
        mInfo.min = 0;
    }

    LogInFile(QString("_CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

void ConoscopeProcess::_FillInfo(SetupConfig_t &setupConfig, QString fileName)
{
    // store capture file name
    mInfo.captureFileName = fileName;

    // store setup configuration
    mInfo.sensorTemperature = (double)setupConfig.sensorTemperature;
    mInfo.eFilter           = setupConfig.eFilter;
    mInfo.eNd               = setupConfig.eNd;
    mInfo.eIris             = setupConfig.eIris;

    mInfo.height = _captureInfo.imageHeight;
    mInfo.width  = _captureInfo.imageWidth;
}

ClassCommon::Error ConoscopeProcess::_CmdExportProcessed(ProcessingConfig_t &config)
{
    QString message;

    message.append("_CmdExportProcessed\n");
    message.append(QString("    bBiasCompensation       %1\n").arg(config.bBiasCompensation));
    message.append(QString("    bSensorDefectCorrection %1\n").arg(config.bSensorDefectCorrection));
    message.append(QString("    bSensorPrnuCorrection   %1\n").arg(config.bSensorPrnuCorrection));
    message.append(QString("    bLinearisation          %1\n").arg(config.bLinearisation));
    message.append(QString("    bFlatField              %1\n").arg(config.bFlatField));
    message.append(QString("    bAbsolute               %1").arg(config.bAbsolute));

    LogInFile(message);

    // check that capture folder exists
    QString capturePath = _CreateFolder(ConoscopeProcess::mSettings.capturePath);

    QString name = QString(FILE_NAME).arg(_timeStampString_test)
                                     .arg(RESOURCE->ToString(_measurementConfig.eFilter))
                                     .arg(RESOURCE->ToString(_measurementConfig.eNd))
                                     .arg(RESOURCE->ToString(_measurementConfig.eIris, true));

#ifdef FILE_NAME_FORMAT
    QMap<FileFormatKey_t, QString> fileNameFormatParams;

    fileNameFormatParams[FileFormatKey_t::TimeStamp] = _timeStampString_test;
    fileNameFormatParams[FileFormatKey_t::Filter]    = RESOURCE->ToString(_measurementConfig.eFilter);
    fileNameFormatParams[FileFormatKey_t::Nd]        = RESOURCE->ToString(_measurementConfig.eNd);
    fileNameFormatParams[FileFormatKey_t::Iris]      = RESOURCE->ToString(_measurementConfig.eIris, true);
    fileNameFormatParams[FileFormatKey_t::ExpoTime]  = QString("%1").arg(mInfo.exposureTimeUs);
    fileNameFormatParams[FileFormatKey_t::NbAcq]     = QString("%1").arg(mInfo.nbAcquisition);
    // fileNameFormatParams[FileFormatKey_t::Height]    = "";
    // fileNameFormatParams[FileFormatKey_t::Width]     = "";

    if(mInfo.AeEnable == true)
    {
        fileNameFormatParams[FileFormatKey_t::AeExpoGran] = QString("%1").arg(mInfo.AeExpoTimeGranularityUs);
    }

    name = FormatFileName(fileNameFormatParams);

    if(name == "")
    {
        name = QString(FILE_NAME).arg(_timeStampString_test)
                                 .arg(RESOURCE->ToString(_measurementConfig.eFilter))
                                 .arg(RESOURCE->ToString(_measurementConfig.eNd))
                                 .arg(RESOURCE->ToString(_measurementConfig.eIris, true));
    }
#endif

    name = QString("%1%2%3").arg(CONVERT_TO_QSTRING(ConoscopeProcess::mSettings.fileNamePrepend))
                                     .arg(name)
                                     .arg(CONVERT_TO_QSTRING(ConoscopeProcess::mSettings.fileNameAppend));

    QString fileName = QString(PROCESSED_FILE_NAME).arg(name);

    fileName = _GetFileName(fileName, capturePath, CAPTURE_EXTENSION);

    ClassCommon::Error eError = _Processed(_measurementConfig, config, fileName, false);

    LogInFile(QString("_CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, bool bSaveImage)
{
    QString message;

    message.append("_CmdExportProcessed\n");
    message.append(QString("    bBiasCompensation       %1\n").arg(config.bBiasCompensation));
    message.append(QString("    bSensorDefectCorrection %1\n").arg(config.bSensorDefectCorrection));
    message.append(QString("    bSensorPrnuCorrection   %1\n").arg(config.bSensorPrnuCorrection));
    message.append(QString("    bLinearisation          %1\n").arg(config.bLinearisation));
    message.append(QString("    bFlatField              %1\n").arg(config.bFlatField));
    message.append(QString("    bAbsolute               %1").arg(config.bAbsolute));

    LogInFile(message);

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if(bSaveImage == true)
    {
        eError = _CmdExportProcessed(config);
    }
    else
    {
        eError = _Processed(_measurementConfig, config);
    }

    buffer.resize((int)_klibData.size()/sizeof(uint16_t));
    memcpy(buffer.data(), _klibData.data(), _klibData.size());

#ifndef ANALYSE_SAT_LEVEL
    if(eError == ClassCommon::Error::Ok)
    {
        // analyse data
        _AnalyseData<int16_t>(mInfo.width, mInfo.height, buffer, mInfo.max);
        mInfo.min = 0;
    }
#endif

    LogInFile(QString("_CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_Processed(SetupConfig_t &setupConfig, ProcessingConfig_t &config, QString fileName, bool bAlwaysComputeKLib)
{
    LogInFile(QString("_Process (%1, %2, %3, %4, %5, %6)").arg(config.bBiasCompensation)
                                                          .arg(config.bSensorDefectCorrection)
                                                          .arg(config.bSensorPrnuCorrection)
                                                          .arg(config.bLinearisation)
                                                          .arg(config.bFlatField)
                                                          .arg(config.bAbsolute));

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    CaptureInfo_t  imgInfo;
    int16* inputData;

    // output data
    _FillInfo(_measurementConfig);

    // store capture file name
    mInfo.captureFileName = fileName;

    // store setup configuration
    mInfo.sensorTemperature = (double)setupConfig.sensorTemperature;
    mInfo.eFilter           = setupConfig.eFilter;
    mInfo.eNd               = setupConfig.eNd;
    mInfo.eIris             = setupConfig.eIris;

    // set default values, will be overwritten if required depending on the configuration
    mInfo.conversionFactorCompX = 0;
    mInfo.conversionFactorCompY = 0;
    mInfo.conversionFactorCompZ = 0;

    // create object for json file
    QMap<QString, QMap<QString, QVariant>> settings;

    settings["Setup"]["sensorTemperature"] = (double)_measurementConfig.sensorTemperature;

#ifdef STORE_INDEX
    settings["Setup"]["filter"] = setupConfig.eFilter;
    settings["Setup"]["nd"]     = setupConfig.eNd;
    settings["Setup"]["iris"]   = setupConfig.eIris;
#else
    settings["Setup"]["filter"] = RESOURCE->ToString(setupConfig.eFilter);
    settings["Setup"]["nd"]     = RESOURCE->ToString(setupConfig.eNd);
    settings["Setup"]["iris"]   = RESOURCE->ToString(setupConfig.eIris);
#endif

    settings["Measure"]["ExposureTimeUs"] = mInfo.exposureTimeUs;
    settings["Measure"]["NbAcquisition"]  = mInfo.nbAcquisition;
    settings["Measure"]["BinningFactor"]  = mInfo.binningFactor;
    settings["Measure"]["TestPattern"]    = mInfo.bTestPattern;

    if(mInfo.AeEnable == true)
    {
        settings["Measure"]["AeExposureTimeGranularityUs"] = mInfo.AeExpoTimeGranularityUs;
    }

    settings["Process"]["BiasCompensation"] = config.bBiasCompensation;
    settings["Process"]["SensorDefectCorrection"] = config.bSensorDefectCorrection;
    settings["Process"]["SensorPrnuCorrection"] = config.bSensorPrnuCorrection;
    settings["Process"]["Linearisation"] = config.bLinearisation;
    settings["Process"]["FlatField"] = config.bFlatField;
    settings["Process"]["Absolute"] = config.bAbsolute;

    // read cfg file
    ConfigContent_t cfgContent;
    QMap<ComposantType_t, float> colorCoef;

    // get needed cfg files depending on configuration
    NeededCfgFiles_t neededCfgFiles;
    neededCfgFiles = _GetNeededCfgFiles(config);

    // first read camera pipeline
    if(neededCfgFiles.bCameraCfg)
    {
        eError = _ReadCfgCameraPipeline(_captureInfo.cameraBoardSerialNumber);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = _GetCfg(setupConfig, cfgContent, colorCoef, neededCfgFiles);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        settings["CfgFile"]["camera"]        = mInfo.cameraCfgFileName.data;
        settings["CfgFile"]["opticalColumn"] = mInfo.opticalColumnCfgFileName.data;
        settings["CfgFile"]["flatFieldFile"] = mInfo.flatFieldFileName.data;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        settings["CfgFile"]["opticalColumnDate"]    = CONVERT_TO_QSTRING(cfgContent.calibrationSummary.date);
        settings["CfgFile"]["opticalColumnTime"]    = CONVERT_TO_QSTRING(cfgContent.calibrationSummary.time);
        settings["CfgFile"]["opticalColumnComment"] = CONVERT_TO_QSTRING(cfgContent.calibrationSummary.comment);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // use last captured image
        imgInfo.Clone(_captureInfo);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // make a copy of rawdata
        _inputData.resize(_rawData.size());
        memcpy(_inputData.data(), _rawData.data(), _rawData.size());

        inputData = (int16*) _inputData.data();

        Pipeline_RawDataParam param;

        param.imageSize.Set(imgInfo.imageWidth, imgInfo.imageHeight,
                            imgInfo.imageOffsetX, imgInfo.imageOffsetY);

        // step 1 - DEFECT PIXELS
        param.sensorDefectEnable = config.bSensorDefectCorrection;

        param.sensorDefects_correctionEnabled = cfgContent.cameraPipeline.sensorDefects.calibrationDone;

        int defectCount = (int)cfgContent.cameraPipeline.sensorDefects.pixels.size();

        for(int index = 0; index < defectCount; index ++)
        {
            Defect pix;
            pix.coord.x = cfgContent.cameraPipeline.sensorDefects.pixels[index].coord.x;
            pix.coord.y = cfgContent.cameraPipeline.sensorDefects.pixels[index].coord.y;
            pix.type    = cfgContent.cameraPipeline.sensorDefects.pixels[index].type;
            param.sensorDefects_pixels.push_back(pix);
        }

        // step 2 - BIAS
        param.bias_compensationEnabled = config.bBiasCompensation;

        param.bias_sensorSaturation = cfgContent.cameraPipeline.sensorSaturation.value;
        // lastOffset map is not used.
        // param.bias_sensorSaturation;
        // step 2 - BIAS - output
        //int16*                           lastOffSet;

        // step 3 - DarkImage
        param.darkMeasurementEnable = config.bBiasCompensation;;
    /*
        DarkMeasurementData              darkMeasurement;
        // step 3 - DarkImage current capture
        int                              recipe_usExposureTime;
        SensorTemperature                sensorTemperature;
        time_t                           timeStamp;
    */
        param.darkMeasurement.usExposureTime = imgInfo.exposureUs;
        param.darkMeasurement.timeStamp = 0;

        param.darkMeasurement.sensorTemperature.heatsink     = imgInfo.temperatureMainBoard;
        param.darkMeasurement.sensorTemperature.die.averaged = imgInfo.temperatureCpu;
        param.darkMeasurement.sensorTemperature.die.current  = imgInfo.temperatureCpu;

        param.darkMeasurement.dataSize = 0;

        // param.darkMeasurement.sensorTemperature

        // step 4 - PRNU
        param.prnuEnable = config.bSensorPrnuCorrection;

        param.prnuScaleFactor       = cfgContent.cameraPipeline.sensorPrnu.scaleFactor;
        param.prnuCorrectionEnabled = cfgContent.cameraPipeline.sensorPrnu.correctionEnabled;

        // convert PRNU data
        param.prnuData = &cfgContent.cameraPipeline.sensorPrnu.data;

        // param.prnuData              = cfgContent.cameraPipeline.sensorPrnu.data;
    /*
        float                            prnuScaleFactor;
        bool                             prnuCorrectionEnabled;
        std::vector<char>*               prnuData;
    */

        Pipeline_ResultRawDataParam resultParam;

        eError = mPipelineLib->CmdComputeRawData(inputData, &param, resultParam);

        _Log("  CmdComputeRawData");
        _Log(QString("    imageSize                    %1x%2").arg(resultParam.imageSize.width).arg(resultParam.imageSize.height));
        _Log(QString("    maxBinaryValue               %1").arg(resultParam.maxBinaryValue));
        _Log(QString("    saturationOccurs             %1").arg(resultParam.saturationOccurs));
        _Log(QString("    saturationScore              %1").arg(resultParam.saturationScore));

/*
        QString message;

        message.append(QString("    saturationOccurs             %1").arg(resultParam.saturationOccurs));
        message.append(QString("    saturationScore              %1").arg(resultParam.saturationScore));
        message.append(QString("    saturationFlag               %1").arg(resultParam.saturationFlag));
        message.append(QString("    saturationLevel              %1").arg(resultParam.saturationLevel));

        LogInFile(message);
*/

        LogInFile(QString("    saturationLevel %1 [%2]").arg(resultParam.saturationLevel)
                  .arg(resultParam.saturationFlag == true ? "sat" : "not sat"));

        // resultParam.darkOffset
        // resultParam.fullSensor;
        _Log(QString("    iDefects                     %1").arg(resultParam.iDefects));
        _Log(QString("    darkImageInfo.deltaTime      %1").arg(resultParam.darkImageInfo.deltaTime));
        _Log(QString("    darkImageInfo.deltaTemp      %1").arg(resultParam.darkImageInfo.deltaTemp));
        _Log(QString("    darkImageInfo.deltaBiasCount %1").arg(resultParam.darkImageInfo.deltaBiasCount));
        _Log(QString("    darkCurrentBiasValue         %1").arg(resultParam.darkCurrentBiasValue));
        _Log(QString("    timestamp                    %1").arg(resultParam.timeStamp));
        _Log(QString("    sensorTemp.die.avrg          %1").arg(resultParam.sensorTemperature.die.averaged));
        _Log(QString("    sensorTemp.die.curr          %1").arg(resultParam.sensorTemperature.die.current));
        _Log(QString("    sensorTemp.heatsink          %1").arg(resultParam.sensorTemperature.heatsink));

        settings["ProcessedData"]["height"] = mInfo.height;
        settings["ProcessedData"]["width"]  = mInfo.width;

        mInfo.saturationFlag = resultParam.saturationFlag;
        mInfo.saturationLevel = resultParam.saturationLevel;

#ifdef ANALYSE_SAT_LEVEL
        mInfo.max = (int)(resultParam.saturationLevel * (float)param.bias_sensorSaturation);

        if(mInfo.max > 4095)
        {
            mInfo.max = 4095;
        }
#endif

        settings["ProcessedData"]["saturationFlag"]  = mInfo.saturationFlag;
        settings["ProcessedData"]["saturationLevel"] = (double)mInfo.saturationLevel;

        // char* pImage = (char*)inputData;

// #define DEBUG_OUTPUT_SET_INPUT
#ifdef DEBUG_OUTPUT_SET_INPUT
        if(mDebugSettings.debugMode == true)
        {
            // set a pattern in input buffer
            uint16* pDebugImage = (uint16*)inputData;

            int debugWidth = imgInfo.imageWidth;
            int debugHeight = imgInfo.imageHeight;

            // fill output with dummy data
            int indexFill = 0;
            int lineValueOdd = 0;
            int lineValueEven = 0;

            int lineValueOffset;

            for(int indexLine = 0; indexLine < (debugHeight / 2); indexLine ++)
            {
                // lineValue = 0;

                for(int indexCol = 0; indexCol < (debugWidth / 2); indexCol ++)
                {
                    indexFill = indexLine * debugWidth                     + indexCol;
                    lineValueOffset = 200;
                    pDebugImage[indexFill] = lineValueOffset + lineValueOdd;

                    indexFill = indexLine * debugWidth                     + (debugWidth - indexCol - 1);
                    lineValueOffset = 400;
                    pDebugImage[indexFill] = lineValueOffset + lineValueEven;

                    indexFill = (debugHeight - indexLine - 1) * debugWidth + indexCol;
                    lineValueOffset = 400;
                    pDebugImage[indexFill] = lineValueOffset + lineValueEven;

                    indexFill = (debugHeight - indexLine - 1) * debugWidth + (debugWidth - indexCol - 1);
                    lineValueOffset = 200;
                    pDebugImage[indexFill] = lineValueOffset + lineValueOdd;

                    if((((debugHeight / 2) - indexLine) % 100) > 50)
                    {
                        if((((debugWidth / 2) - indexCol) % 100) > 50)
                        {
                            lineValueOdd  = 100;
                            lineValueEven = 200;
                        }
                        else
                        {
                            lineValueOdd  = 200;
                            lineValueEven = 100;
                        }
                    }
                    else
                    {
                        if((((debugWidth / 2) -indexCol) % 100) > 50)
                        {
                            lineValueOdd  = 200;
                            lineValueEven = 100;
                        }
                        else
                        {
                            lineValueOdd  = 100;
                            lineValueEven = 200;
                        }
                    }
                }
            }
        }

        /*
                ConoscopeProcess::_WriteImageFile(
                            fileName,
                            pImage,
                            _rawData.size());
        */
#endif
    }

#define NO_RESTRICTTOVIEWANGLE
    if(eError == ClassCommon::Error::Ok)
    {
        if((bAlwaysComputeKLib == true) ||
           (config.bLinearisation == true) ||
           (config.bFlatField == true) ||
           (config.bAbsolute == true))
        {
            Pipeline_KLibDataParam param;
            Pipeline_CalibrationParam calibration;

            /* parameter */
            param.imageSize.Set(imgInfo.imageWidth, imgInfo.imageHeight);

            // fill the active area params
            ImageConfiguration* imageConfiguration = ImageConfiguration::Get();
            param.activeArea.Set(imageConfiguration->active_width,
                                 imageConfiguration->active_height,
                                 imageConfiguration->active_horizontal_offset,
                                 imageConfiguration->active_vertical_offset);

            // PipelineCompute_Linearisation_t linearisation;
            // bool  conversionFactorCorrection;
            // bool  isCalibrated;
            // short sensorSaturationValue;
            // bool  applyFlatField;

            if(config.bLinearisation == true && config.bFlatField == true)
            {
                param.linearisation = Pipeline_Linearisation_MXAndFlatField;
            }
            else if(config.bLinearisation == true)
            {
                param.linearisation = Pipeline_Linearisation_MX;
            }
            else if(config.bFlatField == true)
            {
                // eError = ClassCommon::Error::InvalidParameter;
                param.linearisation = Pipeline_Linearisation_MXAndFlatField;
            }
            else
            {
                param.linearisation = Pipeline_Linearisation_None;
            }

            param.conversionFactorCorrection = config.bAbsolute;

            param.isCalibrated          = cfgContent.opticalColumnCalibration.flatField.isCalibrated;
            param.sensorSaturationValue = 0; // todo
            param.applyFlatField        = config.bFlatField;

            /* calibration data */
            calibration.sensorTemperatureDependancy_Enabled = cfgContent.opticalColumnCalibration.sensorTemperatureDependency.correctionEnable;
            calibration.sensorTemperatureDependancy_Slope   = cfgContent.opticalColumnCalibration.sensorTemperatureDependency.slope;

            calibration.captureArea_OpticalAxis.x = cfgContent.opticalColumnCalibration.captureArea.opticalAxis.X;
            calibration.captureArea_OpticalAxis.y = cfgContent.opticalColumnCalibration.captureArea.opticalAxis.Y;

            calibration.maximumIncidentAngle = cfgContent.opticalColumnCalibration.maximumIncidentAngle;
            calibration.calibratedDataRadius = cfgContent.opticalColumnCalibration.calibratedDataRadius;

            calibration.linearizationCoefficients.A1 = cfgContent.opticalColumnCalibration.linearizationCoefficients.A1;
            calibration.linearizationCoefficients.A3 = cfgContent.opticalColumnCalibration.linearizationCoefficients.A3;
            calibration.linearizationCoefficients.A5 = cfgContent.opticalColumnCalibration.linearizationCoefficients.A5;
            calibration.linearizationCoefficients.A7 = cfgContent.opticalColumnCalibration.linearizationCoefficients.A7;
            calibration.linearizationCoefficients.A9 = cfgContent.opticalColumnCalibration.linearizationCoefficients.A9;

            calibration.flatField = &cfgContent.opticalColumnCalibration.flatField.data;

            calibration.conversionFactor_Value = 1 / imgInfo.exposureUs;

            calibration.conversionFactor_SensorTemperature.die.averaged = imgInfo.temperatureSensor;
            calibration.conversionFactor_SensorTemperature.die.current = imgInfo.temperatureSensor;
            calibration.conversionFactor_SensorTemperature.heatsink = imgInfo.temperatureMainBoard;

            // allocate a buffer for output buffer
            // QByteArray  mKlibData;
            // std::vector<char> mKlibData;

            int klibDataSize = (calibration.calibratedDataRadius * 2 + 1) * (calibration.calibratedDataRadius * 2 + 1);
            _klibData.resize(klibDataSize * sizeof(int16));

            int16* klibData = (int16*) _klibData.data();

            // clean the output buffer
            memset(klibData, 0, klibDataSize * sizeof(int16));

// #define DEBUG_OUTPUT
#ifdef DEBUG_OUTPUT
            // fill output with dummy data
            int indexFill = 0;
            int lineValue = 0;

            for(int indexLine = 0; indexLine < (2 * calibration.calibratedDataRadius + 1); indexLine ++)
            {
                indexFill = indexLine * (calibration.calibratedDataRadius * 2 + 1);
                lineValue = 0;

                for(int indexCol = 0; indexCol < (2 * calibration.calibratedDataRadius + 1); indexCol ++)
                {
                    klibData[indexFill] = lineValue;

                    lineValue ++;
                    if(lineValue > 1000)
                    {
                        lineValue = 0;
                    }

                    indexFill ++;
                }
            }
#endif

            mInfo.height           = calibration.calibratedDataRadius * 2 + 1;
            mInfo.width            = calibration.calibratedDataRadius * 2 + 1;
            // mInfo.conversionFactor = calibration.conversionFactor_Value;

            // conversion factor is 1 / integration time
            mInfo.conversionFactorCompX = 1 / (double)mInfo.exposureTimeUs;
            mInfo.conversionFactorCompY = 1 / (double)mInfo.exposureTimeUs;
            mInfo.conversionFactorCompZ = 1 / (double)mInfo.exposureTimeUs;

            if(config.bAbsolute == true)
            {
                // apply the color coef
                mInfo.conversionFactorCompX *= colorCoef[ComposantType_X];
                mInfo.conversionFactorCompY *= colorCoef[ComposantType_Y];
                mInfo.conversionFactorCompZ *= colorCoef[ComposantType_Z];
            }

            settings["ProcessedData"]["conversionFactorCompX"] = mInfo.conversionFactorCompX;
            settings["ProcessedData"]["conversionFactorCompY"] = mInfo.conversionFactorCompY;
            settings["ProcessedData"]["conversionFactorCompZ"] = mInfo.conversionFactorCompZ;

            // imgInfo.imageHeight = mInfo.height;
            // imgInfo.imageWidth  = mInfo.width;
            imgInfo.imageHeight = 0;
            imgInfo.imageWidth  = 0;

            eError = mPipelineLib->CmdComputeKLibData(inputData, param, &calibration, klibData);

#ifdef FILE_NAME_FORMAT
            // don't know the size of the image before processing

            fileName.replace("<SatFlag>",   QString("%1").arg(mInfo.saturationFlag));
            fileName.replace("<SatLevel>",  QString("%1").arg(mInfo.saturationLevel, 5, 'f', 4, '0'));

            _CleanFileName(fileName);

            mInfo.captureFileName = fileName;
#endif

            // save data and associated json
            if((eError == ClassCommon::Error::Ok) &&
               (fileName.isEmpty() == false))
            {
                char* pKlibData = (char*)klibData;
                int klibDataSize = (int)_klibData.size();

                int cropHeight = mInfo.height;
                int cropWidth  = mInfo.width;

                if(ConoscopeProcess::mSettings.bUseRoi == false)
                {
                    settings["ROI"]["XLeft"]   = 0;
                    settings["ROI"]["XRight"]  = cropWidth;
                    settings["ROI"]["YTop"]    = 0;
                    settings["ROI"]["YBottom"] = cropHeight;
                }
                else
                {
                    cropHeight = ConoscopeProcess::mSettings.RoiYBottom - ConoscopeProcess::mSettings.RoiYTop;
                    cropWidth  = ConoscopeProcess::mSettings.RoiXRight - ConoscopeProcess::mSettings.RoiXLeft;

                    settings["ROI"]["XLeft"]   = ConoscopeProcess::mSettings.RoiXLeft;
                    settings["ROI"]["XRight"]  = ConoscopeProcess::mSettings.RoiXRight;
                    settings["ROI"]["YTop"]    = ConoscopeProcess::mSettings.RoiYTop;
                    settings["ROI"]["YBottom"] = ConoscopeProcess::mSettings.RoiYBottom;

                    int cropOffsetX = ConoscopeProcess::mSettings.RoiXLeft;
                    int cropOffsetY = ConoscopeProcess::mSettings.RoiYTop;

                    // crop the data to store
                    klibDataSize = cropHeight * cropWidth * sizeof(int16_t);
                    _klibDataCrop.resize(klibDataSize);

                    int16_t* pCropData = (int16_t*)_klibDataCrop.data();

                    for(int lineIndex = 0; lineIndex < cropHeight; lineIndex ++)
                    {
                        for(int rowIndex = 0; rowIndex < cropWidth; rowIndex ++)
                        {
                            pCropData[(lineIndex * cropWidth + rowIndex)] = klibData[(cropOffsetY + lineIndex) * mInfo.width + cropOffsetX + rowIndex];
                        }
                    }

                    pKlibData = (char*)_klibDataCrop.data();
                }

                settings["ProcessedData"]["height"] = cropHeight;
                settings["ProcessedData"]["width"]  = cropWidth;

#ifdef FILE_NAME_FORMAT
                // don't know the size of the image before processing
                fileName.replace("<Height>",    QString("%1").arg(cropHeight));
                fileName.replace("<Width>",     QString("%1").arg(cropWidth));
#endif

                if((ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin) ||
                   (ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin_jpg) )
                {
                    eError = _WriteImageFile(
                                fileName,
                                pKlibData,
                                klibDataSize,
                                imgInfo,
                                settings);

                    _Log(QString("  store image in %1  %2").arg(fileName).arg(ClassCommon::ErrorToString(eError)));
                }

                if(ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin_jpg)
                {
                    QString jpgFileName = fileName;
                    jpgFileName.replace(".bin", IMAGE_JPG_EXTENSION);

                    // _SaveImage<int16_t>(jpgFileName, (int16_t*)pKlibData, mInfo.height, mInfo.width);
                    _SaveImage<int16_t>(jpgFileName, (int16_t*)pKlibData, cropHeight, cropWidth);
                }

                if(ConoscopeProcess::mSettings.bUseRoi == true)
                {
                    // clear the crop buffer
                    _klibDataCrop.resize(0);
                    _klibDataCrop.clear();
                }
            }

            if(eError != ClassCommon::Error::Ok)
            {
                mErrorDescription = mPipelineLib->GetErrorDescription();
                _Log(QString("  Error no data processed (%1)").arg(mErrorDescription));
                LogInFile(QString("  Error no data processed (%1)").arg(mErrorDescription));
            }
        }
        else
        {
            char* pSaveData = _inputData.data();
            int saveDataSize = _inputData.size();

            imgInfo.imageHeight = 0;
            imgInfo.imageWidth  = 0;

#ifdef FILE_NAME_FORMAT
            // don't know the size of the image before processing
            fileName.replace("<Height>",    QString("%1").arg(mInfo.height));
            fileName.replace("<Width>",     QString("%1").arg(mInfo.width));

            fileName.replace("<SatFlag>",   QString("%1").arg(mInfo.saturationFlag));
            fileName.replace("<SatLevel>",   QString("%1").arg(mInfo.saturationLevel, 5, 'f', 4, '0'));

            _CleanFileName(fileName);

            mInfo.captureFileName = fileName;
#endif

            if((ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin) ||
               (ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin_jpg) )
            {
                eError = _WriteImageFile(
                            fileName,
                            pSaveData,
                            saveDataSize,
                            imgInfo,
                            settings);

                _Log(QString("  store image in %1  %2").arg(fileName).arg(ClassCommon::ErrorToString(eError)));
            }

            if(ConoscopeProcess::mSettings.exportFormat == ExportFormat_t::ExportFormat_bin_jpg)
            {
                QString jpgFileName = fileName;
                jpgFileName.replace(".bin", IMAGE_JPG_EXTENSION);

                _SaveImage<int16>(jpgFileName, (int16_t*)pSaveData, mInfo.height, mInfo.width);
            }
        }
    }

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdClose()
{
    LogInFile("_CmdClose");

    ClassCommon::Error eError;

#ifdef SET_TEMPERATURE
    // check that setup is not in progress
    _WaitForSetupIsDone();

    mTempMonitor->CmdReset();
#endif

    _Log("  Disconnect");
    eError = mCamera->Disconnect();

    LogInFile(QString("_CmdClose %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdReset()
{
    LogInFile("_CmdReset");

    ClassCommon::Error eError;

    // check that setup is not in progress
    _WaitForSetupIsDone();

    // _Log("  Disconnect");
    // eError = mCamera->Disconnect();

    // connect the camera
    QString cameraSerialNumber = "SN_0";
    QString cameraBoardSerialNumber = "CriticalLink_0";

    QString ipAddr = "TBD_ipAddr";
    QString port = "TBD_port";
    QString connectionConfig = "CXP6_X2";

    // create the camera instance if it is not already done
    // the camera is created during opening (and not during instanciation of the class)
    _CreateCamera();

    _Log("  Register");
    eError = mCamera->Register(cameraSerialNumber, cameraBoardSerialNumber);

    if(eError == ClassCommon::Error::Ok)
    {
        _Log("  ConfigureConnection");
        eError = mCamera->ConfigureConnection(ipAddr, port, connectionConfig);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        _Log("  Power Cycle");
        eError = mCamera->ConnectPowerCycle();
        _Log("  Camera is restarted");
    }
    else
    {
        _Log("  Disconnect failed");
    }

    if(eError == ClassCommon::Error::Ok)
    {
        _GetCameraInfo();
    }
    else
    {
        _Log("  open failed");
    }

    LogInFile(QString("_CmdReset %1").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdSetupDebug(SetupConfig_t &config)
{
    LogInFile("_CmdSetupDebug");

    // set configuration (with debug values)
    _setupConfig = config;

    return ClassCommon::Error::Ok;
}

struct DataHeader
{
    uint32_t head;      // header of the
    uint64_t lengthBytes;  // size of the payload in bytes
    qint64   timeStamp;   // time when the data are stored
    int      version;     // version of the data stored (if ever it changes)
};

#define DATA_HEADER 4112
#define DATA_VERSION 1

static QString cfgFilePath = "E:/TmpConoscope/03/cfg";
//static QString cfgFileName = "test.txt";
static QString cfgFileName = "AIRSHIP_19028816.cfg";
// static QString cfgFileName = "OpticalColumn.xml";

#define DATA_BUFFER_SIZE 1000
static char dataBuffer[DATA_BUFFER_SIZE];

// #define TEST_PREDEFINED_BUFFER
#define TEST_PREDEFINED_BUFFER_NAME "_TestBuffer.bin"

ClassCommon::Error ConoscopeProcess::_CmdCfgFileWrite()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("_CmdCfgFileWrite");

    mCfgFileStatus.eState = CfgFileState_Writing;
    mCfgFileStatus.progress = 0;

    qint64 timeElapsed;
    QElapsedTimer timer;
    timer.start();

    // create the folder if required
    mInfo.cfgPath = _CreateFolder(ConoscopeProcess::mSettings.cfgPath, _captureInfo.cameraBoardSerialNumber);
    mInfo.cfgFileName = CONVERT_TO_QSTRING(ConoscopeProcess::mSettingsI.cfgFileName);
    QString filePath = QDir::cleanPath(mInfo.cfgPath + QDir::separator() + mInfo.cfgFileName);

    // create the zip file
    eError = ClassCommon::ConvertError(CfgHelper::PackCameraFile(_captureInfo.cameraBoardSerialNumber,
                                                                 mInfo.cfgPath,
                                                                 filePath));

    QFileInfo cfgFileInfo;

    if(eError == ClassCommon::Error::Ok)
    {
        mCfgFileStatus.fileName = CONVERT_TO_STRING(filePath);

        LogInApp(QString("WriteCfgFile %1").arg(filePath));

        // get file information
        // QFileInfo cfgFileInfo(filePath);
        cfgFileInfo = QFileInfo(filePath);

        if(cfgFileInfo.exists() == false)
        {
            _Log(QString(" || file %1 does not exist").arg(filePath));
            eError = ClassCommon::Error::Failed;
        }
    }

    if(eError == ClassCommon::Error::Ok)
    {
        try {
            int dataSize = cfgFileInfo.size();
            _Log(QString(" || store %1 bytes").arg(dataSize));

            // fill the header
            DataHeader header;
            header.head        = DATA_HEADER;
            header.lengthBytes = dataSize;
            header.timeStamp   = QDateTime::currentSecsSinceEpoch();
            header.version     = DATA_VERSION;

            // set the camera in write mode
            mCamera->FileTransferOpen();
            mCamera->FileTransferWrite();

            // write the header
            mCamera->FileTransferWrite(&header, sizeof(header));

            // write the data
            int readSize;
            uint64_t remain = dataSize;
            int percent = 0;
            int percentPrevious = 0;
            int step = dataSize / 100;

            QFile cfgFile(filePath);

    #ifdef TEST_PREDEFINED_BUFFER
            int testValue = 1;

            if(cfgFile.open(QIODevice::WriteOnly))
    #else
            if(cfgFile.open(QIODevice::ReadOnly))
    #endif
            {
                do
                {
    #ifdef TEST_PREDEFINED_BUFFER
                    memset(dataBuffer, testValue, DATA_BUFFER_SIZE);
                    testValue = (testValue + 1) % 255;

                    if(remain < DATA_BUFFER_SIZE)
                    {
                        readSize = remain;
                    }
                    else
                    {
                        readSize = DATA_BUFFER_SIZE;
                    }

                    cfgFile.write(dataBuffer, readSize);
    #else
                    readSize = cfgFile.read(dataBuffer, DATA_BUFFER_SIZE);
    #endif
                    mCamera->FileTransferWrite(dataBuffer, readSize);

                    remain -= readSize;

                    percent = remain / step;
                    if(percent != percentPrevious)
                    {
                        percentPrevious = percent;
                        // _Log(QString(" ||   %1% - %2 bytes").arg(100 - percent, 3).arg(remain, 9));

                        mCfgFileStatus.progress = 100 - percent;
                        timeElapsed = timer.elapsed();
                        mCfgFileStatus.elapsedTime = timeElapsed;
                    }
                } while(remain > 0);
            }
            cfgFile.close();

            mCamera->FileTransferClose();

            timeElapsed = timer.elapsed();

            if(timeElapsed < 1000)
            {
                _Log(QString("    done in %1 ms").arg(timeElapsed));
            }
            else
            {
                _Log(QString("    done in %1 s").arg(timeElapsed / 1000));
            }
        }
        catch(...)
        {
            _Log(QString("FileTransfer error"));
            eError = ClassCommon::Error::Failed;
        }
    }

    QFile::remove(filePath);

    mCfgFileStatus.elapsedTime = timeElapsed;

    if(eError == ClassCommon::Error::Ok)
    {
        mCfgFileStatus.eState = CfgFileState_WriteDone;
    }
    else
    {
        mCfgFileStatus.eState = CfgFileState_WriteError;
    }

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdCfgFileRead()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LogInFile("_CmdCfgFileRead");

    mCfgFileStatus.eState = CfgFileState_Reading;
    mCfgFileStatus.progress = 0;

    mInfo.cfgFileName = CONVERT_TO_QSTRING(mSettingsI.cfgFileName);

    qint64 timeElapsed;
    QElapsedTimer timer;
    timer.start();

    // intialise data header
    DataHeader header;
    header.head = 0;
    header.lengthBytes = 0;
    header.timeStamp = 0;
    header.version = 0;

    try
    {
        // set the camera in read mode
        mCamera->FileTransferOpen();
        mCamera->FileTransferRead();

        // read the header
        mCamera->FileTransferRead(&header, sizeof(header));

        QDateTime timeStamp;
        timeStamp.setTime_t(header.timeStamp);

        if(header.head == DATA_HEADER)
        {
            _Log(QString("  header is ok"));
        }
        else
        {
            _Log(QString("  header is FAILED"));
        }

        _Log(QString("  payload size = %1 bytes").arg(header.lengthBytes));
        _Log(QString("  timestamp    = %1").arg(timeStamp.toString(Qt::SystemLocaleShortDate)));
        _Log(QString("  version      = %1").arg(header.version));

        // copy into file
        QString filePath = QDir::cleanPath(mInfo.cfgPath + QDir::separator() + mInfo.cfgFileName);
        _Log(QString(" || copy into file %1").arg(filePath));
        mCfgFileStatus.fileName = CONVERT_TO_STRING(filePath);

        uint64_t dataSize = header.lengthBytes;
        int copySize = DATA_BUFFER_SIZE;

        uint64_t remain = dataSize;
        int percent = 0;
        int percentPrevious = 0;
        int step = dataSize / 100;

        QFile cfgFile(filePath);

        mCfgFileStatus.progress = 0;
        mCfgFileStatus.elapsedTime = 0;

        if(cfgFile.open(QIODevice::WriteOnly))
        {
            do
            {
                if(remain < DATA_BUFFER_SIZE)
                {
                    copySize = remain;
                }

                mCamera->FileTransferRead(dataBuffer, copySize);
                cfgFile.write(dataBuffer, copySize);

                remain -= copySize;

                percent = remain / step;
                if(percent != percentPrevious)
                {
                    percentPrevious = percent;
                    // _Log(QString("      %1% - %2 bytes").arg(100 - percent, 3).arg(remain, 9));

                    mCfgFileStatus.progress = 100 - percent;
                    // timeElapsed = timer.elapsed();
                    // mCfgFileStatus.elapsedTime = timeElapsed;
                }

                mCfgFileStatus.elapsedTime = timer.elapsed();

            } while(remain > 0);

            cfgFile.close();
        }

        mCamera->FileTransferClose();

        timeElapsed = timer.elapsed();

        if(timeElapsed < 1000)
        {
            _Log(QString("    done in %1 ms").arg(timeElapsed));
        }
        else
        {
            _Log(QString("    done in %1 s").arg(timeElapsed / 1000));
        }

        mCfgFileStatus.elapsedTime = timeElapsed;

        if(eError == ClassCommon::Error::Ok)
        {
            mCfgFileStatus.eState = CfgFileState_ReadDone;
        }
        else
        {
            mCfgFileStatus.eState = CfgFileState_ReadError;
        }

        if(eError == ClassCommon::Error::Ok)
        {
            // unzip if this is a zip
            if(mSettingsI.cfgFileIsZip == true)
            {
                QString _fileName = QDir::cleanPath(filePath);
                QString _cfgPath = mInfo.cfgPath;

                QString cmdLine =
                        QString("powershell.exe -nologo -noprofile -command \"& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('%1', '%2'); }\"")
                        .arg(_fileName)
                        .arg(_cfgPath);

                QProcess::execute(cmdLine);
            }

            QFile::remove(filePath);
        }
    }
    catch(...)
    {
        _Log(QString("FileTransfer error"));
        mCfgFileStatus.eState = CfgFileState_ReadError;
        eError = ClassCommon::Error::Failed;
    }

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdCfgFileStatus(CfgFileStatus_t &status)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    status = mCfgFileStatus;

    return eError;
}

ClassCommon::Error ConoscopeProcess::_CmdConvertRaw(ConvertRaw_t &param)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // read back the file (and associated json)
    QString fileName = CONVERT_TO_QSTRING(param.fileName);
    QByteArray imgData;
    ImageInfoRead_t info;

    eError = _ReadImageFile(fileName, imgData, info);

    if(eError != ClassCommon::Error::Ok)
    {
        _Log(QString("    Error reading file %1").arg(fileName));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // save jpg file
        QString outputFileName = "";

        // extract current file name

        QFileInfo fileInfo(fileName);
        QString path = fileInfo.absolutePath();
        QString baseName = fileInfo.baseName();

#ifdef FILE_NAME_FORMAT
        QMap<FileFormatKey_t, QString> fileNameFormatParams;

        QStringList dateSplit = info.date.split("/");
        QStringList timeSplit = info.time.split(":");

        fileNameFormatParams[FileFormatKey_t::TimeStamp] = dateSplit[2] + dateSplit[1] + dateSplit[0] + "_" + timeSplit[0] + timeSplit[1] + timeSplit[2];
        fileNameFormatParams[FileFormatKey_t::Filter]   = RESOURCE->ToString(info.eFilter);
        fileNameFormatParams[FileFormatKey_t::Nd]       = RESOURCE->ToString(info.eNd);
        fileNameFormatParams[FileFormatKey_t::Iris]     = RESOURCE->ToString(info.eIris);
        fileNameFormatParams[FileFormatKey_t::ExpoTime] = QString("%1").arg(info.exposureUs);
        fileNameFormatParams[FileFormatKey_t::NbAcq]    = QString("%1").arg(info.nbAcquisition);
        fileNameFormatParams[FileFormatKey_t::Height]   = QString("%1").arg(info.imageHeight);
        fileNameFormatParams[FileFormatKey_t::Width]    = QString("%1").arg(info.imageWidth);

        fileNameFormatParams[FileFormatKey_t::SatFlag]  = QString("%1").arg(info.saturationFlag);
        fileNameFormatParams[FileFormatKey_t::SatLevel] = QString("%1").arg(info.saturationLevel);

        baseName = FormatFileName(fileNameFormatParams);

        if(baseName == "")
        {
            baseName = fileInfo.baseName();
        }

        outputFileName = path + "/" + baseName + IMAGE_JPG_EXTENSION;
#else
        outputFileName = path + "/" + baseName + IMAGE_JPG_EXTENSION;
#endif

        int imageHeight = 0;
        int imageWidth = 0;

        imageHeight = info.imageHeight;
        imageWidth = info.imageWidth;

        _CleanFileName(fileName);

        if(info.bProcessed == false)
        {
            eError = _SaveImage<uint16_t>(outputFileName, (uint16_t*)imgData.data(), imageHeight, imageWidth);
        }
        else
        {
            eError = _SaveImage<int16_t>(outputFileName, (int16_t*)imgData.data(), imageHeight, imageWidth);
        }

        if(eError != ClassCommon::Error::Ok)
        {
            LogInApp(QString("    Error saving file %1").arg(outputFileName));
        }
        else
        {
            LogInApp(QString("    Saved file %1").arg(outputFileName));
        }
    }

    return eError;
}

CameraInfo_t ConoscopeProcess::_OpeningInfo()
{
    LogInFile("_OpeningInfo");

    CameraInfo_t cameraFeature;

    ClassCommon::Error eError = ClassCommon::Error::Ok;
    Camera::CameraInfo_t cameraInfo;

    eError = mCamera->GetInfo(cameraInfo);

    if(eError == ClassCommon::Error::Ok)
    {
        _Log(QString("    info : %1").arg(cameraInfo.VendorName.data));
        _Log(QString("    info : %1").arg(cameraInfo.ModelName.data));
        _Log(QString("    info : %1").arg(cameraInfo.cameraBoardSerialNumber.data));

        for(int infoIndex = 0; infoIndex < cameraInfo.cameraSpecific.length(); infoIndex ++)
        {
            _Log(QString("    info : %1").arg(
                     QString::fromStdString(cameraInfo.cameraSpecific[infoIndex].value)
                     ));
        }

        if(cameraInfo.cameraBoardSerialNumber.valid == true)
        {
            _captureInfo.cameraBoardSerialNumber = cameraInfo.cameraBoardSerialNumber.data;
        }

        // store camera info
        if(cameraInfo.cameraBoardSerialNumber.valid == true)
        {
            cameraFeature.cameraBoardSerialNumber = cameraInfo.cameraBoardSerialNumber.data;
        }

        cameraFeature.cameraVersion.clear();

        for(int infoIndex = 0; infoIndex < cameraInfo.cameraSpecific.length(); )
        {
            cameraFeature.cameraVersion.append(QString::fromStdString(cameraInfo.cameraSpecific[infoIndex].value));

            if( ++infoIndex < cameraInfo.cameraSpecific.length())
            {

                cameraFeature.cameraVersion.append(" | ");
            }
        }
    }

    return cameraFeature;
}

ClassCommon::Error ConoscopeProcess::_WriteImageFile(
        QString filename,
        char *pImage,
        int imageSize,
        CaptureInfo_t& captureInfo,
        QMap<QString, QMap<QString, QVariant> > &settings,
        const QRect& fullImage,
        const QRect& zoneToSave)
{
    LogInFile("_WriteImageFile");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if((filename.isEmpty()) ||
       (pImage == nullptr))
    {
        eError = ClassCommon::Error::InvalidParameter;

        if(pImage == nullptr)
        {
            _Log("ERROR: Image is null");
        }
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = _WriteImageFile(filename,
                                  pImage,
                                  imageSize,
                                  fullImage,
                                  zoneToSave);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        _WriteImageInfo(filename, captureInfo, settings);
    }

    return eError;
}

ClassCommon::Error ConoscopeProcess::_WriteImageFile(
        QString filename,
        char* pImage,
        int imageSize,
        const QRect& fullImage,
        const QRect& zoneToSave)
{
    LogInFile("_WriteImageFile");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    qint64 writtenBytesCount = 0;
    qint64 bytesToCopy = 0;

    int bytesPerPixel = 0;
    int lineLength = 0;
    int lineOffset = 0;

    QString path = QFileInfo(filename).absolutePath();

    // create dir if it does not exists
    QDir dir(path);
    if (!dir.exists())
    {
        dir.mkpath(path);
    }

    // check parameters image size
    int fullImageSize = fullImage.height() * fullImage.width() * PIXEL_SIZE;

    if((imageSize < fullImageSize) ||
       ((zoneToSave.y() + (zoneToSave.height() / 2)) > fullImage.height()) ||
       ((zoneToSave.x() + (zoneToSave.width() / 2)) > fullImage.width()))
    {
        _Log("_WriteImageFile invalid parameters");
        eError = ClassCommon::Error::InvalidParameter;
    }

    if(filename.isEmpty())
    {
        // filename can not be empty
        _Log("_WriteImageFile invalid name - empty");
        eError = ClassCommon::Error::InvalidParameter;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        QFile filePointer(filename);

        if(!filePointer.open(QFile::WriteOnly))
        {
            _Log(QString("Failed to open file: %1").arg(filename));
            eError = ClassCommon::Error::Failed;
        }
        else
        {
            if(zoneToSave == QRect(0, 0, 0, 0))
            {
                // save the full image
                writtenBytesCount = filePointer.write(pImage, imageSize);
                bytesToCopy = imageSize;
            }
            else
            {
                // save a part of the image
                qint16* px = (qint16*)pImage;

                bytesPerPixel = sizeof(qint16);
                lineLength = zoneToSave.width() * bytesPerPixel;

                // offset of the first pixel
                lineOffset = zoneToSave.y() * fullImage.width() + zoneToSave.x();

                // copy each line
                for(int line = 0; line < zoneToSave.height(); line ++)
                {
                    writtenBytesCount += filePointer.write((char*)(&px[lineOffset]), lineLength);

                    lineOffset += fullImage.width();
                }

                bytesToCopy = zoneToSave.width() * zoneToSave.height() * bytesPerPixel;
            }

            filePointer.close();

            if (writtenBytesCount != bytesToCopy)
            {
                _Log(QString("Error writing file: %1").arg(filename));
                eError = ClassCommon::Error::Failed;
            }
        }
    }

    return eError;
}

void ConoscopeProcess::_WriteImageInfo(QString filePath,
                                       CaptureInfo_t &captureInfo,
                                       QMap<QString, QMap<QString, QVariant> > &settings)
{
    LogInFile("_WriteImageInfo");

    // should check is file exists
    if(!QFile(filePath).exists())
    {
        return;
    }

    // retrieve path and name
    QString path = QFileInfo(filePath).absolutePath();
    QString name = QFileInfo(filePath).fileName().section(".",0,0);
    QString fileName = QFileInfo(filePath).fileName();

    // json part
    QJsonObject softwareObject;
    softwareObject.insert("SwVersion", QString("%1").arg(VERSION_STR));
    softwareObject.insert("SwDate",    QString("%1").arg(QString(RELEASE_DATE)));

    // camera part
    QJsonObject cameraInfoObject;

    cameraInfoObject.insert("Width", captureInfo.imageWidth);
    cameraInfoObject.insert("Height", captureInfo.imageHeight);
    cameraInfoObject.insert("SerialNumber", captureInfo.cameraBoardSerialNumber);

    // configuration part
    QJsonObject measureObject;

    int exposureMs = 0;
    int numImages = 0;

    measureObject.insert("ExposureMs",   exposureMs);
    measureObject.insert("NumberImages", numImages);

    // measurement part
    QJsonObject infoObject;
    infoObject.insert("File",        fileName);
    infoObject.insert("Date",        captureInfo.timeStampDate);
    infoObject.insert("Time",        captureInfo.timeStampTime);
    infoObject.insert("Temperature", captureInfo.temperature);

    // record
    QJsonObject recordObject;
    recordObject.insert("Software",      softwareObject);
    recordObject.insert("Camera",        cameraInfoObject);
    recordObject.insert("Measure",       measureObject);
    recordObject.insert("Info",          infoObject);

    // settings
    QMapIterator<QString, QMap<QString, QVariant>> settingsIter(settings);
    while (settingsIter.hasNext())
    {
        settingsIter.next();

        QString itemObjectName = settingsIter.key();
        QMap<QString, QVariant> itemObjectValue = settingsIter.value();

        QMapIterator<QString, QVariant> itemIter(itemObjectValue);

        QJsonObject itemObject;

        while (itemIter.hasNext())
        {
            itemIter.next();

            QString itemName = itemIter.key();
            QVariant itemValue = itemIter.value();

            switch(itemValue.type())
            {
            case QVariant::Type::Int:
                itemObject.insert(itemName, itemValue.toInt());
                break;

            case QVariant::Type::Double:
                itemObject.insert(itemName, itemValue.toDouble());
                break;

            case QVariant::Type::Bool:
                itemObject.insert(itemName, itemValue.toBool());
                break;

            case QVariant::Type::String:
                itemObject.insert(itemName, itemValue.toString());
                break;
            }
        }

        recordObject.insert(itemObjectName, itemObject);
    }

    QJsonDocument doc(recordObject);

    QString jsonFileName = path + "/"+ name + IMAGE_INFO_EXTENSION;

    QFile jsonFile(jsonFileName);
    if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&jsonFile);
        out.setCodec("UTF-8");
        out << doc.toJson();
        jsonFile.close();
    }
}

ClassCommon::Error ConoscopeProcess::_ReadImageFile(
        QString acFilename,
        QByteArray& imgData,
        ImageInfoRead_t& info)
{
    LogInFile("_ReadImageFile");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    QFile ff(QString("%1").arg(acFilename));

    if(ff.exists() && ff.open(QFile::ReadOnly))
    {
        imgData = ff.readAll();
        ff.close();
    }
    else
    {
        //writeInfo(QString("Image does not exist or couldn't be open: %1").arg(acFilename));
        eError = ClassCommon::Error::Failed;

        LogInApp(QString("    ERROR Image does not exist or can't be open: %1").arg(acFilename));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        // read associated data
        struct Camera::RawDataInfo rawDataInfo;
        QMap<QString, QVariant> optionalInfo;

        QFileInfo fileInfo(acFilename);
        QString path = fileInfo.absolutePath();
        QString baseName = fileInfo.baseName();

        // retrieve the date from file name
/*
        QStringList fileNamePart = baseName.split("_");
        if(fileNamePart.count() >= 4)
        {
            info.timeStampString = QString("%1_%2").arg(fileNamePart[0]).arg(fileNamePart[1]);
        }
        else
        {
            info.timeStampString = "";
        }
*/
        QString rawFileName = path + "/" + baseName + IMAGE_INFO_EXTENSION;

        eError = _ReadImageInfo(rawFileName, info);

        if(eError != ClassCommon::Error::Ok)
        {
             LogInApp(QString("    ERROR reading json file %1").arg(rawFileName));
        }
    }

    return eError;
}

ClassCommon::Error ConoscopeProcess::_ReadImageInfo(QString filePath, ImageInfoRead_t &info)
{
    LogInFile("_ReadImageInfo");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    //QString jsonFileName = filePath + IMAGE_INFO_EXTENSION;
    QString jsonFileName = filePath;

    // should check is file exists
    if(!QFile(filePath).exists())
    {
        eError = ClassCommon::Error::Failed;
    }

    if(eError == ClassCommon::Error::Ok)
    {
        QString jsonData;
        QFile jsonFile(jsonFileName);
        if(jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            jsonData = (QString)jsonFile.readAll();
            jsonFile.close();

            // retrieve data from the json file

            QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonData.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            // QJsonArray jsonArray = jsonObject["properties"].toArray();

#ifdef REMOVED
            QJsonObject cameraInfoObject    = jsonObject["CameraInfo"].toObject();
            QJsonObject configurationObject = jsonObject["Configuration"].toObject();
            QJsonObject descriptionObject   = jsonObject["Description"].toObject();
            QJsonObject measurementObject   = jsonObject["Measurement"].toObject();

            QJsonObject temperatureObject = cameraInfoObject["Temperature"].toObject();

            info.temperature.Cpu       = (float)temperatureObject["Cpu"].toDouble();
            info.temperature.MainBoard = (float)temperatureObject["MainBoard"].toDouble();
            info.temperature.Sensor    = (float)temperatureObject["Sensor"].toDouble();

            info.exposureUs            = configurationObject["ExposureMs"].toInt();

            QJsonObject imageObject = cameraInfoObject["Image"].toObject();

            info.imageWidth  = (int)imageObject["Width"].toInt();;
            info.imageHeight = (int)imageObject["Height"].toInt();
#else
            QJsonObject cameraObject  = jsonObject["Camera"].toObject();
            QJsonObject measureObject = jsonObject["Measure"].toObject();
            QJsonObject setupObject   = jsonObject["Setup"].toObject();
            QJsonObject processObject = jsonObject["Process"].toObject();
            QJsonObject infoObject    = jsonObject["Info"].toObject();

            info.imageHeight = (int)cameraObject["Height"].toInt();
            info.imageWidth  = (int)cameraObject["Width"].toInt();

            info.exposureUs = (int)measureObject["ExposureTimeUs"].toInt();
            info.nbAcquisition = (int)measureObject["NbAcquisition"].toInt();

            if((info.imageHeight == 0) && (info.imageWidth == 0))
            {
                // check if height and width are in process part
                QJsonObject processedDataObject = jsonObject["ProcessedData"].toObject();

                info.imageHeight = processedDataObject["height"].toInt();
                info.imageWidth  = processedDataObject["width"].toInt();

                info.saturationFlag  = processedDataObject["saturationFlag"].toBool();
                info.saturationLevel = (float)processedDataObject["saturationLevel"].toDouble();
            }

            // retrieve setup information
            info.eFilter     = (Filter_t) RESOURCE->Convert(ConoscopeResource::ResourceType_Filter, setupObject["filter"].toString());
            info.eNd         = (Nd_t) RESOURCE->Convert(ConoscopeResource::ResourceType_Nd, setupObject["nd"].toString());
            info.eIris       = (IrisIndex_t) RESOURCE->Convert(ConoscopeResource::ResourceType_Iris, setupObject["iris"].toString());
            info.temperature = (float)setupObject["sensorTemperature"].toDouble();

            if(processObject.count() == 0)
            {
                info.bProcessed = false;
            }
            else
            {
                info.bProcessed = true;

                info.bBiasCompensation       = processObject["BiasCompensation"].toBool();
                info.bSensorDefectCorrection = processObject["SensorDefectCorrection"].toBool();
                info.bSensorPrnuCorrection   = processObject["SensorPrnuCorrection"].toBool();

                info.bLinearisation          = processObject["Linearisation"].toBool();
                info.bFlatField              = processObject["FlatField"].toBool();
                info.bAbsolute               = processObject["Absolute"].toBool();
            }

            info.date = infoObject["Date"].toString();
            info.time = infoObject["Time"].toString();
#endif
        }
    }

    return eError;
}

#include "CfgHelper.h"

ClassCommon::Error ConoscopeProcess::_GetCfg(
        SetupConfig_t& setupConfig,
        ConfigContent_t& configContent,
        QMap<ComposantType_t, float>& colorCoef,
        NeededCfgFiles_t neededCfgFiles)
{
    LogInFile("_GetCfg");

    ClassCommon::Error eError = ClassCommon::Error::Ok;

    _Log(QString("Read Cfg in %1").arg(mInfo.cfgPath));
    _Log(QString("     iris   %1").arg(setupConfig.eIris));
    _Log(QString("     filter %1").arg(setupConfig.eFilter));
    _Log(QString("     nb     %1").arg(setupConfig.eNd));

    CfgOutput output;

    bool res = CfgHelper::GetCfgFile(mInfo.cfgPath,
                                     setupConfig.eIris,
                                     setupConfig.eFilter,
                                     setupConfig.eNd,
                                     configContent,
                                     output,
                                     neededCfgFiles.bOpticalColumn,
                                     neededCfgFiles.bFlatField);

    mInfo.opticalColumnCfgFileName  = output.opticalColumnCfgFileName;

    mInfo.flatFieldFileName  = output.flatFieldFileName;

    mInfo.colorCoefCompX = output.colorCoefCompX.GetValue();
    mInfo.colorCoefCompY = output.colorCoefCompY.GetValue();
    mInfo.colorCoefCompZ = output.colorCoefCompZ.GetValue();

    mInfo.opticalColumnCfgDate     = CONVERT_TO_QSTRING(configContent.calibrationSummary.date);
    mInfo.opticalColumnCfgTime     = CONVERT_TO_QSTRING(configContent.calibrationSummary.time);
    mInfo.opticalColumnCfgComment  = CONVERT_TO_QSTRING(configContent.calibrationSummary.comment);

    colorCoef[ComposantType_X] = output.colorCoefCompX.GetValue();
    colorCoef[ComposantType_Y] = output.colorCoefCompY.GetValue();
    colorCoef[ComposantType_Z] = output.colorCoefCompZ.GetValue();

    if(res == false)
    {
        eError = ClassCommon::Error::Failed;
    }

    return eError;
}

void ConoscopeProcess::_Log(QString message)
{
    if(!message.isEmpty())
    {
        // emit OnLog(message);
    }
}

NeededCfgFiles_t ConoscopeProcess::_GetNeededCfgFiles(ProcessingConfig_t &config)
{
    NeededCfgFiles_t neededFiles;

    neededFiles.bCameraCfg = false;
    neededFiles.bOpticalColumn = false;
    neededFiles.bFlatField = false;

    if((config.bBiasCompensation == true) ||
       (config.bSensorDefectCorrection == true) ||
       (config.bSensorPrnuCorrection == true))
    {
        neededFiles.bCameraCfg = true;
    }

    if(config.bLinearisation == true)
    {
        neededFiles.bOpticalColumn = true;
    }

    if((config.bFlatField == true) ||
       (config.bAbsolute == true))
    {
        neededFiles.bFlatField = true;
    }

    return neededFiles;
}

ClassCommon::Error ConoscopeProcess::_ReadCfgCameraPipeline(QString sn)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    CfgOutput output;

    bool res = CfgHelper::ReadCfgCameraPipeline(sn, mInfo.cfgPath, output);

    if(res == false)
    {
        eError = ClassCommon::Error::Failed;
    }

    // for output purpose
    mInfo.cameraCfgFileName  = output.cameraCfgFileName;

    return eError;
}

bool ConoscopeProcess::_HasSetupChanged(float sensorTemperature)
{
    bool hasChanged = true;

    TemperatureMonitoringState_t eStatus;
    mTempMonitor->GetStatus(eStatus);

    if((eStatus == TemperatureMonitoringState_Processing) ||
       (eStatus == TemperatureMonitoringState_Locked))
    {
        if(sensorTemperature == mTempMonitor->GetTemperatureTarget())
        {
            hasChanged = false;
        }
    }

    return hasChanged;
}

#define WAIT_TIMEOUT_MS 10000

void ConoscopeProcess::_WaitForSetupIsDone()
{
    TemperatureMonitoringState_t eStatus;

    if(mTempMonitor != nullptr)
    {
        // retrieve monitoring status.
        // no processing must be on going to disconnect the camera
        mTempMonitor->GetStatus(eStatus);

        if(eStatus == TemperatureMonitoringState_Processing)
        {
            mTempMonitor->CmdCancel();

            // and wait for the process to be finished
            qint64 elapsed = 0;

            do{
                mTempMonitor->GetStatus(eStatus);

                // Wait(&res, TIME_QUANTA_MS);
                elapsed += TIME_QUANTA_MS;

                QThread::msleep(TIME_QUANTA_MS);
                QApplication::processEvents(QEventLoop::AllEvents, TIME_QUANTA_MS);

            } while((eStatus == TemperatureMonitoringState_Processing) && (elapsed < WAIT_TIMEOUT_MS));
        }
    }

    return;
}

#define WHEEL_MAX_VALUE 8

WheelStatus_t ConoscopeProcess::_GetWheelStatus()
{
    WheelStatus_t eWheelStatus = WheelStatus_Error;

    unsigned char wheelStatus = mDevices->CurrentBiWheelStatus();

    eWheelStatus = (WheelStatus_t) wheelStatus;

    return eWheelStatus;
}

WheelStatus_t ConoscopeProcess::_GetWheelStatus(
        Nd_t& eNdWheel,
        Filter_t& eFilterWheel)
{
    WheelStatus_t eWheelStatus = WheelStatus_Error;
    unsigned char NdIndex = 0;
    unsigned char FilterIndex = 0;

    eNdWheel     = Nd_Invalid;
    eFilterWheel = Filter_Invalid;

    eWheelStatus = _GetWheelStatus();

    if(eWheelStatus == WheelStatus_Success)
    {
        NdIndex = mDevices->BiWheelPosition(WheelTypeIndexMap[WheelType_Nd]);

        if(NdIndex > WHEEL_MAX_VALUE)
        {
            eWheelStatus = WheelStatus_Error;
        }
     }

    if(eWheelStatus == WheelStatus_Success)
    {
        FilterIndex = mDevices->BiWheelPosition(WheelTypeIndexMap[WheelType_Filter]);

        if(FilterIndex > WHEEL_MAX_VALUE)
        {
            eWheelStatus = WheelStatus_Error;
        }
    }

    if(eWheelStatus == WheelStatus_Success)
    {
        if(NdWheelRevertMap.count(NdIndex) == 0)
        {
            eNdWheel = Nd_Invalid;
            eWheelStatus = WheelStatus_Error;
        }
        else
        {
            eNdWheel = NdWheelRevertMap[NdIndex];
        }

        if(FilterWheelRevertMap.count(FilterIndex) == 0)
        {
            eFilterWheel = Filter_Invalid;
            eWheelStatus = WheelStatus_Error;
        }
        else
        {
            eFilterWheel = FilterWheelRevertMap[FilterIndex];
        }
    }

    return eWheelStatus;
}

QString ConoscopeProcess::_CreateFolder(std::string path, QString cameraSerialNumber)
{
    QString pathCreated;

    if(!cameraSerialNumber.isEmpty())
    {
        pathCreated = QDir::cleanPath(CONVERT_TO_QSTRING(path) + QDir::separator() + cameraSerialNumber);
    }
    else
    {
        pathCreated = QDir::cleanPath(CONVERT_TO_QSTRING(path));
    }

    QDir().mkpath(pathCreated);

    return pathCreated;
}

QString ConoscopeProcess::_GetFileName(QString name, QString path, QString extension)
{
    int index = 1;
    QString fileName;

    // create the file name template
    QString fileNameTemplate = QDir::cleanPath(path + QDir::separator() + name);

    do
    {
        fileName = QString("%1_%2.%3").arg(fileNameTemplate).arg(index ++).arg(extension);
        // check if the file exists
    } while(QFile::exists(fileName) == true);

    return fileName;
}

void ConoscopeProcess::_GetCameraInfo()
{
    CameraInfo_t cameraFeature = _OpeningInfo();

    QString sn1;
    QString sn2;

    mCamera->GetSerialNumber(sn1, sn2);

    mInfo.cameraSerialNumber = sn2;
    mInfo.cameraVersion = cameraFeature.cameraVersion;

    if(!sn2.isEmpty())
    {
        // check that cfg path exists
        mInfo.cfgPath = _CreateFolder(ConoscopeProcess::mSettings.cfgPath, mInfo.cameraSerialNumber);
    }
    else
    {
        mInfo.cfgPath = "";
    }
}

#ifdef AVERAGE_ANALYSE
#ifdef REMOVED
void ConoscopeProcess::_ListMin(QList<uint16_t>& list, int listSize, int16 value)
{
    int listLength = list.size();

    if(listLength == 0)
    {
        list.append(value);
    }
    else if(listLength == 1)
    {
        if(value > list[0])
        {
            list.push_back(value);
        }
        else
        {
            list.push_front(value);
        }
    }
    else
    {
        int16 valueLast = list.last();

        if((value < valueLast) || (listLength < listSize))
        {
            bool bInserted = false;
            int listIndex = 0;
            do
            {
                if((value >= list[listIndex]) &&
                   (value < list[listIndex + 1]))
                {
                    list.insert(listIndex, value);

                    bInserted = true;
                }

                listIndex ++;
            } while ((bInserted == false) && (listIndex < (listLength - 1)));

            if(bInserted == false)
            {
                list.push_back(value);
            }

            _ListTrim(list, listSize);
        }
    }
}
#endif

void ConoscopeProcess::_ListMax(QList<uint16_t>& list, int listSize, uint16_t value)
{
    // this function update the ordered max list
    // with the new value

    int listLength = list.size();

    uint16_t valueLast = list.last();

    if(value > valueLast)
    {
        if(listLength == 1)
        {
            list[0] = value;
        }
        else
        {
            // search for the insertion point
            bool bInserted = false;
            int listIndex = 0;

            // defines the range of the array
            int listIndexLeft = 0;
            int listIndexRight = listLength;

            listIndex = listLength / 2;

            int listIndexMove = 0;

            do {
                if (value == list[listIndex])
                {
                    bInserted = true;
                }
                else
                {
                    if ((value >= list[listIndex + 1]) &&
                        (value < list[listIndex]))
                    {
                        bInserted = true;
                    }
                    else
                    {
                        // next index
                        if (value > list[listIndex])
                        {
                            listIndexMove = (listIndex - listIndexLeft + 1) / 2;
                            listIndexRight = listIndex;

                            listIndex -= listIndexMove;
                        }
                        else
                        {
                            listIndexMove = (listIndexRight - listIndex) / 2;
                            listIndexLeft = listIndex;

                            listIndex += listIndexMove;
                        }
                    }
                }
            } while ((bInserted == false) && (listIndexMove != 0));

            if(bInserted == true)
            {
                list.insert(listIndex + 1, value);
            }
            else
            {
                list.push_front(value);
            }

            _ListTrim(list, listSize);
        }
    }
}

void ConoscopeProcess::_ListTrim(QList<uint16_t>& list, int listSize)
{
    int overload = list.length() - listSize;
    while(overload > 0)
    {
        list.removeLast();
        overload --;
    }
}

uint16_t ConoscopeProcess::_ListAverage(QList<uint16_t>& list)
{
    qint64 sum = 0;
    int listSize = list.size();

    for(int index = 0; index < listSize; index ++)
    {
        sum += list[index];
    }

    sum = sum / listSize;

    return (uint16_t) sum;
}
#endif

void ConoscopeProcess::_GetSomeInfo(SomeInfo_t& info)
{
    info.timeStampString = _timeStampString_test;
    info.capturePath     = _CreateFolder(ConoscopeProcess::mSettings.capturePath);

    info.cameraCfgFileName         = mInfo.cameraCfgFileName.GetValue();
    info.opticalColumnCfgFileName  = mInfo.opticalColumnCfgFileName.GetValue();
    info.flatFieldFileName         = mInfo.flatFieldFileName.GetValue();
    info.opticalColumnCfgDate      = mInfo.opticalColumnCfgDate.GetValue();
    info.opticalColumnCfgTime      = mInfo.opticalColumnCfgTime.GetValue();
    info.opticalColumnCfgComment   = mInfo.opticalColumnCfgComment.GetValue();
}

#ifdef SATURATION_FLAG_RAW
ClassCommon::Error ConoscopeProcess::_GetSaturationFlag(uint16_t saturationValue, int nbPixels, uint16_t* data, bool& saturationFlag, float& saturationLevel)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    uint16_t* pData = (uint16_t*)data;
    uint16_t dataValue;
    uint16_t pixelMax = 0;

    saturationFlag = false;

    for(int index = 0; index < nbPixels; index ++)
    {
         dataValue = pData[index];

         if(dataValue >= saturationValue)
         {
             // this part can be optimised
             saturationFlag = true;
         }

         if(dataValue > pixelMax)
         {
             pixelMax = dataValue;
         }
    }

    // calculate saturation level
    saturationLevel = (float)pixelMax / (float)saturationValue;

    return eError;
}
#endif

#ifdef FILE_NAME_FORMAT

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define FORMAT_REPLACE(a) if((params.contains(FileFormatKey_t::a) && (params[FileFormatKey_t::a] != ""))) { formatedFileName.replace(QString("<") + TOSTRING(a) + ">", params[FileFormatKey_t::a]); }

QString ConoscopeProcess::FormatFileName(QMap<FileFormatKey_t, QString> params)
{
    QString formatedFileName = CONVERT_TO_QSTRING(ConoscopeProcess::mSettings.exportFileNameFormat);

    bool bValid = true;

    if(formatedFileName != "")
    {
        FORMAT_REPLACE(TimeStamp)
        FORMAT_REPLACE(Filter)
        FORMAT_REPLACE(Nd)
        FORMAT_REPLACE(Iris)
        FORMAT_REPLACE(ExpoTime)
        FORMAT_REPLACE(NbAcq)
        FORMAT_REPLACE(Height)
        FORMAT_REPLACE(Width)
        FORMAT_REPLACE(SatLevel)
        FORMAT_REPLACE(SatFlag)
        FORMAT_REPLACE(AeExpoGran)
    }
    else
    {
        bValid = false;
    }

    if(bValid == false)
    {
        formatedFileName = "";
    }

    return formatedFileName;
}

void ConoscopeProcess::_CleanFileName(QString& fileName)
{
    // remove all the <...> things
    bool bEnd = false;
    do {
        int indexStart = fileName.indexOf("<");
        int indexEnd = fileName.indexOf(">");

        if((indexStart != -1) && (indexEnd != -1))
        {
            int length = indexEnd - indexStart + 1;
            fileName.replace(indexStart, length, "");
        }
        else
        {
            bEnd = true;
        }

    } while(bEnd == false);

    fileName.replace("<", "");
    fileName.replace(">", "");

    while(fileName.contains("__"))
    {
        fileName.replace("__", "_");
    }
}
#endif

void ConoscopeProcess::OnCameraLogInFile(QString header, QString message)
{
    RESOURCE->AppendLog(QString("%1 | ").arg("header", -20), message);
}
