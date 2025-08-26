#include <QVector>
#include <QMap>
#include <QMetaEnum>

#include "appControllerWorker.h"
#include "appResource.h"

#include <QDir>
#include <QDebug>

#define LOG_CW(x) Log("              Worker", x)

#define PICOAMMETER_INTERVAL 10

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

AppControllerWorker::AppControllerWorker(QObject *parent) : ClassCommon(parent)
{
}

AppControllerWorker::~AppControllerWorker()
{
}

void AppControllerWorker::OnWorkRequest(int value)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    Request eRequest = static_cast<Request>(value);

    bCancelCommand = false;

    switch(eRequest)
    {
    case Request::CmdReset:
        eError = _CmdReset();
        break;

    case Request::Test1:
        eError = _Test1();
        break;

    case Request::Test2:
        eError = _Test2();
        break;

    case Request::TestSetup:
        eError = _TestSetup();
        break;

    case Request::ProcessRawData:
        eError = _ProcessRawData();
        break;

    case Request::TestMeasureAE:
        eError = _TestMeasureAE();
        break;

    case Request::TestCapture:
        eError = _TestCapture();
        break;

    case Request::TestCaptureSequence:
        eError = _TestCaptureSequence();
        break;

    default:
        break;
    }

    LOG_CW(QString("done     request [%1] %2%3")
        .arg(EnumToString("Request", (int)eRequest))
        .arg((eError == ClassCommon::Error::Ok) ? "" : "ERROR : ")
        .arg(ClassCommon::ErrorToString(eError)));

    emit WorkDone(value, (int)eError);
}

void AppControllerWorker::CancelCmd()
{
    // some commands (streaming) are loops that required to be stopped
    bCancelCommand = true;
}

ClassCommon::Error AppControllerWorker::_CmdReset()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdReset();
    LOG_CW(QString("CmdReset %2").arg(ClassCommon::ErrorToString(eError)));

    return eError;
}

