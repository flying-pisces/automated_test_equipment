#include "formCmd.h"
#include "ui_formCmd.h"

#include "appResource.h"

// #define FORM_COLOR QColor(255, 54, 33)
#define FORM_COLOR_0 QColor(220, 220, 220)
#define FORM_COLOR_1 QColor(255, 205, 3)
#define FORM_COLOR_2 QColor(56, 222, 255)

QList<QColor> _ColorMap = {FORM_COLOR_0, FORM_COLOR_1, FORM_COLOR_2};

FormCmd::FormCmd(CmdType eCmdType, CmdParameters_t *params, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCmd)
{
    ui->setupUi(this);

    mParameters = params;

    QString ctrlName = EnumToString("CmdType", (int)eCmdType);

    ui->btn->setText(ctrlName);

    // setup
    FormCmdParam::ParamSetting paramTimeoutMs           = FormCmdParam::ParamSetting(-1, TIMEOUT_MAX, 1);
    FormCmdParam::ParamSetting paramSensorTemperature   = FormCmdParam::ParamSetting(TEMP_MIN, TEMP_MAX, 1);
    // FormCmdParam::ParamSetting paramFilterWheelPosition = FormCmdParam::ParamSetting(0, FILT_WHEEL_MAX, 1);
    // FormCmdParam::ParamSetting paramNdWheelPosition     = FormCmdParam::ParamSetting(0, ND_WHEEL_MAX, 1);
    // FormCmdParam::ParamSetting paramIrisIndex           = FormCmdParam::ParamSetting(0, IRIS_MAX, 1);

    // measure
    FormCmdParam::ParamSetting paramTimeoutMs_    = FormCmdParam::ParamSetting(-1, TIMEOUT_MAX, 1);
    FormCmdParam::ParamSetting paramExposureTimeUs= FormCmdParam::ParamSetting(0, EXPOSURE_TIME_MAX, 1);
    FormCmdParam::ParamSetting paramNbAcquisition = FormCmdParam::ParamSetting(0, NB_ACQUISITION_MAX, 1);
    FormCmdParam::ParamSetting paramBinningFactor = FormCmdParam::ParamSetting(0, NB_BINNING_MAX, 1);

    switch(eCmdType)
    {
    case CmdType::Reset:
    case CmdType::Test1:
    case CmdType::Test2:
    case CmdType::TestSetup:
    case CmdType::ProcessRawData:
    case CmdType::TestMeasureAE:
    case CmdType::TestCapture:
    case CmdType::TestCaptureSequence:

    case CmdType::TestAbort:
    default:
        mForms.append(new FormCmdParam(this));
        break;

    case CmdType::SetConfig:
        mForms.append(new FormCmdParam("cfgPath",    &params->cmdSetConfig.cfgPath, this));
        mForms.append(new FormCmdParam("capturePath", &params->cmdSetConfig.capturePath, this));
        break;
    }

    // configure back color
    switch(eCmdType)
    {
    case CmdType::Test1:
    case CmdType::Test2:
    case CmdType::TestSetup:
    case CmdType::TestAbort:
        _SetColor(1);
        break;

    case CmdType::ProcessRawData:
        _SetColor(2);
        break;

    case CmdType::TestMeasureAE:
    case CmdType::TestCapture:
    case CmdType::TestCaptureSequence:
        _SetColor(1);
        break;

    default:
        _SetColor();
        break;
    }

    for(int index = 0; index < mForms.count(); index ++)
    {
        ui->valueLayout->addWidget(mForms[index]);
    }

    connect(ui->btn, &QPushButton::clicked,
            this, &FormCmd::ExecuteCmd);
}

FormCmd::~FormCmd()
{
    delete ui;

    for(int index = 0; index < mForms.count(); index ++)
    {
        delete mForms[index];
    }
}

void FormCmd::UpdateUi()
{
    PleaseUpdate();
}

void FormCmd::PleaseUpdate()
{
    for(int index = 0; index < mForms.count(); index ++)
    {
        mForms[index]->PleaseUpdate();
    }
}

void FormCmd::_SetColor(int index)
{
    QPalette pal;
    pal.setColor(QPalette::Background, _ColorMap[index]);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

