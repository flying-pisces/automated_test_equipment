#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configuration.h"

#include "toolReturnCode.h"
#include "toolString.h"
#include "formCmd.h"

#include <QMessageBox>

#define UPDATE_BTN(a) ui->btn##a->setEnabled(b_##a)
#define UPDATE_FORM(a) form##a->setEnabled(b_##a)

#define CONNECT_FORM(a, b) connect(a, &FormCmd::ExecuteCmd, this, b)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lblStatus->setFixedHeight(30);
    ui->lblVersion->setFixedHeight(30);

    // create a scroll area for command panel
    central = new QWidget;
    scrollArea = new QScrollArea;
    verticalLayout = new QVBoxLayout(central);

    verticalLayout->setMargin(0);
    verticalLayout->setSpacing(0);

    scrollArea->setWidget(central);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedWidth(420);

    ui->gridLayout->addWidget(scrollArea, 1, 0, 1, 1);

    // initialise parameters from json configuration file
    mCmdParameters.applicationConfig.bStreamProcessedData  = RESOURCES->mAppConfig.bStreamProcessedData;
    mCmdParameters.applicationConfig.autoExposure          = RESOURCES->mAppConfig.autoExposure;

    mCmdParameters.applicationState.cfgPathEnable = true;
    mCmdParameters.applicationState.emulatedCameraEnable = true;

    // create the forms for the buttons
    // note mParameter will be updated by the ui

    formCmdSetConfig     = new FormCmd(FormCmd::CmdType::SetConfig, &mCmdParameters, this);
    formCmdSetAE         = new FormCmd(FormCmd::CmdType::AE, &mCmdParameters, this);
    formCmdSetAEMeasArea = new FormCmd(FormCmd::CmdType::AEMeasArea, &mCmdParameters, this);
    formCmdSetRoi        = new FormCmd(FormCmd::CmdType::ROI, &mCmdParameters, this);

    formCmdOpen          = new FormCmd(FormCmd::CmdType::Open, &mCmdParameters, this);
    formCmdSetup         = new FormCmd(FormCmd::CmdType::Setup, &mCmdParameters, this);
    formCmdSetupStatus   = new FormCmd(FormCmd::CmdType::SetupStatus, &mCmdParameters, this);
    formCmdMeasure       = new FormCmd(FormCmd::CmdType::Measure, &mCmdParameters, this);
    formCmdExportRaw     = new FormCmd(FormCmd::CmdType::ExportRaw, &mCmdParameters, this);
    formCmdExportProc    = new FormCmd(FormCmd::CmdType::ExportProcessed, &mCmdParameters, this);
    formCmdClose         = new FormCmd(FormCmd::CmdType::Close, &mCmdParameters, this);
    formCmdReset         = new FormCmd(FormCmd::CmdType::Reset, &mCmdParameters, this);

    formDisplayStream    = new FormCmd(FormCmd::CmdType::DisplayStream, &mCmdParameters, this);
    formDisplayRaw       = new FormCmd(FormCmd::CmdType::DisplayRaw, &mCmdParameters, this);
    formDisplayProcessed = new FormCmd(FormCmd::CmdType::DisplayProcessed, &mCmdParameters, this);

    formCaptureSequence        = new FormCmd(FormCmd::CmdType::CaptureSequence, &mCmdParameters, this);
    formCaptureSequenceCancel  = new FormCmd(FormCmd::CmdType::CaptureSequenceCancel, &mCmdParameters, this);
    formCaptureSequenceStatus  = new FormCmd(FormCmd::CmdType::CaptureSequenceStatus, &mCmdParameters, this);

    formCmdCfgFileWrite  = new FormCmd(FormCmd::CmdType::CfgFileWrite, &mCmdParameters, this);
    formCmdCfgFileRead   = new FormCmd(FormCmd::CmdType::CfgFileRead, &mCmdParameters, this);
    formCmdCfgFileStatus = new FormCmd(FormCmd::CmdType::CfgFileStatus, &mCmdParameters, this);

    formCmdMeasureAE        = new FormCmd(FormCmd::CmdType::MeasureAE, &mCmdParameters, this);
    formCmdMeasureAECancel  = new FormCmd(FormCmd::CmdType::MeasureAECancel, &mCmdParameters, this);
    formCmdMeasureAEStatus  = new FormCmd(FormCmd::CmdType::MeasureAEStatus, &mCmdParameters, this);

    formCmdConvertRaw = new FormCmd(FormCmd::CmdType::ConvertRaw, &mCmdParameters, this);

    mFormList.append(formCmdSetConfig);
    mFormList.append(formCmdSetAE);
    mFormList.append(formCmdSetAEMeasArea);
    mFormList.append(formCmdSetRoi);

    mFormList.append(formCmdOpen);
    mFormList.append(formCmdSetup);
    mFormList.append(formCmdSetupStatus);
    mFormList.append(formCmdMeasure);

    mFormList.append(formCmdMeasureAE);
    mFormList.append(formCmdMeasureAECancel);
    mFormList.append(formCmdMeasureAEStatus);

    mFormList.append(formCmdExportRaw);
    mFormList.append(formCmdExportProc);
    mFormList.append(formCmdClose);
    mFormList.append(formCmdReset);

    mFormList.append(formCaptureSequence);
    mFormList.append(formCaptureSequenceCancel);
    mFormList.append(formCaptureSequenceStatus);

    mFormList.append(formDisplayStream);
    mFormList.append(formDisplayRaw);
    mFormList.append(formDisplayProcessed);

    mFormList.append(formCmdConvertRaw);

    if(RESOURCES->mAppConfig.bAdmin == true)
    {
        mFormList.append(formCmdCfgFileWrite);
    }
    else
    {
        formCmdCfgFileWrite->hide();
    }
    mFormList.append(formCmdCfgFileRead);
    mFormList.append(formCmdCfgFileStatus);

    verticalSpacer = new QSpacerItem(20, 400, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // populate the
    for(int index = 0; index < mFormList.count(); index ++)
    {
        verticalLayout->addWidget(mFormList[index]);
    }

    verticalLayout->addItem(verticalSpacer);

    // register the action to do when the button is pressed
    CONNECT_FORM(formCmdOpen,               &MainWindow::on_btnCmdOpen);
    CONNECT_FORM(formCmdSetup,              &MainWindow::on_btnCmdSetup);
    CONNECT_FORM(formCmdSetupStatus,        &MainWindow::on_btnCmdSetupStatus);
    CONNECT_FORM(formCmdMeasure,            &MainWindow::on_btnCmdMeasure);
    CONNECT_FORM(formCmdExportRaw,          &MainWindow::on_btnCmdExportRaw);
    CONNECT_FORM(formCmdExportProc,         &MainWindow::on_btnCmdExportProc);
    CONNECT_FORM(formCmdClose,              &MainWindow::on_btnCmdClose);
    CONNECT_FORM(formCmdReset,              &MainWindow::on_btnCmdReset);
    CONNECT_FORM(formCmdSetConfig,          &MainWindow::on_btnCmdSetConfig);
    CONNECT_FORM(formCmdCfgFileWrite,       &MainWindow::on_btnCmdCfgFileWrite);
    CONNECT_FORM(formCmdCfgFileRead,        &MainWindow::on_btnCmdCfgFileRead);
    CONNECT_FORM(formCmdCfgFileStatus,      &MainWindow::on_btnCmdCfgFileStatus);
    CONNECT_FORM(formDisplayStream,         &MainWindow::on_btnDisplayStream);
    CONNECT_FORM(formDisplayRaw,            &MainWindow::on_btnDisplayRaw);
    CONNECT_FORM(formDisplayProcessed,      &MainWindow::on_btnDisplayProcessed);
    CONNECT_FORM(formCaptureSequence,       &MainWindow::on_btnCaptureSequence);
    CONNECT_FORM(formCaptureSequenceCancel, &MainWindow::on_btnCaptureSequenceCancel);
    CONNECT_FORM(formCaptureSequenceStatus, &MainWindow::on_btnCaptureSequenceStatus);

    CONNECT_FORM(formCmdMeasureAE,          &MainWindow::on_btnMeasureAE);
    CONNECT_FORM(formCmdMeasureAECancel,    &MainWindow::on_btnMeasureAECancel);
    CONNECT_FORM(formCmdMeasureAEStatus,    &MainWindow::on_btnMeasureAEStatus);

    CONNECT_FORM(formCmdConvertRaw, &MainWindow::on_btnConvertRaw);

    mController = new AppController();

    connect(mController, &AppController::WriteLog,
            this, &MainWindow::onLog);

    connect(mController, &AppController::WriteStatus,
            this, &MainWindow::onStatus);

    connect(mController, &AppController::StateChange,
            this, &MainWindow::onStateChange);

    connect(mController, &AppController::WarningMessage,
            this, &MainWindow::onWarningMessage);

    // buffer used for streaming
    mController->SetRawDataBuffer(&mFrameBuffer);

    mController->Start();

    DisplayVersion();

    mController->RegisterCallback();

    // retrieve current parameters
    mController->CmdGetConfig(mCmdParameters.cmdSetupConfig,
                              mCmdParameters.cmdMeasureConfig,
                              mCmdParameters.cmdProcessingConfig,
                              mCmdParameters.debugConfig,
                              mCmdParameters.cmdCaptureSequenceConfig);

    QString windowTitle = APPLICATION_TITLE;

    if(mCmdParameters.debugConfig.debugMode == true)
    {
        windowTitle = windowTitle + " | DEBUG mode";
    }

    if(mCmdParameters.debugConfig.emulateCamera == true)
    {
        windowTitle = windowTitle + " | emulated camera";
    }

    if(mCmdParameters.debugConfig.emulateWheel == true)
    {
        windowTitle = windowTitle + " | emulated wheel";
    }

    if(RESOURCES->mAppConfig.bAdmin == true)
    {
        windowTitle = windowTitle + " | ADMIN";
    }

    setWindowTitle(windowTitle);

    mController->CmdGetConfig(mCmdParameters.config);

    // check that the config matched with the level of the application
    if(RESOURCES->mAppConfig.bAdmin == false)
    {
        mController->CmdSetConfig(mCmdParameters.config);
    }

    // request ui to update settings according to configuration
    for(int index = 0; index < mFormList.count(); index ++)
    {
        mFormList[index]->PleaseUpdate();
    }

    // streaming
    dialogStream = new DialogStream();
    dialogStream->pFrameBuffer = &mFrameBuffer;

    connect(mController, &AppController::RawBufferReady,
            dialogStream, &DialogStream::onRawBufferReady);

    connect(mController, &AppController::RawBufferAeRoi,
            dialogStream, &DialogStream::onRawBufferAeRoi);

    connect(mController, &AppController::PleaseUpdateExposureTime,
            this, &MainWindow::onPleaseUpdateExposureTime);

    connect(dialogStream, &QDialog::finished,
            this, &MainWindow::dialogStreamFinished);

    UpdateFormsVisibility(AppController::State::Undefined);
}

