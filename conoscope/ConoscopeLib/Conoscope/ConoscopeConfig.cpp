#include "ConoscopeConfig.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define JSON_INSERT(a, b) object##a.insert(TOSTRING(b), m##a.b)
#define JSON_INSERT_STR(a, b) object##a.insert(TOSTRING(b), CONVERT_TO_QSTRING(m##a.b))

#define SETUP_LABEL          "CmdSetup"
#define MEASURE_LABEL        "CmdMeasure"
#define PROCESSING_LABEL     "CmdProcessing"
#define DEBUG_SETTINGS_LABEL "DebugSettings"
#define SETTINGS_LABEL       "Settings"
#define SETTINGS_I_LABEL     "SettingsI"
#define CAPTURE_SEQUENCE     "CaptureSequence"

ConoscopeConfig::ConoscopeConfig(QObject *parent) : ClassCommon(parent),
    bSaveConfig(true)
{
    bool res = false;
    mFileName = ".\\config.json";

    _Default();

    if(QFile(mFileName).exists())
    {
        res = true;
        res = _Load();
    }

    if(res == false)
    {
        _Default();
        _Save();
    }
}

ConoscopeConfig::~ConoscopeConfig()
{
    
}

void ConoscopeConfig::SetConfig(SetupConfig_t &config)
{
    if(bSaveConfig == true)
    {
        mCmdSetupConfig = config;
        _Save();
    }
}

void ConoscopeConfig::GetConfig(SetupConfig_t &config)
{
    config = mCmdSetupConfig;
}

void ConoscopeConfig::SetConfig(MeasureConfig_t &config)
{
    if(bSaveConfig == true)
    {
        mCmdMeasureConfig = config;
        _Save();
    }
}

void ConoscopeConfig::SetConfig(MeasureConfigWithCropFactor_t &config)
{
    if(bSaveConfig == true)
    {
        mCmdMeasureConfig = CopyMeasureConfig(config);
        _Save();
    }
}

void ConoscopeConfig::GetConfig(MeasureConfig_t &config)
{
    config = mCmdMeasureConfig;
}

void ConoscopeConfig::SetConfig(ProcessingConfig_t &config)
{
    if(bSaveConfig == true)
    {
        mCmdProcessingConfig = config;
        _Save();
    }
}

void ConoscopeConfig::GetConfig(ProcessingConfig_t &config)
{
    config = mCmdProcessingConfig;
}

void ConoscopeConfig::SetConfig(ConoscopeDebugSettings_t& config)
{
    if(bSaveConfig == true)
    {
        mConoscopeDebugSettings = config;
        _Save();
    }
}

void ConoscopeConfig::GetConfig(ConoscopeDebugSettings_t& config)
{
    config = mConoscopeDebugSettings;
}

void ConoscopeConfig::SetConfig(ConoscopeSettings_t& config)
{
    if(bSaveConfig == true)
    {
        mConoscopeSettings = config;
        _Save();
    }
}

void ConoscopeConfig::GetConfig(ConoscopeSettings_t& config)
{
    config = mConoscopeSettings;
}

void ConoscopeConfig::GetConfig(ConoscopeSettingsI_t& config)
{
    config = mConoscopeSettingsI;
}

void ConoscopeConfig::GetConfig(CaptureSequenceConfig_t& config)
{
    config = mCaptureSequenceConfig;
}

void ConoscopeConfig::SetConfig(CaptureSequenceConfig_t& config)
{
    if(bSaveConfig == true)
    {
        mCaptureSequenceConfig = config;
        _Save();
    }
}

