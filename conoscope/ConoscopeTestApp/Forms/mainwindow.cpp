#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "configuration.h"

#include "toolReturnCode.h"
#include "toolString.h"
#include "formCmd.h"

#define UPDATE_BTN(a) ui->btn##a->setEnabled(b_##a)
#define UPDATE_FORM(a) form##a->setEnabled(b_##a)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //setWindowTitle(APPLICATION_TITLE);

    connect(ui->btn, &QPushButton::clicked,
            this, &MainWindow::on_GeneralPurpose_clicked);

    connect(ui->btnGetVersion, &QPushButton::clicked,
            this, &MainWindow::on_btnGetVersion);

    // create the forms for the buttons
    // note mParameter will be updated by the ui

    formCmdSetConfig     = new FormCmd(FormCmd::CmdType::SetConfig, &mCmdParameters, this);
    formCmdReset = new FormCmd(FormCmd::CmdType::Reset, &mCmdParameters, this);

    formTest1 = new FormCmd(FormCmd::CmdType::Test1, &mCmdParameters, this);
    formTest2 = new FormCmd(FormCmd::CmdType::Test2, &mCmdParameters, this);
    formTestSetup = new FormCmd(FormCmd::CmdType::TestSetup, &mCmdParameters, this);
    formTestMeasureAE = new FormCmd(FormCmd::CmdType::TestMeasureAE, &mCmdParameters, this);
    formTestCapture = new FormCmd(FormCmd::CmdType::TestCapture, &mCmdParameters, this);
    formTestCaptureSequence = new FormCmd(FormCmd::CmdType::TestCaptureSequence, &mCmdParameters, this);
    formProcessRawData = new FormCmd(FormCmd::CmdType::ProcessRawData, &mCmdParameters, this);

    formTestAbort = new FormCmd(FormCmd::CmdType::TestAbort, &mCmdParameters, this);

    mFormList.append(formCmdSetConfig);
    mFormList.append(formCmdReset);
    mFormList.append(formTest1);
    mFormList.append(formTest2);
    mFormList.append(formTestSetup);
    mFormList.append(formTestMeasureAE);
    mFormList.append(formTestCapture);
    mFormList.append(formTestCaptureSequence);
    mFormList.append(formProcessRawData);

    mFormList.append(formTestAbort);

    verticalSpacer = new QSpacerItem(20, 400, QSizePolicy::Minimum, QSizePolicy::Expanding);

    for(int index = 0; index < mFormList.count(); index ++)
    {
        ui->verticalLayout->addWidget(mFormList[index]);
    }

    ui->verticalLayout->addItem(verticalSpacer);

    // register the action to do when the button is pressed

    connect(formCmdSetConfig, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnCmdSetConfig);

    connect(formCmdReset, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnCmdReset);

    connect(formTest1, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTest1);

    connect(formTest2, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTest2);

    connect(formTestSetup, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTestSetup);

    connect(formTestMeasureAE, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTestMeasureAE);

    connect(formTestCapture, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTestCapture);

    connect(formTestCaptureSequence, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTestCaptureSequence);

    connect(formProcessRawData, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnProcessRawData);

    connect(formTestAbort, &FormCmd::ExecuteCmd,
            this, &MainWindow::on_btnTestAbort);

    mController = new AppController();

    connect(mController, &AppController::WriteLog,
            this, &MainWindow::onLog);

    connect(mController, &AppController::WriteStatus,
            this, &MainWindow::onStatus);

    connect(mController, &AppController::StateChange,
            this, &MainWindow::onStateChange);

    // buffer used for streaming
    mController->Start();

    on_btnGetVersion();

    // retrieve current parameters
    mController->CmdGetConfig(mCmdParameters.cmdSetupConfig,
                              mCmdParameters.cmdMeasureConfig,
                              mCmdParameters.cmdProcessingConfig,
                              mDebugSettings);


    QString windowTitle = APPLICATION_TITLE;

    if(mDebugSettings.debugMode == true)
    {
        windowTitle = windowTitle + " | DEBUG mode";
    }

    if(mDebugSettings.emulateCamera == true)
    {
        windowTitle = windowTitle + " | emulated camera";
    }

    setWindowTitle(windowTitle);

    mController->CmdGetConfig(mCmdParameters.cmdSetConfig);

    // request ui to update settings according to configuration
    for(int index = 0; index < mFormList.count(); index ++)
    {
        mFormList[index]->PleaseUpdate();
    }
}