MainWindow::~MainWindow()
{
    RESOURCES->Close();

    _CloseWindow(dialogStream);

    mController->Stop();

    delete mController;

#ifdef V_LAYOUT
    for(int index = 0; index < mFormList.count(); index ++)
    {
        verticalLayout->removeWidget(mFormList[index]);
    }

    verticalLayout->removeItem(verticalSpacer);
#endif

    delete verticalSpacer;

    for(int index = 0; index < mFormList.count(); index ++)
    {
        delete mFormList[index];
    }

    delete ui;
}

#define MAX_NUMBER_LINES 200

void MainWindow::onLog(QString message)
{
    if(txtLogCount ++ > MAX_NUMBER_LINES)
    {
        ui->txtLog->clear();
        txtLogCount = 0;
    }

    ui->txtLog->append(message);
}

void MainWindow::onStatus(QString message)
{
    ui->lblStatus->setText(message);
}

void MainWindow::onStateChange(int state)
{
    onStatus(AppController::EnumToString("State", state));

    AppController::State eState = (AppController::State)state;

    bool b_CmdOpen = false;
    bool b_CmdSetup = false;
    bool b_CmdSetupStatus = false;
    bool b_CmdMeasure = false;
    bool b_CmdExportRaw = false;
    bool b_CmdExportProc = false;
    bool b_CmdClose = false;
    bool b_CmdReset = false;

    bool b_CmdCfgFileRead = false;
    bool b_CmdCfgFileWrite = false;
    bool b_CmdCfgFileStatus = false;

    bool b_DisplayStream = false;
    bool b_DisplayRaw = false;
    bool b_DisplayProcessed = false;

    bool b_CaptureSequence = false;
    bool b_CaptureSequenceCancel = false;
    bool b_CaptureSequenceStatus = false;

    bool b_CmdMeasureAE = false;
    bool b_CmdMeasureAECancel = false;
    bool b_CmdMeasureAEStatus = false;

    bool b_CmdConvertRaw = false;

    switch(eState)
    {
    case AppController::State::Undefined:
        b_CmdOpen = true;
        b_CmdReset = true;
        break;

    case AppController::State::Idle:
        b_CmdOpen = true;
        b_CmdReset = true;
        b_CmdConvertRaw = true;

        if(mCmdParameters.debugConfig.debugMode == true)
        {
            b_CmdExportProc = true;
        }
        break;

    case AppController::State::Opening:
        b_CmdReset = true;
        break;

    case AppController::State::Opened:
        b_CmdSetup = true;
        b_CmdSetupStatus = true;

        b_CmdClose = true;
        b_CmdReset = true;

        b_CmdCfgFileRead   = true;
        b_CmdCfgFileWrite  = true;
        b_CmdCfgFileStatus = true;

        b_CaptureSequence = true;
        break;

    case AppController::State::SettingUp:
        b_CmdSetupStatus = true;
        break;

    case AppController::State::Ready:
        b_CmdSetup = true;
        b_CmdSetupStatus = true;

        b_CmdMeasure = true;
        b_CmdMeasureAE = true;

        b_CmdClose = true;
        b_CmdReset = true;

        b_CmdCfgFileRead   = true;
        b_CmdCfgFileWrite  = true;
        b_CmdCfgFileStatus = true;

        b_DisplayStream = true;

        b_CaptureSequence = true;
        break;

    case AppController::State::Measuring:
        break;

    case AppController::State::MeasureDone:
        b_CmdSetup = true;
        b_CmdSetupStatus = true;

        b_CmdMeasure = true;
        b_CmdMeasureAE = true;
        b_CmdExportRaw = true;
        b_CmdExportProc = true;

        b_DisplayRaw = true;
        b_DisplayProcessed = true;

        b_CmdClose = true;
        b_CmdReset = true;

        b_CaptureSequence = true;
        break;

    case AppController::State::Closing:
        break;

    case AppController::State::Reseting:
        break;

    case AppController::State::Error:
        b_CmdClose = true;
        b_CmdReset = true;
        break;

    case AppController::State::CfgFileReading:
    case AppController::State::CfgFileWriting:
        b_CmdCfgFileStatus = true;
        break;

    case AppController::State::Streaming:
        b_DisplayStream = true;
        break;

    case AppController::State::StreamStoping:
        break;

    case AppController::State::DisplayRaw:
        break;

    case AppController::State::DisplayProcessed:
        break;

    case AppController::State::CapturingSequence:
        b_CaptureSequenceCancel = true;
        b_CaptureSequenceStatus = true;
        break;

    case AppController::State::CaptureSequenceCanceling:
        b_CaptureSequenceStatus = true;
        break;

    case AppController::State::MeasuringAE:
        b_CmdMeasureAECancel = true;
        b_CmdMeasureAEStatus = true;
        break;

    default:
        break;
    }

    if(mCmdParameters.debugConfig.debugMode == true)
    {
        b_CmdOpen          = true;
        b_CmdSetup         = true;
        b_CmdSetupStatus   = true;
        b_CmdMeasure       = true;
        b_CmdMeasureAE     = true;
        b_CmdExportRaw     = true;
        b_CmdExportProc    = true;
        b_CmdClose         = true;
        b_CmdReset         = true;

        b_CmdCfgFileRead   = true;
        b_CmdCfgFileWrite  = true;
        b_CmdCfgFileStatus = true;

        b_DisplayStream    = true;
        b_DisplayRaw       = true;
        b_DisplayProcessed = true;
    }

    UPDATE_FORM(CmdOpen);
    UPDATE_FORM(CmdSetup);
    UPDATE_FORM(CmdSetupStatus);
    UPDATE_FORM(CmdMeasure);
    UPDATE_FORM(CmdExportRaw);
    UPDATE_FORM(CmdExportProc);
    UPDATE_FORM(CmdClose);
    UPDATE_FORM(CmdReset);

    UPDATE_FORM(CmdCfgFileRead);
    UPDATE_FORM(CmdCfgFileWrite);
    UPDATE_FORM(CmdCfgFileStatus);

    UPDATE_FORM(DisplayStream);
    UPDATE_FORM(DisplayRaw);
    UPDATE_FORM(DisplayProcessed);

    UPDATE_FORM(CaptureSequence);
    UPDATE_FORM(CaptureSequenceCancel);
    UPDATE_FORM(CaptureSequenceStatus);

    UPDATE_FORM(CmdMeasureAE);
    UPDATE_FORM(CmdMeasureAECancel);
    UPDATE_FORM(CmdMeasureAEStatus);

    UPDATE_FORM(CmdConvertRaw);

    // display window
    if((eState == AppController::State::Streaming) ||
       (eState == AppController::State::DisplayRaw) ||
       (eState == AppController::State::DisplayProcessed))
    {
        dialogStream->Start();
        _OpenWindow(dialogStream, 10, 10, WindowAlignment_FromWindow);

    }
    else if((eState == AppController::State::StreamStoping) ||
            (eState == AppController::State::Error))
    {
        _CloseWindow(dialogStream);
    }

    // handle the enable/disable of control
    switch(eState)
    {
    case AppController::State::Undefined:
        break;

    case AppController::State::Idle:
        mCmdParameters.applicationState.cfgPathEnable = true;
        formCmdSetConfig->PleaseUpdate();
        break;

    case AppController::State::Opening:
        mCmdParameters.applicationState.cfgPathEnable = false;
        mCmdParameters.applicationState.emulatedCameraEnable = false;
        formCmdSetConfig->PleaseUpdate();
        break;

    case AppController::State::Opened:
        break;

    case AppController::State::SettingUp:
        break;

    case AppController::State::Ready:
        break;

    case AppController::State::Measuring:
        break;

    case AppController::State::MeasureDone:
        break;

    case AppController::State::ExportRaw:
        break;

    case AppController::State::ExportProcessing:
        break;

    case AppController::State::CfgFileReading:
        break;

    case AppController::State::CfgFileWriting:
        break;

    case AppController::State::Closing:
        break;

    case AppController::State::Reseting:
        break;

    case AppController::State::Error:
        break;

    case AppController::State::Streaming:
        break;

    case AppController::State::StreamStoping:
        break;

    case AppController::State::DisplayRaw:
        break;

    case AppController::State::DisplayProcessed:
        break;

    case AppController::State::CapturingSequence:
        break;
    }

    // status color
    StatusColor_t eStatusColor = StatusColor_Error;

    switch(eState)
    {
    case AppController::State::Undefined:
        break;

    case AppController::State::Idle:
    case AppController::State::Opened:
    case AppController::State::Ready:
    case AppController::State::MeasureDone:
    case AppController::State::Streaming:
        eStatusColor = StatusColor_Ok;
        break;

    case AppController::State::Opening:
    case AppController::State::SettingUp:
    case AppController::State::Measuring:
    case AppController::State::ExportRaw:
    case AppController::State::ExportProcessing:
    case AppController::State::CfgFileReading:
    case AppController::State::CfgFileWriting:
    case AppController::State::Closing:
    case AppController::State::Reseting:
    case AppController::State::StreamStoping:
    case AppController::State::DisplayRaw:
    case AppController::State::DisplayProcessed:
    case AppController::State::CapturingSequence:
    case AppController::State::MeasuringAE:
    case AppController::State::MeasuringAECanceling:
        eStatusColor = StatusColor_Processing;
        break;

    case AppController::State::Error:
        eStatusColor = StatusColor_Error;
        break;
    }

    SetStatusColor(eStatusColor);

    // display or hide controls depending on state and configuration
    UpdateFormsVisibility(eState);

    // error case need a pop-up (some user request that)
    if(eState == AppController::State::Error)
    {
        QString errorMessage = mController->GetErrorMessage();

        QMessageBox msgBox;
        msgBox.setText("Error");
        msgBox.setInformativeText("Following error happened: \n" + errorMessage);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

void MainWindow::onWarningMessage(QString message)
{
    if(RESOURCES->mAppConfig.enableWarningMessage == true)
    {
        QMessageBox msgBox;
        msgBox.setText("Warning");
        msgBox.setInformativeText("" + message);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

void MainWindow::dialogStreamFinished()
{
    mController->DisplayStreamStop();
}

void MainWindow::onPleaseUpdateExposureTime(int exposureTimeUs)
{
    mCmdParameters.cmdMeasureConfig.exposureTimeUs = exposureTimeUs;

    formCmdMeasure->PleaseUpdate();
    formCmdMeasureAE->PleaseUpdate();
}

void MainWindow::DisplayVersion()
{
    QString message = mController->GetVersion();

    ToolReturnCode eError = ToolReturnCode(message);

    QMap<QString, QVariant> options = eError.GetOption();

    QString libVersion = QString("%1 %2 (%3)").arg(options[RETURN_ITEM_LIB_NAME].toString(),    -15)
                                            .arg(options[RETURN_ITEM_LIB_VERSION].toString(), -8)
                                            .arg(options[RETURN_ITEM_LIB_DATE].toString(),    -10);

    QString pipVersion = QString("%1 %2 (%3)").arg(options["Pipeline_Name"].toString(),    -15)
                                            .arg(options["Pipeline_Version"].toString(), -8)
                                            .arg(options["Pipeline_Date"].toString(),    -10);

    ui->lblVersion->setText(QString("%1\r\n%2").arg(libVersion).arg(pipVersion));
}

void MainWindow::on_btnCmdOpen()
{
    mController->CmdOpen();
}

void MainWindow::on_btnCmdSetup()
{
    mController->CmdSetup(mCmdParameters.cmdSetupConfig);
}

void MainWindow::on_btnCmdSetupStatus()
{
    SetupStatus_t status;
    mController->CmdSetupStatus(status);
}

void MainWindow::on_btnCmdMeasure()
{
    if(mCmdParameters.debugConfig.emulateCamera == true)
    {
        // the camera is emulated, retrive the capture path
        // and launch the command

        mController->CmdSetConfig(mCmdParameters.debugConfig);
    }

    formDisplayStream->PleaseUpdate();

    mController->CmdMeasure(mCmdParameters.cmdMeasureConfig);
}

void MainWindow::on_btnCmdExportRaw()
{
    mController->CmdExportRaw();
}

void MainWindow::on_btnCmdExportProc()
{
    formDisplayProcessed->PleaseUpdate();

    if(RESOURCES->mAppConfig.bAdmin == false)
    {
        mCmdParameters.cmdProcessingConfig.bBiasCompensation = true;
        mCmdParameters.cmdProcessingConfig.bSensorDefectCorrection = true;
        mCmdParameters.cmdProcessingConfig.bSensorPrnuCorrection = true;
        mCmdParameters.cmdProcessingConfig.bLinearisation = true;
        mCmdParameters.cmdProcessingConfig.bFlatField = true;
        mCmdParameters.cmdProcessingConfig.bAbsolute = true;
    }

    mController->CmdExportProcessed(mCmdParameters.cmdProcessingConfig);
}

void MainWindow::on_btnCmdClose()
{
    mController->CmdClose();
}

void MainWindow::on_btnCmdReset()
{
    mController->CmdReset();
}

#define ALIGN_VALUE(a, b) a - (a % b)
#define IMAGE_SIZE 6001
#define IMAGE_HEIGHT 6004
#define IMAGE_WIDTH  7920
#define CHECK_MAX_VALUE(a, b) if(a > b) a = b

void MainWindow::on_btnCmdSetConfig()
{
    formDisplayStream->PleaseUpdate();

    // check ROI parameters
    bool bUpdateRoiUI = false;

    // check AE measurement area
    mCmdParameters.config.AEMeasAreaHeight        = ALIGN_VALUE(mCmdParameters.config.AEMeasAreaHeight,   4);
    mCmdParameters.config.AEMeasAreaWidth         = ALIGN_VALUE(mCmdParameters.config.AEMeasAreaWidth,   16);
    mCmdParameters.config.AEMeasAreaX             = ALIGN_VALUE(mCmdParameters.config.AEMeasAreaX,       16);
    mCmdParameters.config.AEMeasAreaY             = ALIGN_VALUE(mCmdParameters.config.AEMeasAreaY,        2);

    CHECK_MAX_VALUE(mCmdParameters.config.AEMeasAreaWidth, IMAGE_WIDTH);
    CHECK_MAX_VALUE(mCmdParameters.config.AEMeasAreaHeight, IMAGE_HEIGHT);

    // if the couple size, offset is not correct, change the size of the capture
    if(mCmdParameters.config.AEMeasAreaX + mCmdParameters.config.AEMeasAreaWidth > IMAGE_WIDTH)
    {
        mCmdParameters.config.AEMeasAreaWidth = IMAGE_WIDTH - mCmdParameters.config.AEMeasAreaX;
    }

    if(mCmdParameters.config.AEMeasAreaY + mCmdParameters.config.AEMeasAreaHeight > IMAGE_HEIGHT)
    {
        mCmdParameters.config.AEMeasAreaHeight = IMAGE_HEIGHT - mCmdParameters.config.AEMeasAreaY;
    }

    formCmdSetAEMeasArea->PleaseUpdate();

    // check ROI
    CHECK_MAX_VALUE(mCmdParameters.config.RoiXLeft, IMAGE_SIZE);
    CHECK_MAX_VALUE(mCmdParameters.config.RoiXRight, IMAGE_SIZE);
    CHECK_MAX_VALUE(mCmdParameters.config.RoiYTop, IMAGE_SIZE);
    CHECK_MAX_VALUE(mCmdParameters.config.RoiYBottom, IMAGE_SIZE);

    if(mCmdParameters.config.RoiXRight < mCmdParameters.config.RoiXLeft)
    {
        mCmdParameters.config.RoiXRight = mCmdParameters.config.RoiXLeft;
        bUpdateRoiUI = true;
    }

    if(mCmdParameters.config.RoiYBottom < mCmdParameters.config.RoiYTop)
    {
        mCmdParameters.config.RoiYBottom = mCmdParameters.config.RoiYTop;
        bUpdateRoiUI = true;
    }

    if(mCmdParameters.config.bUseRoi == true)
    {
        if((bUpdateRoiUI == true) &&
           ((mCmdParameters.config.RoiXRight == mCmdParameters.config.RoiXLeft) ||
            (mCmdParameters.config.RoiYBottom == mCmdParameters.config.RoiYTop)))
        {
            // add a warning
            onLog("\n** WARNING ** Please check ROI parameters\n");
        }
    }

    if(bUpdateRoiUI == true)
    {
        formCmdSetRoi->PleaseUpdate();
    }

    mController->CmdSetConfig(mCmdParameters.config);
    mController->CmdSetConfig(mCmdParameters.debugConfig);

    formCmdMeasure->PleaseUpdate();
    formCaptureSequence->PleaseUpdate();

    // UpdateFormsVisibility(AppController::State::Undefined);
}

void MainWindow::on_btnCmdCfgFileWrite()
{
    mController->CmdCfgFileWrite();
}

void MainWindow::on_btnCmdCfgFileRead()
{
    mController->CmdCfgFileRead();
}

void MainWindow::on_btnCmdCfgFileStatus()
{
    mController->CmdCfgFileStatus();
}

void MainWindow::on_btnDisplayStream()
{
    dialogStream->ClearScreen();

    formCmdMeasure->PleaseUpdate();

    // update application configuration
    if(RESOURCES->mAppConfig.bAdmin == false)
    {
        mCmdParameters.applicationConfig.bStreamProcessedData = true;
    }

    RESOURCES->mAppConfig.bStreamProcessedData = mCmdParameters.applicationConfig.bStreamProcessedData;
    RESOURCES->mAppConfig.autoExposure = mCmdParameters.applicationConfig.autoExposure;

    RESOURCES->SaveAppConfig();

    mController->DisplayStream(mCmdParameters.cmdMeasureConfig.exposureTimeUs,
                               mCmdParameters.applicationConfig.bStreamProcessedData,
                               mCmdParameters.applicationConfig.autoExposure,
                               mCmdParameters.config.AELevelPercent);
}

void MainWindow::on_btnDisplayRaw()
{
    dialogStream->ClearScreen();

    mController->DisplayRaw();
}

void MainWindow::on_btnDisplayProcessed()
{
    dialogStream->ClearScreen();

    formCmdExportProc->PleaseUpdate();

    mController->DisplayProcessed(mCmdParameters.cmdProcessingConfig);
}

void MainWindow::on_btnCaptureSequence()
{
    mController->CaptureSequence(mCmdParameters.cmdCaptureSequenceConfig);
}

void MainWindow::on_btnCaptureSequenceCancel()
{
    mController->CaptureSequenceCancel();
}

void MainWindow::on_btnCaptureSequenceStatus()
{
    mController->CaptureSequenceStatus();
}

void MainWindow::on_btnMeasureAE()
{
    mController->MeasureAE(mCmdParameters.cmdMeasureConfig);
}

void MainWindow::on_btnMeasureAECancel()
{
    mController->MeasureAECancel();
}

void MainWindow::on_btnMeasureAEStatus()
{
    mController->MeasureAEStatus();
}

void MainWindow::on_btnConvertRaw()
{
    mController->ConvertRaw(mCmdParameters.cmdConvertRaw);
}

void MainWindow::PleaseUpdateCmdControl()
{
    for(int index = 0; index < mFormList.count(); index ++)
    {
        mFormList[index]->PleaseUpdate();
    }
}

void MainWindow::_OpenWindow(QWidget* window, int xOffset, int yOffset, WindowAlignment_t alignment)
{
    if(window != NULL)
    {
        static bool bSetPosition = true;

        // set position only once
        if(bSetPosition == true)
        {
            QRect rw = window->geometry();

            QRect r = geometry();
            int xPos = r.x();
            int yPos = r.y();

            switch(alignment)
            {
                case WindowAlignment_LeftFromWindow:
                    // alignment on left
                    // x<--width--><--offset-->[window]
                    xPos -= (rw.width() + xOffset);
                    break;

                case WindowAlignment_RightFromWindow:
                    // alignment on right
                    // [ window  ]
                    // <--width--><--offset-->x
                    xPos += (r.width() + xOffset);
                    break;

                case WindowAlignment_FromWindow:
                    xPos += xOffset;
                    break;
            }

            if(xPos < 0)
            {
                xPos = 0;
            }

            yPos += yOffset;

            window->move(xPos, yPos);

            bSetPosition = false;
        }

        window->setWindowModality(Qt::NonModal);
        window->setWindowFlags(Qt::Window);
        window->setWindowTitle(APPLICATION_TITLE);

        window->show();
        window->activateWindow();
        window->raise();
    }
}

void MainWindow::_CloseWindow(QWidget* window)
{
    window->hide();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    //! Ignore the event by default.. otherwise the window will be closed always.
    event->ignore();

    _CloseWindow(dialogStream);

    event->accept();
}

#define FORM_COLOR_OK         QColor(51, 245, 142)
#define FORM_COLOR_PROCESSING QColor(245, 135, 51)
#define FORM_COLOR_ERROR      QColor(245, 51, 51)

QMap<StatusColor_t, QColor> _ColorMap = {
    {StatusColor_Ok,         FORM_COLOR_OK},
    {StatusColor_Processing, FORM_COLOR_PROCESSING},
    {StatusColor_Error,      FORM_COLOR_ERROR}};

void MainWindow::SetStatusColor(StatusColor_t eColor)
{
    QPalette pal;
    QColor statusColor = _ColorMap[eColor];

    pal.setColor(QPalette::Background, statusColor);
    ui->frameStatus->setAutoFillBackground(true);
    ui->frameStatus->setPalette(pal);
}

void MainWindow::UpdateFormsVisibility(AppController::State eState)
{
    // depending on configuration (emulated camera)
    // and state, hide some commands

    if(mCmdParameters.debugConfig.emulateCamera == true)
    {
        formCmdMeasureAE->hide();
        formCmdMeasureAECancel->hide();
        formCmdMeasureAEStatus->hide();
    }
    else
    {
        formCmdMeasureAE->show();

        if(eState == AppController::State::MeasuringAE)
        {
            formCmdMeasureAECancel->show();
            formCmdMeasureAEStatus->show();
        }
        else
        {
            formCmdMeasureAECancel->hide();
            formCmdMeasureAEStatus->hide();
        }

        if(eState == AppController::State::CapturingSequence)
        {
            formCaptureSequenceCancel->show();
            formCaptureSequenceStatus->show();
        }
        else
        {
            formCaptureSequenceCancel->hide();
            formCaptureSequenceStatus->hide();
        }
    }
}