void ConoscopeConfig::_Default()
{
    // set default values

    mCmdSetupConfig.sensorTemperature          = 25.0;
    mCmdSetupConfig.eFilter                    = Filter_X;
    mCmdSetupConfig.eNd                        = Nd_0;
    mCmdSetupConfig.eIris                      = IrisIndex_2mm;

    mCmdMeasureConfig.exposureTimeUs           = 40000;
    mCmdMeasureConfig.nbAcquisition            = 1;
    mCmdMeasureConfig.binningFactor            = 0;
    mCmdMeasureConfig.bTestPattern             = false;

    mCmdProcessingConfig.bBiasCompensation            = true;
    mCmdProcessingConfig.bSensorDefectCorrection      = true;
    mCmdProcessingConfig.bSensorPrnuCorrection        = true;
    mCmdProcessingConfig.bLinearisation               = true;
    mCmdProcessingConfig.bFlatField                   = true;
    mCmdProcessingConfig.bAbsolute                    = true;

    mConoscopeDebugSettings.debugMode      = false;
    mConoscopeDebugSettings.emulateCamera = false;
    mConoscopeDebugSettings.dummyRawImagePath = "";
    mConoscopeDebugSettings.emulateWheel = false;

    mConoscopeSettings.cfgPath = "./Cfg";
    mConoscopeSettings.capturePath = "./Capture";

    mConoscopeSettings.fileNamePrepend = "";
    mConoscopeSettings.fileNameAppend = "";
    mConoscopeSettings.exportFileNameFormat = "";
    mConoscopeSettings.exportFormat = ExportFormat_t::ExportFormat_bin;
    mConoscopeSettings.AEMinExpoTimeUs = 10;
    mConoscopeSettings.AEMaxExpoTimeUs = 980000;
    mConoscopeSettings.AEExpoTimeGranularityUs = 1;

    mConoscopeSettings.AELevelPercent = 80.0;

    mConoscopeSettings.AEMeasAreaHeight = 200;
    mConoscopeSettings.AEMeasAreaWidth  = 288;
    mConoscopeSettings.AEMeasAreaX      = 3808;
    mConoscopeSettings.AEMeasAreaY      = 2902;

    mConoscopeSettings.bUseRoi = false;
    mConoscopeSettings.RoiXLeft = 0;
    mConoscopeSettings.RoiXRight = 6004;
    mConoscopeSettings.RoiYTop = 0;
    mConoscopeSettings.RoiYBottom = 6004;

    mConoscopeSettingsI.cfgFileName = "Cfg.zip";
    mConoscopeSettingsI.cfgFileIsZip = true;
    mConoscopeSettingsI.AEMaxNbPixel  = 5000;

    mCaptureSequenceConfig.sensorTemperature = 25;
    mCaptureSequenceConfig.bWaitForSensorTemperature = false;
    mCaptureSequenceConfig.eNd = Nd_0;
    mCaptureSequenceConfig.eIris = IrisIndex_2mm;

    mCaptureSequenceConfig.exposureTimeUs_FilterX  = 10000;
    mCaptureSequenceConfig.exposureTimeUs_FilterXz = 10000;
    mCaptureSequenceConfig.exposureTimeUs_FilterYa = 10000;
    mCaptureSequenceConfig.exposureTimeUs_FilterYb = 10000;
    mCaptureSequenceConfig.exposureTimeUs_FilterZ  = 10000;

    mCaptureSequenceConfig.nbAcquisition = 1;
    mCaptureSequenceConfig.bAutoExposure = true;
    mCaptureSequenceConfig.bUseExpoFile = false;
    mCaptureSequenceConfig.bSaveCapture = true;
}

#define ALIGN_VALUE(a, b) a - (a % b)
#define CHECK_MAX_VALUE(a, b) if(a > b) a = b

#define IMAGE_WIDTH  7920
#define IMAGE_HEIGHT 6004

#define IMAGE_SIZE   6001

