#include "appConfig.h"

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

#define SETUP_APP          "Application"

AppConfig::AppConfig(QObject *parent) : ClassCommon(parent)
{
    bool res = false;
    mFileName = ".\\demo.json";

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

AppConfig::~AppConfig()
{
    _Save();
}

void AppConfig::SetConfig(AppConfig_t &config)
{
    mAppConfig = config;

    _Save();
}

void AppConfig::GetConfig(AppConfig_t &config)
{
    config = mAppConfig;
}

void AppConfig::_Default()
{
    // set default values
    mAppConfig.bAdmin = false;
    mAppConfig.bStreamProcessedData = false;
    mAppConfig.autoExposure = true;

    QList<LogMask_t> logMasks = {LogMask_State,
                                 LogMask_StateMachine,
                                 LogMask_Worker};

    mAppConfig.mLogMasks = LogMask(logMasks);
    mAppConfig.enableWarningMessage = true;
}

bool AppConfig::_Load()
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

        QJsonObject objectAppConfig = object[SETUP_APP].toObject();;

        int count = 0;
        count += objectAppConfig.count();

        int itemCountCheck = 4;

        if(count < itemCountCheck)
        {
            res = false;
        }

        if(res == true)
        {
            mAppConfig.bAdmin = objectAppConfig["bAdmin"].toBool();
            mAppConfig.bStreamProcessedData = objectAppConfig["bStreamProcessedData"].toBool();
            mAppConfig.mLogMasks = LogMask(objectAppConfig["LogMasks"].toArray());
            mAppConfig.autoExposure = objectAppConfig["autoExposure"].toBool();

            if(objectAppConfig.contains("enableWarningMessage") == true)
            {
                mAppConfig.enableWarningMessage = objectAppConfig["enableWarningMessage"].toBool();
            }
            else
            {
                mAppConfig.enableWarningMessage = true;
            }
        }
    }

    return res;
}

#include <QJsonArray>

void AppConfig::_Save()
{
    QJsonObject objectAppConfig;

    JSON_INSERT(AppConfig, bAdmin);
    JSON_INSERT(AppConfig, bStreamProcessedData);

    QJsonArray logMasks = mAppConfig.mLogMasks.GetJson();
    objectAppConfig.insert("LogMasks", logMasks);

    JSON_INSERT(AppConfig, autoExposure);

    JSON_INSERT(AppConfig, enableWarningMessage);

    // record
    QJsonObject recordObject;
    recordObject.insert(SETUP_APP,          objectAppConfig);

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

#define EXPORTOPTION_LABEL "ExportOption"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define JSON_INSERT(a, b) object##a.insert(TOSTRING(b), m##a.b)

#define GET_JSON_FEATURE(a, b, c) if(objectConfig.contains(TOSTRING(a)) == true) { \
    b = objectConfig[TOSTRING(a)].toBool(); } else { \
    b = c; bUpdateConfigFile = true; }

void AppConfig::GetDisplayStreamConfig(ProcessingConfig_t &processingConfig)
{
    bool bUpdateConfigFile = false;

    QString fileName = ".\\DisplayStreamOption.json";

    QFile jsonFile(fileName);

    if(jsonFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileArray;
        fileArray = jsonFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(fileArray));
        QJsonObject object = loadDoc.object();

        QJsonObject objectConfig = object[EXPORTOPTION_LABEL].toObject();

        GET_JSON_FEATURE(bAbsolute,               processingConfig.bAbsolute,               true)
        GET_JSON_FEATURE(bBiasCompensation,       processingConfig.bBiasCompensation,       true)
        GET_JSON_FEATURE(bFlatField,              processingConfig.bFlatField,              true)
        GET_JSON_FEATURE(bLinearisation,          processingConfig.bLinearisation,          true)
        GET_JSON_FEATURE(bSensorDefectCorrection, processingConfig.bSensorDefectCorrection, true)
        GET_JSON_FEATURE(bSensorPrnuCorrection,   processingConfig.bSensorPrnuCorrection,   true)

        jsonFile.close();
    }
    else
    {
        processingConfig.bBiasCompensation       = false;
        processingConfig.bSensorDefectCorrection = true;
        processingConfig.bSensorPrnuCorrection   = false;
        processingConfig.bLinearisation          = false;
        processingConfig.bFlatField              = false;
        processingConfig.bAbsolute               = false;

        bUpdateConfigFile = true;
    }

    if(bUpdateConfigFile == true)
    {
        _SaveDisplayStreamConfig(fileName, processingConfig);
    }
}

void AppConfig::_SaveDisplayStreamConfig(QString fileName, ProcessingConfig_t& processingConfig)
{
    QJsonObject objectConfig;

    objectConfig.insert("bAbsolute",                processingConfig.bAbsolute);
    objectConfig.insert("bBiasCompensation",        processingConfig.bBiasCompensation);
    objectConfig.insert("bFlatField",               processingConfig.bFlatField);
    objectConfig.insert("bLinearisation",           processingConfig.bLinearisation);
    objectConfig.insert("bSensorDefectCorrection",  processingConfig.bSensorDefectCorrection);
    objectConfig.insert("bSensorPrnuCorrection",    processingConfig.bSensorPrnuCorrection);

    // record
    QJsonObject recordObject;
    recordObject.insert(EXPORTOPTION_LABEL, objectConfig);

    QJsonDocument doc(recordObject);

    QFile jsonFile(fileName);
    if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&jsonFile);
        out.setCodec("UTF-8");
        out << doc.toJson();
        jsonFile.close();
    }
}