MainWindow::~MainWindow()
{
    mController->Stop();

    delete mController;

    for(int index = 0; index < mFormList.count(); index ++)
    {
        ui->verticalLayout->removeWidget(mFormList[index]);
    }

    ui->verticalLayout->removeItem(verticalSpacer);

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

    bool b_CmdSetConfig = false;
    bool b_ProcessRawData = false;

    bool b_CmdReset = false;
    bool b_Test1 = false;
    bool b_Test2 = false;
    bool b_TestSetup = false;
    bool b_TestAbort = false;
    bool b_TestMeasureAE = false;
    bool b_TestCapture = false;
    bool b_TestCaptureSequence = false;

    switch(eState)
    {
    case AppController::State::Idle:
        b_CmdSetConfig = true;
        b_ProcessRawData = true;
        b_CmdReset = true;
        b_Test1 = true;
        b_Test2 = true;
        b_TestSetup = true;
        b_TestMeasureAE = true;
        b_TestCapture = true;
        b_TestCaptureSequence = true;
        break;

    case AppController::State::Test1:
    case AppController::State::Test2:
    case AppController::State::TestSetup:
    case AppController::State::TestMeasureAE:
    case AppController::State::TestCapture:
    case AppController::State::TestCaptureSequence:
    case AppController::State::ProcessRawData:
        b_TestAbort = true;
        break;

    case AppController::State::Error:
        break;

    default:
        break;
    }

    UPDATE_FORM(CmdSetConfig);
    UPDATE_FORM(ProcessRawData);

    UPDATE_FORM(CmdReset);
    UPDATE_FORM(Test1);
    UPDATE_FORM(Test2);
    UPDATE_FORM(TestSetup);
    UPDATE_FORM(TestMeasureAE);
    UPDATE_FORM(TestCapture);
    UPDATE_FORM(TestCaptureSequence);
    UPDATE_FORM(TestAbort);
}

void MainWindow::on_GeneralPurpose_clicked()
{
}

void MainWindow::on_btnGetVersion()
{
    QString message = mController->GetVersion();

    ToolReturnCode eError = ToolReturnCode(message);

    QMap<QString, QVariant> options = eError.GetOption();

    ui->txtInfo->setText("DLL version\r\n");

    QMapIterator<QString, QVariant> iter(options);
    while (iter.hasNext())
    {
        iter.next();

        QString iKey = ToolsString::FixedLength(iter.key(), 20);
        QString iValue = iter.value().toString();

        ui->txtInfo->append(QString("%1 %2").arg(iKey).arg(iValue));
    }
}

void MainWindow::on_btnCmdSetConfig()
{
    mController->CmdSetConfig(mCmdParameters.cmdSetConfig);
}

void MainWindow::on_btnCmdReset()
{
    mController->CmdReset();
}

void MainWindow::on_btnTest1()
{
    mController->Test1();
}

void MainWindow::on_btnTest2()
{
    mController->Test2();
}

void MainWindow::on_btnTestSetup()
{
    mController->TestSetup();
}

void MainWindow::on_btnProcessRawData()
{
    mController->ProcessRawData();
}

void MainWindow::on_btnTestMeasureAE()
{
    mController->TestMeasureAE();
}

void MainWindow::on_btnTestCapture()
{
    mController->TestCapture();
}

void MainWindow::on_btnTestCaptureSequence()
{
    mController->TestCaptureSequence();
}

void MainWindow::on_btnTestAbort()
{
    mController->TestAbort();
}

void MainWindow::PleaseUpdateCmdControl()
{
    for(int index = 0; index < mFormList.count(); index ++)
    {
        mFormList[index]->PleaseUpdate();
    }
}