bool ConoscopeConfig::_Load()
{
    bool res = true;

    QByteArray fileArray;

    QFile jsonFile(mFileName);

    // read back data
    if(jsonFile.open(QIODevice::ReadOnly))
    {
        fileArray = jsonFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(fileArray));
        QJsonObject object = loadDoc.object();

        QJsonObject setupConfigObject = object[SETUP_LABEL].toObject();
        QJsonObject measureConfigObject = object[MEASURE_LABEL].toObject();
        QJsonObject processingConfigObject = object[PROCESSING_LABEL].toObject();
        QJsonObject conoscopeDebugSettingsObject = object[DEBUG_SETTINGS_LABEL].toObject();
        QJsonObject conoscopeSettingsObject = object[SETTINGS_LABEL].toObject();
        QJsonObject conoscopeSettingsIObject = object[SETTINGS_I_LABEL].toObject();
        QJsonObject captureSequenceConfigObject = object[CAPTURE_SEQUENCE].toObject();

        int count = 0;
        count += setupConfigObject.count();
        count += measureConfigObject.count();
        count += processingConfigObject.count();
        count += conoscopeDebugSettingsObject.count();
        count += conoscopeSettingsObject.count();
        count += conoscopeSettingsIObject.count();
        count += captureSequenceConfigObject.count();

        int itemCountCheck = 53;

        if(count != itemCountCheck)
        {
            res = false;
        }

        if(res == true)
        {
            mCmdSetupConfig.sensorTemperature   = (float)setupConfigObject["sensorTemperature"].toDouble();
            mCmdSetupConfig.eFilter             = (Filter_t)setupConfigObject["eFilter"].toInt();
            mCmdSetupConfig.eNd                 = (Nd_t)setupConfigObject["eNd"].toInt();
            mCmdSetupConfig.eIris               = (IrisIndex_t)setupConfigObject["eIris"].toInt();

            mCmdMeasureConfig.exposureTimeUs    = measureConfigObject["exposureTimeUs"].toInt();
            mCmdMeasureConfig.nbAcquisition     = measureConfigObject["nbAcquisition"].toInt();
            mCmdMeasureConfig.binningFactor     = measureConfigObject["binningFactor"].toInt();
            mCmdMeasureConfig.bTestPattern      = measureConfigObject["bTestPattern"].toBool();

            mCmdProcessingConfig.bBiasCompensation            = processingConfigObject["bBiasCompensation"].toBool();
            mCmdProcessingConfig.bSensorDefectCorrection      = processingConfigObject["bSensorDefectCorrection"].toBool();
            mCmdProcessingConfig.bSensorPrnuCorrection        = processingConfigObject["bSensorPrnuCorrection"].toBool();
            mCmdProcessingConfig.bLinearisation               = processingConfigObject["bLinearisation"].toBool();
            mCmdProcessingConfig.bFlatField                   = processingConfigObject["bFlatField"].toBool();
            mCmdProcessingConfig.bAbsolute                    = processingConfigObject["bAbsolute"].toBool();

            mConoscopeDebugSettings.debugMode          = conoscopeDebugSettingsObject["debugMode"].toBool();
            mConoscopeDebugSettings.emulateCamera      = conoscopeDebugSettingsObject["emulateCamera"].toBool();
            mConoscopeDebugSettings.dummyRawImagePath  = CONVERT_TO_STRING(conoscopeDebugSettingsObject["dummyRawImagePath"].toString());
            mConoscopeDebugSettings.emulateWheel       = conoscopeDebugSettingsObject["emulateWheel"].toBool();

            mConoscopeSettings.cfgPath                 = CONVERT_TO_STRING(conoscopeSettingsObject["cfgPath"].toString());
            mConoscopeSettings.capturePath             = CONVERT_TO_STRING(conoscopeSettingsObject["capturePath"].toString());
            mConoscopeSettings.fileNamePrepend         = CONVERT_TO_STRING(conoscopeSettingsObject["fileNamePrepend"].toString());
            mConoscopeSettings.fileNameAppend          = CONVERT_TO_STRING(conoscopeSettingsObject["fileNameAppend"].toString());
            mConoscopeSettings.exportFileNameFormat    = CONVERT_TO_STRING(conoscopeSettingsObject["exportFileNameFormat"].toString());
            mConoscopeSettings.exportFormat            = (ExportFormat_t)conoscopeSettingsObject["exportFormat"].toInt();
            mConoscopeSettings.AEMinExpoTimeUs         = conoscopeSettingsObject["AEMinExpoTimeUs"].toInt();
            _CheckThreshold<int>(mConoscopeSettings.AEMinExpoTimeUs, 10, 985000);
            mConoscopeSettings.AEMaxExpoTimeUs         = conoscopeSettingsObject["AEMaxExpoTimeUs"].toInt();
            _CheckThreshold<int>(mConoscopeSettings.AEMaxExpoTimeUs, 10, 985000);

            mConoscopeSettings.AEExpoTimeGranularityUs = conoscopeSettingsObject["AEExpoTimeGranularityUs"].toInt();
            _CheckThreshold<int>(mConoscopeSettings.AEExpoTimeGranularityUs, 1, 985000);

            mConoscopeSettings.AELevelPercent          = (float)conoscopeSettingsObject["AELevelPercent"].toDouble();
            _CheckThreshold<float>(mConoscopeSettings.AELevelPercent, 0, 100);

            mConoscopeSettings.AEMeasAreaHeight        = conoscopeSettingsObject["AEMeasAreaHeight"].toInt();
            mConoscopeSettings.AEMeasAreaWidth         = conoscopeSettingsObject["AEMeasAreaWidth"].toInt();
            mConoscopeSettings.AEMeasAreaX             = conoscopeSettingsObject["AEMeasAreaX"].toInt();
            mConoscopeSettings.AEMeasAreaY             = conoscopeSettingsObject["AEMeasAreaY"].toInt();

            // check cropping configuration is matching alignment
            mConoscopeSettings.AEMeasAreaHeight        = ALIGN_VALUE(mConoscopeSettings.AEMeasAreaHeight,   4);
            mConoscopeSettings.AEMeasAreaWidth         = ALIGN_VALUE(mConoscopeSettings.AEMeasAreaWidth,   16);
            mConoscopeSettings.AEMeasAreaX             = ALIGN_VALUE(mConoscopeSettings.AEMeasAreaX,       16);
            mConoscopeSettings.AEMeasAreaY             = ALIGN_VALUE(mConoscopeSettings.AEMeasAreaY,        2);

            CHECK_MAX_VALUE(mConoscopeSettings.AEMeasAreaWidth, IMAGE_WIDTH);
            CHECK_MAX_VALUE(mConoscopeSettings.AEMeasAreaHeight, IMAGE_HEIGHT);

            // change image size if it does not fit into the sensor
            if(mConoscopeSettings.AEMeasAreaX + mConoscopeSettings.AEMeasAreaWidth > IMAGE_WIDTH)
            {
                mConoscopeSettings.AEMeasAreaWidth = IMAGE_WIDTH - mConoscopeSettings.AEMeasAreaX;
            }

            if(mConoscopeSettings.AEMeasAreaY + mConoscopeSettings.AEMeasAreaHeight > IMAGE_HEIGHT)
            {
                mConoscopeSettings.AEMeasAreaHeight = IMAGE_HEIGHT - mConoscopeSettings.AEMeasAreaY;
            }

            mConoscopeSettings.bUseRoi    = conoscopeSettingsObject["bUseRoi"].toBool();
            mConoscopeSettings.RoiXLeft   = conoscopeSettingsObject["RoiXLeft"].toInt();
            mConoscopeSettings.RoiXRight  = conoscopeSettingsObject["RoiXRight"].toInt();
            mConoscopeSettings.RoiYTop    = conoscopeSettingsObject["RoiYTop"].toInt();
            mConoscopeSettings.RoiYBottom = conoscopeSettingsObject["RoiYBottom"].toInt();

            // check ROI settings
            CHECK_MAX_VALUE(mConoscopeSettings.RoiXLeft, IMAGE_SIZE);
            CHECK_MAX_VALUE(mConoscopeSettings.RoiXRight, IMAGE_SIZE);
            CHECK_MAX_VALUE(mConoscopeSettings.RoiYTop, IMAGE_SIZE);
            CHECK_MAX_VALUE(mConoscopeSettings.RoiYBottom, IMAGE_SIZE);

            if(mConoscopeSettings.RoiXRight < mConoscopeSettings.RoiXLeft)
            {
                mConoscopeSettings.RoiXRight = mConoscopeSettings.RoiXLeft;
            }

            if(mConoscopeSettings.RoiYBottom < mConoscopeSettings.RoiYTop)
            {
                mConoscopeSettings.RoiYBottom = mConoscopeSettings.RoiYTop;
            }

            mConoscopeSettingsI.cfgFileName            = CONVERT_TO_STRING(conoscopeSettingsIObject["cfgFileName"].toString());
            mConoscopeSettingsI.cfgFileIsZip           = conoscopeSettingsIObject["cfgFileIsZip"].toBool();
            mConoscopeSettingsI.AEMaxNbPixel           = conoscopeSettingsIObject["captureSequenceMaxNbPixel"].toInt();

            mCaptureSequenceConfig.sensorTemperature         = captureSequenceConfigObject["sensorTemperature"].toDouble();
            mCaptureSequenceConfig.bWaitForSensorTemperature = captureSequenceConfigObject["bWaitForSensorTemperature"].toBool();
            mCaptureSequenceConfig.eNd                       = (Nd_t)captureSequenceConfigObject["eNd"].toInt();
            mCaptureSequenceConfig.eIris                     = (IrisIndex_t)captureSequenceConfigObject["eIris"].toInt();

            mCaptureSequenceConfig.exposureTimeUs_FilterX  = captureSequenceConfigObject["exposureTimeUs_FilterX"].toInt();;
            mCaptureSequenceConfig.exposureTimeUs_FilterXz = captureSequenceConfigObject["exposureTimeUs_FilterXz"].toInt();;
            mCaptureSequenceConfig.exposureTimeUs_FilterYa = captureSequenceConfigObject["exposureTimeUs_FilterYa"].toInt();;
            mCaptureSequenceConfig.exposureTimeUs_FilterYb = captureSequenceConfigObject["exposureTimeUs_FilterYb"].toInt();;
            mCaptureSequenceConfig.exposureTimeUs_FilterZ  = captureSequenceConfigObject["exposureTimeUs_FilterZ"].toInt();;

            _CheckThreshold<int>(mCaptureSequenceConfig.exposureTimeUs_FilterX,  10, 985000);
            _CheckThreshold<int>(mCaptureSequenceConfig.exposureTimeUs_FilterXz, 10, 985000);
            _CheckThreshold<int>(mCaptureSequenceConfig.exposureTimeUs_FilterYa, 10, 985000);
            _CheckThreshold<int>(mCaptureSequenceConfig.exposureTimeUs_FilterYb, 10, 985000);
            _CheckThreshold<int>(mCaptureSequenceConfig.exposureTimeUs_FilterZ,  10, 985000);

            mCaptureSequenceConfig.nbAcquisition             = captureSequenceConfigObject["nbAcquisition"].toInt();
            mCaptureSequenceConfig.bAutoExposure             = captureSequenceConfigObject["bAutoExposure"].toBool();
            mCaptureSequenceConfig.bUseExpoFile              = captureSequenceConfigObject["bUseExpoFile"].toBool();
            mCaptureSequenceConfig.bSaveCapture              = captureSequenceConfigObject["bSaveCapture"].toBool();
        }
    }

    return res;
}