ClassCommon::Error AppControllerWorker::_Test1()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    int index = 0;

    while (((index ++) < 50000 ) && (eError == ClassCommon::Error::Ok) && (bCancelCommand == false))
    {
        eError = _Test1Sub(index);
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_Test1Sub(int testIndex)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdOpen();
    LOG_CW(QString("%1 CmdOpen %2").arg(testIndex, 5).arg(ClassCommon::ErrorToString(eError)));

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();

        LOG_CW(QString("%1 CmdClose %2").arg(testIndex, 5).arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_Test2()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdOpen();
    LOG_CW(QString("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));

    int index = 0;

    while (((index ++) < 50000 ) && (eError == ClassCommon::Error::Ok) && (bCancelCommand == false))
    {
        eError = _Test2Sub(index);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_Test2Sub(int)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    static SetupConfig_t setupConfig;
    static MeasureConfig_t measureConfig;
    static ProcessingConfig_t processingConfig;

    if(eError == ClassCommon::Error::Ok)
    {
        setupConfig.sensorTemperature = 25;
        setupConfig.eFilter = Filter_X;
        setupConfig.eNd = Nd_0;
        setupConfig.eIris = IrisIndex_2mm;

        eError = RESOURCES->mConoscopeApi->CmdSetup(setupConfig);
        LOG_CW(QString("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
    }


    //for(int loopIndex = 0; loopIndex < 1000; loopIndex ++)
    {
        if(eError == ClassCommon::Error::Ok)
        {
            measureConfig.exposureTimeUs = 10000;
            measureConfig.nbAcquisition  = 1;
            measureConfig.binningFactor = 1;
            measureConfig.bTestPattern = false;

            eError = RESOURCES->mConoscopeApi->CmdMeasure(measureConfig);
            LOG_CW(QString("CmdMeasure %1").arg(ClassCommon::ErrorToString(eError)));
            // LOG_CW(QString("CmdMeasure [%1] err = %2").arg(loopIndex).arg(ClassCommon::ErrorToString(eError)));
        }
    }
/*
    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdExportRaw();
        LOG_CW(QString("CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

        QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
        QFile file (fileName);
        file.remove();
    }

    if(eError == ClassCommon::Error::Ok)
    {
        processingConfig.bBiasCompensation = true;
        processingConfig.bSensorDefectCorrection = true;
        processingConfig.bSensorPrnuCorrection = true;
        processingConfig.bLinearisation = true;
        processingConfig.bFlatField = true;
        processingConfig.bAbsolute = true;

        eError = RESOURCES->mConoscopeApi->CmdExportProcessed(processingConfig);
        LOG_CW(QString("CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

        QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
        QFile file (fileName);
        file.remove();
    }
*/
    return eError;
}

ClassCommon::Error AppControllerWorker::_TestSetup()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdOpen();
    LOG_CW(QString("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));

    int index = 0;

    while (((index ++) < 5 ) && (eError == ClassCommon::Error::Ok) && (bCancelCommand == false))
    {
        eError = _TestSetupSub(index);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_TestSetupSub(int)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    static SetupConfig_t setupConfig;

    setupConfig.sensorTemperature = 15;
    setupConfig.eFilter = Filter_X;
    setupConfig.eNd = Nd_0;
    setupConfig.eIris = IrisIndex_2mm;

    QList<Filter_t> filterList;

    filterList.append(Filter_Mirror);
    filterList.append(Filter_BK7);
    filterList.append(Filter_IrCut);
    filterList.append(Filter_Z);
    filterList.append(Filter_Xz);
    filterList.append(Filter_Yb);
    filterList.append(Filter_Ya);
    filterList.append(Filter_X);

    QList<Nd_t>     ndList;

    ndList.append(Nd_1);
    ndList.append(Nd_2);
    ndList.append(Nd_3);
    ndList.append(Nd_4);
    ndList.append(Nd_0);

    for(int filterIndex = 0; filterIndex < filterList.count(); filterIndex ++)
    {
        _UpdateSetupTemperature(setupConfig);

        if(eError == ClassCommon::Error::Ok)
        {
            setupConfig.eFilter = filterList.at(filterIndex);

            eError = RESOURCES->mConoscopeApi->CmdSetup(setupConfig);
        }

        if(eError == ClassCommon::Error::Ok)
        {
            eError = _GetSetup();
        }
        else
        {
            LOG_CW(QString("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
        }

        if(bCancelCommand == true)
        {
            return ClassCommon::Error::Aborted;
        }
    }

    for(int ndIndex = 0; ndIndex < ndList.count(); ndIndex ++)
    {
        _UpdateSetupTemperature(setupConfig);

        if(eError == ClassCommon::Error::Ok)
        {
            setupConfig.eNd = ndList.at(ndIndex);

            eError = RESOURCES->mConoscopeApi->CmdSetup(setupConfig);
        }

        if(eError == ClassCommon::Error::Ok)
        {
            eError = _GetSetup();
        }
        else
        {
            LOG_CW(QString("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
        }

        if(bCancelCommand == true)
        {
            return ClassCommon::Error::Aborted;
        }
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_TestMeasureAE()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdOpen();
    LOG_CW(QString("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));

    int index = 0;
    int loopNumber = 50;

    while (((index ++) < loopNumber ) && (eError == ClassCommon::Error::Ok) && (bCancelCommand == false))
    {
        eError = _TestMeasureAESub(index);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();
        LOG_CW(QString("CmdClose %1").arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_TestMeasureAESub(int testIndex)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    LOG_CW(QString("**** _TestMeasureAESub %1").arg(testIndex));

    static SetupConfig_t setupConfig;
    static MeasureConfig_t measureConfig;
    static ProcessingConfig_t processingConfig;

    if(eError == ClassCommon::Error::Ok)
    {
        setupConfig.sensorTemperature = 25;
        setupConfig.eFilter = Filter_X;
        setupConfig.eNd = Nd_0;
        setupConfig.eIris = IrisIndex_2mm;

        eError = RESOURCES->mConoscopeApi->CmdSetup(setupConfig);
        LOG_CW(QString("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        measureConfig.exposureTimeUs = 10000;
        measureConfig.nbAcquisition  = 1;
        measureConfig.binningFactor = 1;
        measureConfig.bTestPattern = false;

        eError = _MeasureAE(measureConfig);
        LOG_CW(QString("_MeasureAE %1").arg(ClassCommon::ErrorToString(eError)));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdExportRaw();
        LOG_CW(QString("CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

        QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
        QFile file (fileName);
        file.remove();
    }

    if(eError == ClassCommon::Error::Ok)
    {
        processingConfig.bBiasCompensation = true;
        processingConfig.bSensorDefectCorrection = true;
        processingConfig.bSensorPrnuCorrection = true;
        processingConfig.bLinearisation = true;
        processingConfig.bFlatField = true;
        processingConfig.bAbsolute = true;

        eError = RESOURCES->mConoscopeApi->CmdExportProcessed(processingConfig);
        LOG_CW(QString("CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

        QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
        QFile file (fileName);
        file.remove();
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_MeasureAE(MeasureConfig_t& measureConfig)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    eError = RESOURCES->mConoscopeApi->CmdMeasureAE(measureConfig);
    LOG_CW(QString("CmdMeasureAE %1").arg(ClassCommon::ErrorToString(eError)));

    if(eError == ClassCommon::Error::Ok)
    {
        // wait for the end of the processing
        MeasureStatus_t measureAEStatus;
        bool bDone = false;
        int timeout = 120; // 60 sec with a 0.5 sec delay

        while((bDone == false) && (eError == ClassCommon::Error::Ok))
        {
            eError = RESOURCES->mConoscopeApi->CmdMeasureAEStatus(measureAEStatus);
            if(eError != ClassCommon::Error::Ok)
            {
                LOG_CW(QString("CmdMeasureAEStatus %1").arg(ClassCommon::ErrorToString(eError)));
            }

            if(measureAEStatus.state == MeasureStatus_t::State_t::State_Done)
            {
                bDone = true;
            }
            else
            {
                QThread::msleep(500);

                if(timeout-- == 0)
                {
                    eError = ClassCommon::Error::Timeout;
                }
            }
        }
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_TestCapture()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

#ifdef TEST_SHIFT
    if(eError == ClassCommon::Error::Ok)
    {
        ConoscopeSettings_t ConoscopeConfig;
        eError = RESOURCES->mConoscopeApi->CmdGetConfig(ConoscopeConfig);

        ConoscopeConfig.exportFormat = ExportFormat_bin_jpg;

        eError = RESOURCES->mConoscopeApi->CmdSetConfig(ConoscopeConfig);
        LOG_CW(QString("CmdSetConfig %1").arg(ClassCommon::ErrorToString(eError)));
    }
#endif

    eError = RESOURCES->mConoscopeApi->CmdOpen();
    LOG_CW(QString("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));

    int index = 0;

#define CAPTURE_TEST_NB_LOOPS 100

    while(((index ++) < CAPTURE_TEST_NB_LOOPS ) &&
          (eError == ClassCommon::Error::Ok) &&
          (bCancelCommand == false))
    {
        eError = _TestCaptureSub(index);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_TestCaptureSub(int testIndex)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    static SetupConfig_t setupConfig;
    static MeasureConfig_t measureConfig;
    static ProcessingConfig_t processingConfig;

#ifdef TEST_SHIFT
    Filter_t eFilter;
    Nd_t     eNd;

    int filterIndex = testIndex % 5;
    int ndIndex = (testIndex / 5) % 3;

    if(filterIndex == 0)
    {
        eFilter = Filter_X;
    }
    else if(filterIndex == 1)
    {
        eFilter = Filter_Xz;
    }
    else if(filterIndex == 2)
    {
        eFilter = Filter_Ya;
    }
    else if(filterIndex == 3)
    {
        eFilter = Filter_Yb;
    }
    else if(filterIndex == 4)
    {
        eFilter = Filter_Z;
    }

    eNd = Nd_0;

/*
    if(ndIndex == 0)
    {
        eNd = Nd_0;
    }
    else if(ndIndex == 1)
    {
        eNd = Nd_1;
    }
    else if(ndIndex == 2)
    {
        eNd = Nd_2;
    }
*/
#endif

    if(eError == ClassCommon::Error::Ok)
    {
        setupConfig.sensorTemperature = 25;
#ifdef TEST_SHIFT
        setupConfig.eFilter = eFilter;
        setupConfig.eNd     = eNd;
#else
        setupConfig.eFilter = Filter_X;
        setupConfig.eNd     = Nd_0;
#endif
        setupConfig.eIris = IrisIndex_2mm;

        eError = RESOURCES->mConoscopeApi->CmdSetup(setupConfig);
        LOG_CW(QString("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
    }

    if(eError == ClassCommon::Error::Ok)
    {
#ifdef TEST_SHIFT
        measureConfig.exposureTimeUs = 38000;
#else
        measureConfig.exposureTimeUs = 10000;
#endif
        measureConfig.nbAcquisition  = 1;
        measureConfig.binningFactor = 1;
        measureConfig.bTestPattern = false;

        eError = RESOURCES->mConoscopeApi->CmdMeasure(measureConfig);
        LOG_CW(QString("CmdMeasure %1").arg(ClassCommon::ErrorToString(eError)));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdExportRaw();
        LOG_CW(QString("CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));

        QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
        QFile file (fileName);
        file.remove();
    }

#ifndef TEST_SHIFT
    if(eError == ClassCommon::Error::Ok)
    {
        processingConfig.bBiasCompensation = true;
        processingConfig.bSensorDefectCorrection = true;
        processingConfig.bSensorPrnuCorrection = true;
        processingConfig.bLinearisation = true;
        processingConfig.bFlatField = true;
        processingConfig.bAbsolute = true;

        eError = RESOURCES->mConoscopeApi->CmdExportProcessed(processingConfig);
        LOG_CW(QString("CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));

        QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
        QFile file (fileName);
        file.remove();
    }
#else
    QString fileName = RESOURCES->mConoscopeApi->GetFilePath();
    QFile file (fileName);
    file.remove();
#endif


    return eError;
}

#define DELETE_BIN_FILES

ClassCommon::Error AppControllerWorker::_TestCaptureSequence()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    ConoscopeSettings_t conoscopeConfig;
    eError = RESOURCES->mConoscopeApi->CmdGetConfig(conoscopeConfig);

    ConoscopeDebugSettings_t debugConfig;

    debugConfig.emulateCamera = false;
    // debugConfig.dummyRawImagePath = CONVERT_TO_STRING(imagePath);
    debugConfig.emulateWheel = true;
    debugConfig.debugMode = false;

    eError = RESOURCES->mConoscopeApi->CmdSetDebugConfig(debugConfig);
    LOG_CW(QString("CmdSetDebugConfig %1").arg(ClassCommon::ErrorToString(eError)));

    eError = RESOURCES->mConoscopeApi->CmdOpen();
    LOG_CW(QString("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));

    CaptureSequenceConfig_t config;

    eError = RESOURCES->mConoscopeApi->CmdGetCaptureSequence(config);

    int index = 0;
#define CAPTURE_SEQUENCE_TEST_NB_LOOPS 500

// #define CHANGE_ND_POSITION

#ifdef CHANGE_ND_POSITION
    int ndIndex;

    QMap<int, Nd_t> ndMap = {{0, Nd_0},
                             {1, Nd_1},
                             {2, Nd_2},
                             {3, Nd_3},
                             {4, Nd_4}};
#endif

    config.nbAcquisition = 1;
    config.bAutoExposure = true;
    config.bUseExpoFile = false;
    config.bSaveCapture = true;

    config.bUseRoi = false;

    // while(((index ++) < CAPTURE_SEQUENCE_TEST_NB_LOOPS ) &&
    while((1) &&
          (eError == ClassCommon::Error::Ok) &&
          (bCancelCommand == false))
    {
        LOG_CW(QString("Step %1").arg(index));

#ifdef CHANGE_ND_POSITION
        ndIndex = index % 5;
        qDebug() << QString("    %1").arg(ndIndex);
        config.eNd = ndMap[ndIndex];
#endif
        eError = _TestCaptureSequenceSub(index, config);

#ifdef DELETE_BIN_FILES
        // delete all bin files
        QString path = CONVERT_TO_QSTRING(conoscopeConfig.capturePath);
        QDir dir(path);
        dir.setNameFilters(QStringList() << "*.bin");
        dir.setFilter(QDir::Files);
        foreach(QString dirFile, dir.entryList())
        {
            dir.remove(dirFile);
        }
#endif

    }

    if(eError == ClassCommon::Error::Ok)
    {
        LOG_CW(QString("-> OK"));
    }
    else
    {
        LOG_CW(QString("-> FAILED"));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_TestCaptureSequenceSub(int testIndex, CaptureSequenceConfig_t config)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // config.bAutoExposure = false;
    // config.bUseExpoFile = false;
    // config.bSaveCapture = false;

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdCaptureSequence(config);
    }

    // ClassCommon::Error CmdCaptureSequenceCancel();

    CaptureSequenceStatus_t status;

    if(eError == ClassCommon::Error::Ok)
    {
        status.state = CaptureSequenceStatus_t::State_t::State_Done;
        int currentStep = -1;
        status.currentSteps = currentStep;

        bool bDone = false;
        bool bLaunchCancelCommand = false;

        do
        {
            QThread::msleep(500);

            if(bCancelCommand == true)
            {
                if(bLaunchCancelCommand == false)
                {
                    eError = RESOURCES->mConoscopeApi->CmdCaptureSequenceCancel();
                }

                bLaunchCancelCommand = true;
            }
            else
            {
                if(currentStep != status.currentSteps)
                {
                    LOG_CW(QString("  step %1 of %2").arg(status.currentSteps).arg(status.nbSteps));
                    currentStep = status.currentSteps;
                }
            }

            if(eError == ClassCommon::Error::Ok )
            {
                eError = RESOURCES->mConoscopeApi->CmdCaptureSequenceStatus(status);
                // LOG_CW(QString("CmdCaptureSequenceStatus %1").arg(status.state));

                if((status.state == CaptureSequenceStatus_t::State_t::State_Done) ||
                   (status.state == CaptureSequenceStatus_t::State_t::State_Error) ||
                   (status.state == CaptureSequenceStatus_t::State_t::State_Cancel))
                {
                    bDone = true;
                }
            }
        } while((bDone == false) && (eError == ClassCommon::Error::Ok));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        if(status.state != CaptureSequenceStatus_t::State_t::State_Done)
        {
            if(status.state == CaptureSequenceStatus_t::State_t::State_Cancel)
            {
                eError = ClassCommon::Error::Aborted;
                LOG_CW(QString("                   canceled"));
            }
            else
            {
                eError = ClassCommon::Error::Failed;
                LOG_CW(QString("                   with status not ok"));
            }
        }
    }
    else
    {
        LOG_CW(QString("CmdCaptureSequence ERROR"));
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_ProcessRawData()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    QStringList directoryPathList;

/*
    directoryPathList.append("E:\\TmpConoscope\\20200224_process data\\ND0- all iris - all filters\\02");
    directoryPathList.append("E:\\TmpConoscope\\20200224_process data\\ND0- all iris - all filters\\03");
    directoryPathList.append("E:\\TmpConoscope\\20200224_process data\\ND0- all iris - all filters\\04");
    directoryPathList.append("E:\\TmpConoscope\\20200224_process data\\ND0- all iris - all filters\\05");
*/

//     directoryPathList.append("E:\\_Taprisiot_2\\LED10-80%");
/*    directoryPathList.append("E:\\_Taprisiot_2\\LED2-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\LED3-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\LED4-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\LED5-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\LED6-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\LED7-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\LED8-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\White-80%");
    directoryPathList.append("E:\\_Taprisiot_2\\DensityPowerVsFilter- NoOpale");
    directoryPathList.append("E:\\_Taprisiot_2\\DensityPowerVsFilter- NoOpale\\Sans-opale-100%");
    directoryPathList.append("E:\\_Taprisiot_2\\White-80%");*/

    directoryPathList.append("E:\\_Taprisiot_6\\19041831");

    foreach(QString directoryPath, directoryPathList)
    {
        _ProcessRawData(directoryPath);
    }

    return eError;
}

#define CONVERT_TO_STRING(a) a.toUtf8().constData();

ClassCommon::Error AppControllerWorker::_ProcessRawData(QString directoryPath)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    // QString directoryPath = "E:\\_Taprisiot";

    QDir directory(directoryPath);
    QStringList images = directory.entryList(QStringList() << "*.bin",QDir::Files);

    static ConoscopeDebugSettings_t debugConfig;
    eError = RESOURCES->mConoscopeApi->CmdGetDebugConfig(debugConfig);
    LOG_CW(QString("CmdGetDebugConfig %1").arg(ClassCommon::ErrorToString(eError)));

    if(eError == ClassCommon::Error::Ok)
    {
/*
    bool        debugMode;
    bool        emulateCamera;
    std::string dummyRawImagePath; // path of the dymmy image when camera is emulated
*/
        debugConfig.emulateCamera = true;
        eError = RESOURCES->mConoscopeApi->CmdSetDebugConfig(debugConfig);
        LOG_CW(QString(" %1").arg(ClassCommon::ErrorToString(eError)));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdOpen();
        LOG_CW(QString("CmdOpen %1").arg(ClassCommon::ErrorToString(eError)));
    }

    if(eError == ClassCommon::Error::Ok)
    {
        ConoscopeSettings_t CononscopeConfig;
        eError = RESOURCES->mConoscopeApi->CmdGetConfig(CononscopeConfig);
        CononscopeConfig.capturePath = CONVERT_TO_STRING(QString("%1\\proc").arg(directoryPath));
        eError = RESOURCES->mConoscopeApi->CmdSetConfig(CononscopeConfig);
    }

    if(eError == ClassCommon::Error::Ok)
    {
        static SetupConfig_t setupConfig;

        setupConfig.sensorTemperature = 15;
        setupConfig.eFilter = Filter_X;
        setupConfig.eNd = Nd_0;
        setupConfig.eIris = IrisIndex_2mm;

        eError = RESOURCES->mConoscopeApi->CmdSetup(setupConfig);
        LOG_CW(QString("CmdSetup %1").arg(ClassCommon::ErrorToString(eError)));
    }

    QString imagePath;

    foreach(QString filename, images)
    {
        imagePath = QString("%1\\%2").arg(directoryPath).arg(filename);

        LOG_CW(QString("FILE IS %1").arg(imagePath));

        if(eError == ClassCommon::Error::Ok)
        {
            debugConfig.emulateCamera = true;
            // debugConfig.dummyRawImagePath = "E:\\_Taprisiot\\20200214_134421_raw_1.bin";
            debugConfig.dummyRawImagePath = CONVERT_TO_STRING(imagePath);

            eError = RESOURCES->mConoscopeApi->CmdSetDebugConfig(debugConfig);
            LOG_CW(QString("CmdSetDebugConfig %1").arg(ClassCommon::ErrorToString(eError)));
        }

        if(eError == ClassCommon::Error::Ok)
        {
            MeasureConfig_t measurementConfig;

            measurementConfig.exposureTimeUs = 10000;
            measurementConfig.nbAcquisition = 1;
            measurementConfig.binningFactor = 1;
            measurementConfig.bTestPattern = false;

            eError = RESOURCES->mConoscopeApi->CmdMeasure(measurementConfig);
            LOG_CW(QString("CmdMeasure %1").arg(ClassCommon::ErrorToString(eError)));
        }

/*
        if(eError == ClassCommon::Error::Ok)
        {
            eError = RESOURCES->mConoscopeApi->CmdExportRaw();
            LOG_CW(QString("CmdExportRaw %1").arg(ClassCommon::ErrorToString(eError)));
        }
*/

        if(eError == ClassCommon::Error::Ok)
        {
            ProcessingConfig_t processedConfig;

            processedConfig.bBiasCompensation       = true;
            processedConfig.bSensorDefectCorrection = true;
            processedConfig.bSensorPrnuCorrection   = true;
            processedConfig.bLinearisation          = true;
            processedConfig.bFlatField              = true;
            processedConfig.bAbsolute               = true;

            eError = RESOURCES->mConoscopeApi->CmdExportProcessed(processedConfig);
            LOG_CW(QString("CmdExportProcessed %1").arg(ClassCommon::ErrorToString(eError)));
        }
    }

    // if(eError == ClassCommon::Error::Ok)
    {
        eError = RESOURCES->mConoscopeApi->CmdClose();
        LOG_CW(QString("CmdClose %1").arg(ClassCommon::ErrorToString(eError)));
    }

    return eError;
}

ClassCommon::Error AppControllerWorker::_GetSetup()
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;
    SetupStatus_t setupStatus;

    eError = RESOURCES->mConoscopeApi->CmdSetupStatus(setupStatus);

    LOG_CW(QString("CmdSetup [%1, %2]")
    .arg(RESOURCES->mFilterToStringMap[setupStatus.eFilter], -7)
    .arg(RESOURCES->mNdToStringMap[setupStatus.eNd], -3));

    return eError;
}

void AppControllerWorker::_UpdateSetupTemperature(SetupConfig_t& setupConfig)
{
    if(setupConfig.sensorTemperature > 35)
    {
        setupConfig.sensorTemperature = 15;
    }
    else
    {
        setupConfig.sensorTemperature += 5;
    }
}