void ConoscopeConfig::_Save()
{
    QJsonObject objectCmdSetupConfig;

    JSON_INSERT(CmdSetupConfig, sensorTemperature);
    JSON_INSERT(CmdSetupConfig, eFilter);
    JSON_INSERT(CmdSetupConfig, eNd);
    JSON_INSERT(CmdSetupConfig, eIris);

    QJsonObject objectCmdMeasureConfig;

    JSON_INSERT(CmdMeasureConfig, exposureTimeUs);
    JSON_INSERT(CmdMeasureConfig, nbAcquisition);
    JSON_INSERT(CmdMeasureConfig, binningFactor);
    JSON_INSERT(CmdMeasureConfig, bTestPattern);

    QJsonObject objectCmdProcessingConfig;

    JSON_INSERT(CmdProcessingConfig, bBiasCompensation);
    JSON_INSERT(CmdProcessingConfig, bSensorDefectCorrection);
    JSON_INSERT(CmdProcessingConfig, bSensorPrnuCorrection);
    JSON_INSERT(CmdProcessingConfig, bLinearisation);
    JSON_INSERT(CmdProcessingConfig, bFlatField);
    JSON_INSERT(CmdProcessingConfig, bAbsolute);

    QJsonObject objectConoscopeDebugSettings;

    JSON_INSERT(ConoscopeDebugSettings, debugMode);
    JSON_INSERT(ConoscopeDebugSettings, emulateCamera);
    JSON_INSERT_STR(ConoscopeDebugSettings, dummyRawImagePath);
    JSON_INSERT(ConoscopeDebugSettings, emulateWheel);

    QJsonObject objectConoscopeSettings;

    JSON_INSERT_STR(ConoscopeSettings, cfgPath);
    JSON_INSERT_STR(ConoscopeSettings, capturePath);

    JSON_INSERT_STR(ConoscopeSettings, fileNamePrepend);
    JSON_INSERT_STR(ConoscopeSettings, fileNameAppend);
    JSON_INSERT_STR(ConoscopeSettings, exportFileNameFormat);
    JSON_INSERT(ConoscopeSettings, exportFormat);
    JSON_INSERT(ConoscopeSettings, AEMinExpoTimeUs);
    JSON_INSERT(ConoscopeSettings, AEMaxExpoTimeUs);
    JSON_INSERT(ConoscopeSettings, AEExpoTimeGranularityUs);
    JSON_INSERT(ConoscopeSettings, AELevelPercent);

    JSON_INSERT(ConoscopeSettings, AEMeasAreaHeight);
    JSON_INSERT(ConoscopeSettings, AEMeasAreaWidth);
    JSON_INSERT(ConoscopeSettings, AEMeasAreaX);
    JSON_INSERT(ConoscopeSettings, AEMeasAreaY);

    JSON_INSERT(ConoscopeSettings, bUseRoi);
    JSON_INSERT(ConoscopeSettings, RoiXLeft);
    JSON_INSERT(ConoscopeSettings, RoiXRight);
    JSON_INSERT(ConoscopeSettings, RoiYTop);
    JSON_INSERT(ConoscopeSettings, RoiYBottom);

    QJsonObject objectConoscopeSettingsI;

    JSON_INSERT_STR(ConoscopeSettingsI, cfgFileName);
    JSON_INSERT(ConoscopeSettingsI, cfgFileIsZip);
    JSON_INSERT(ConoscopeSettingsI, AEMaxNbPixel);

    QJsonObject objectCaptureSequenceConfig;

    JSON_INSERT(CaptureSequenceConfig, sensorTemperature);
    JSON_INSERT(CaptureSequenceConfig, bWaitForSensorTemperature);
    JSON_INSERT(CaptureSequenceConfig, eNd);
    JSON_INSERT(CaptureSequenceConfig, eIris);

    JSON_INSERT(CaptureSequenceConfig, exposureTimeUs_FilterX);
    JSON_INSERT(CaptureSequenceConfig, exposureTimeUs_FilterXz);
    JSON_INSERT(CaptureSequenceConfig, exposureTimeUs_FilterYa);
    JSON_INSERT(CaptureSequenceConfig, exposureTimeUs_FilterYb);
    JSON_INSERT(CaptureSequenceConfig, exposureTimeUs_FilterZ);

    JSON_INSERT(CaptureSequenceConfig, nbAcquisition);
    JSON_INSERT(CaptureSequenceConfig, bAutoExposure);
    JSON_INSERT(CaptureSequenceConfig, bUseExpoFile);
    JSON_INSERT(CaptureSequenceConfig, bSaveCapture);

    // record
    QJsonObject recordObject;
    recordObject.insert(SETUP_LABEL,          objectCmdSetupConfig);
    recordObject.insert(MEASURE_LABEL,        objectCmdMeasureConfig);
    recordObject.insert(PROCESSING_LABEL,     objectCmdProcessingConfig);
    recordObject.insert(DEBUG_SETTINGS_LABEL, objectConoscopeDebugSettings);
    recordObject.insert(SETTINGS_LABEL,       objectConoscopeSettings);
    recordObject.insert(SETTINGS_I_LABEL,     objectConoscopeSettingsI);
    recordObject.insert(CAPTURE_SEQUENCE,     objectCaptureSequenceConfig);

    QJsonDocument doc(recordObject);

    QFile jsonFile(mFileName);
    if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&jsonFile);
        out.setCodec("UTF-8");
        out << doc.toJson();
        jsonFile.close();
    }
}
