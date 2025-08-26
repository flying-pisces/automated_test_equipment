#include "formCmd.h"
#include "ui_formCmd.h"

#include "appResource.h"

// #define FORM_COLOR QColor(255, 54, 33)
#define FORM_COLOR_0 QColor(220, 220, 220)
#define FORM_COLOR_2 QColor(242, 141, 82)
#define FORM_COLOR_1 QColor(106, 196, 230)
#define FORM_COLOR_3 QColor(245, 216, 122)

#define FORM_COLOR_5 QColor(242, 138, 138)

#define FORM_COLOR_4 QColor(180, 190, 200)
#define FORM_COLOR_6 QColor(190, 200, 210)
#define FORM_COLOR_7 QColor(200, 210, 220)
#define FORM_COLOR_8 QColor(210, 220, 230)

QList<QColor> _ColorMap = {FORM_COLOR_0, FORM_COLOR_1, FORM_COLOR_2, FORM_COLOR_3, FORM_COLOR_4, FORM_COLOR_5, FORM_COLOR_6, FORM_COLOR_7, FORM_COLOR_8};

FormCmd::FormCmd(CmdType eCmdType, CmdParameters_t *params, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCmd)
{
    ui->setupUi(this);

    mParameters = params;

    QString ctrlName = EnumToString("CmdType", (int)eCmdType);

    ui->btn->setText(ctrlName);
    ui->label->setText(ctrlName);
    ui->label->hide();

    // setup
    //FormCmdParam::ParamSetting paramTimeoutMs           = FormCmdParam::ParamSetting(-1, TIMEOUT_MAX, 1);
    FormCmdParam::ParamSetting paramSensorTemperature   = FormCmdParam::ParamSetting(TEMP_MIN, TEMP_MAX, 1);

    // measure
    //FormCmdParam::ParamSetting paramTimeoutMs_    = FormCmdParam::ParamSetting(-1, TIMEOUT_MAX, 1);
    FormCmdParam::ParamSetting paramExposureTimeUs= FormCmdParam::ParamSetting(0, EXPOSURE_TIME_MAX, 1);
    FormCmdParam::ParamSetting paramNbAcquisition = FormCmdParam::ParamSetting(0, NB_ACQUISITION_MAX, 1);
    //FormCmdParam::ParamSetting paramBinningFactor = FormCmdParam::ParamSetting(0, NB_BINNING_MAX, 1);

    FormCmdParam::ParamSetting paramAEThresholdTimeUs= FormCmdParam::ParamSetting(10, EXPOSURE_TIME_MAX, 1);

    FormCmdParam::ParamSetting paramROI= FormCmdParam::ParamSetting(0, 6001, 1);

    FormCmdParam::ParamSetting paramWidth   = FormCmdParam::ParamSetting(0, 7920, 1);
    FormCmdParam::ParamSetting paramHeight  = FormCmdParam::ParamSetting(0, 6004, 1);

    // stream
    FormCmdParam::ParamSetting paramAELevelPercent = FormCmdParam::ParamSetting(0, 100, 1);

    switch(eCmdType)
    {
    case CmdType::Open:
    case CmdType::SetupStatus:
    case CmdType::ExportRaw:
    case CmdType::Close:
    case CmdType::Reset:
    case CmdType::CfgFileWrite:
    case CmdType::CfgFileRead:
    case CmdType::CfgFileStatus:
    case CmdType::DisplayRaw:
    case CmdType::CaptureSequenceCancel:
    case CmdType::CaptureSequenceStatus:
    default:
        _ParamAppend(new FormCmdParam(this));
        break;

    case CmdType::Setup:
        _ParamAppend(new FormCmdParam("sensorTemperature",  &params->cmdSetupConfig.sensorTemperature, paramSensorTemperature, this));
        _ParamAppend(new FormCmdParam("filter",       (int*)&params->cmdSetupConfig.eFilter,           RESOURCES->mFilterToStringMap, this));
        _ParamAppend(new FormCmdParam("nd",           (int*)&params->cmdSetupConfig.eNd,               RESOURCES->mNdToStringMap, this));
        _ParamAppend(new FormCmdParam("irisIndex",    (int*)&params->cmdSetupConfig.eIris,             RESOURCES->mIrisIndexToStringMap, this));
        break;

    case CmdType::Measure:
        _ParamAppend(new FormCmdParam("exposureTimeUs", &params->cmdMeasureConfig.exposureTimeUs, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("nbAcquisition", &params->cmdMeasureConfig.nbAcquisition, paramNbAcquisition, this));

        if(RESOURCES->mAppConfig.bAdmin == true)
        {
            _ParamAppend(new FormCmdParam("bTestPattern",   &params->cmdMeasureConfig.bTestPattern, this));
        }

        _ParamAppend(new FormCmdParam("path",    &params->debugConfig.dummyRawImagePath, this));

        _ParamHideCondition("exposureTimeUs", &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("nbAcquisition", &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("bTestPattern", &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("path", &params->debugConfig.emulateCamera, true);

        break;

    case CmdType::ExportProcessed:
        if(RESOURCES->mAppConfig.bAdmin == true)
        {
            _ParamAppend(new FormCmdParam("bBiasCompensation",            &params->cmdProcessingConfig.bBiasCompensation, this));
            _ParamAppend(new FormCmdParam("bSensorDefectCorrection",      &params->cmdProcessingConfig.bSensorDefectCorrection, this));
            _ParamAppend(new FormCmdParam("bSensorPrnuCorrection",        &params->cmdProcessingConfig.bSensorPrnuCorrection, this));
            _ParamAppend(new FormCmdParam("bLinearisation",               &params->cmdProcessingConfig.bLinearisation, this));
            _ParamAppend(new FormCmdParam("bFlatField",                   &params->cmdProcessingConfig.bFlatField, this));
            _ParamAppend(new FormCmdParam("bAbsolute",                    &params->cmdProcessingConfig.bAbsolute, this));
        }
        else
        {
            _ParamAppend(new FormCmdParam(this));
        }
        break;

    case CmdType::SetConfig:
        _ParamAppend(new FormCmdParam("cfgPath",    &params->config.cfgPath, this));
        _ParamAppend(new FormCmdParam("capturePath", &params->config.capturePath, this));

        _ParamAppend(new FormCmdParam("fileNamePrepend", &params->config.fileNamePrepend, this));
        _ParamAppend(new FormCmdParam("fileNameAppend", &params->config.fileNameAppend, this));
        _ParamAppend(new FormCmdParam("exportFileNameFormat", &params->config.exportFileNameFormat, this));

        _ParamAppend(new FormCmdParam("exportFormat", (int*)&params->config.exportFormat, RESOURCES->mExportFormatToStringMap, this));

        _ParamAppend(new FormCmdParam("emulatedCamera", &params->debugConfig.emulateCamera, this));
        if(RESOURCES->mAppConfig.bAdmin == true)
        {
            _ParamAppend(new FormCmdParam("emulatedWheel",  &params->debugConfig.emulateWheel, this));
        }

        _ParamEnableCondition("cfgPath", &params->applicationState.cfgPathEnable);
        _ParamEnableCondition("emulatedCamera", &params->applicationState.emulatedCameraEnable);
        break;

    case CmdType::AE:
        _ParamAppend(new FormCmdParam("AEMinExpoTimeUs", &params->config.AEMinExpoTimeUs, paramAEThresholdTimeUs, this));
        _ParamAppend(new FormCmdParam("AEMaxExpoTimeUs", &params->config.AEMaxExpoTimeUs, paramAEThresholdTimeUs, this));
        _ParamAppend(new FormCmdParam("AEExpoTimeGranularityUs", &params->config.AEExpoTimeGranularityUs, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("AELevel (percent)", &params->config.AELevelPercent, paramAELevelPercent, this));
        ui->btn->hide();
        ui->label->show();
        break;

    case CmdType::AEMeasArea:
        _ParamAppend(new FormCmdParam("AEMeasAreaWidth",  &params->config.AEMeasAreaWidth,  paramWidth, this));
        _ParamAppend(new FormCmdParam("AEMeasAreaHeight", &params->config.AEMeasAreaHeight, paramHeight, this));
        _ParamAppend(new FormCmdParam("AEMeasAreaX",      &params->config.AEMeasAreaX,      paramWidth, this));
        _ParamAppend(new FormCmdParam("AEMeasAreaY",      &params->config.AEMeasAreaY,      paramHeight, this));

        ui->btn->hide();
        ui->label->show();
        break;

    case CmdType::ROI:
        _ParamAppend(new FormCmdParam("bUseRoi",   &params->config.bUseRoi, this));

        _ParamAppend(new FormCmdParam("XLeft",     &params->config.RoiXLeft, paramROI, this));
        _ParamAppend(new FormCmdParam("XRight",    &params->config.RoiXRight, paramROI,this));
        _ParamAppend(new FormCmdParam("YTop",      &params->config.RoiYTop, paramROI,this));
        _ParamAppend(new FormCmdParam("YBottom",   &params->config.RoiYBottom, paramROI,this));

        _ParamHideCondition("XLeft", &params->config.bUseRoi);
        _ParamHideCondition("XRight", &params->config.bUseRoi);
        _ParamHideCondition("YTop", &params->config.bUseRoi);
        _ParamHideCondition("YBottom", &params->config.bUseRoi);

        ui->btn->hide();
        ui->label->show();
        break;

    case FormCmd::CmdType::DisplayStream:
        _ParamAppend(new FormCmdParam("AE", &params->applicationConfig.autoExposure, this));

        if(RESOURCES->mAppConfig.bAdmin == true)
        {
            _ParamAppend(new FormCmdParam("bProcessed", &params->applicationConfig.bStreamProcessedData, this));
        }
        _ParamAppend(new FormCmdParam("exposureTimeUs", &params->cmdMeasureConfig.exposureTimeUs, paramExposureTimeUs, this));
        break;

    case CmdType::DisplayProcessed:
        if(RESOURCES->mAppConfig.bAdmin == true)
        {
            _ParamAppend(new FormCmdParam("bBiasCompensation",            &params->cmdProcessingConfig.bBiasCompensation, this));
            _ParamAppend(new FormCmdParam("bSensorDefectCorrection",      &params->cmdProcessingConfig.bSensorDefectCorrection, this));
            _ParamAppend(new FormCmdParam("bSensorPrnuCorrection",        &params->cmdProcessingConfig.bSensorPrnuCorrection, this));
            _ParamAppend(new FormCmdParam("bLinearisation",               &params->cmdProcessingConfig.bLinearisation, this));
            _ParamAppend(new FormCmdParam("bFlatField",                   &params->cmdProcessingConfig.bFlatField, this));
            _ParamAppend(new FormCmdParam("bAbsolute",                    &params->cmdProcessingConfig.bAbsolute, this));
        }
        else
        {
            _ParamAppend(new FormCmdParam(this));
        }
        break;

    case CmdType::CaptureSequence:
        _ParamAppend(new FormCmdParam("sensorTemperature",  &params->cmdCaptureSequenceConfig.sensorTemperature, paramSensorTemperature, this));
        _ParamAppend(new FormCmdParam("waitForTemp",        &params->cmdCaptureSequenceConfig.bWaitForSensorTemperature, this));
        _ParamAppend(new FormCmdParam("nd",           (int*)&params->cmdCaptureSequenceConfig.eNd, RESOURCES->mNdToStringMap, this));
        _ParamAppend(new FormCmdParam("irisIndex",    (int*)&params->cmdCaptureSequenceConfig.eIris, RESOURCES->mIrisIndexToStringMap, this));
        _ParamAppend(new FormCmdParam("autoExpo",           &params->cmdCaptureSequenceConfig.bAutoExposure, this));
        _ParamAppend(new FormCmdParam("useExpoFile",        &params->cmdCaptureSequenceConfig.bUseExpoFile, this));

        _ParamAppend(new FormCmdParam("exposureTimeUs FilterX",     &params->cmdCaptureSequenceConfig.exposureTimeUs_FilterX, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("exposureTimeUs FilterXz",    &params->cmdCaptureSequenceConfig.exposureTimeUs_FilterXz, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("exposureTimeUs FilterYa",    &params->cmdCaptureSequenceConfig.exposureTimeUs_FilterYa, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("exposureTimeUs FilterYb",    &params->cmdCaptureSequenceConfig.exposureTimeUs_FilterYb, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("exposureTimeUs FilterZ",     &params->cmdCaptureSequenceConfig.exposureTimeUs_FilterZ, paramExposureTimeUs, this));

        _ParamAppend(new FormCmdParam("nbAcquisition",      &params->cmdCaptureSequenceConfig.nbAcquisition, paramNbAcquisition, this));
        _ParamAppend(new FormCmdParam("bSaveCaptures",      &params->cmdCaptureSequenceConfig.bSaveCapture, this));

        // used to keep button the right size when all other params are gone
        _ParamAppend(new FormCmdParam("dummy"));

        _ParamHideCondition("useExpoFile",      &params->cmdCaptureSequenceConfig.bAutoExposure, false);

        _ParamHideCondition("exposureTimeUs FilterX",  &params->cmdCaptureSequenceConfig.bAutoExposure, false);
        _ParamHideCondition("exposureTimeUs FilterX",  &params->cmdCaptureSequenceConfig.bUseExpoFile, false);

        _ParamHideCondition("exposureTimeUs FilterXz", &params->cmdCaptureSequenceConfig.bAutoExposure, false);
        _ParamHideCondition("exposureTimeUs FilterXz", &params->cmdCaptureSequenceConfig.bUseExpoFile, false);

        _ParamHideCondition("exposureTimeUs FilterYa", &params->cmdCaptureSequenceConfig.bAutoExposure, false);
        _ParamHideCondition("exposureTimeUs FilterYa", &params->cmdCaptureSequenceConfig.bUseExpoFile, false);

        _ParamHideCondition("exposureTimeUs FilterYb", &params->cmdCaptureSequenceConfig.bAutoExposure, false);
        _ParamHideCondition("exposureTimeUs FilterYb", &params->cmdCaptureSequenceConfig.bUseExpoFile, false);

        _ParamHideCondition("exposureTimeUs FilterZ",  &params->cmdCaptureSequenceConfig.bAutoExposure, false);
        _ParamHideCondition("exposureTimeUs FilterZ",  &params->cmdCaptureSequenceConfig.bUseExpoFile, false);

        _ParamHideCondition("sensorTemperature", &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("waitForTemp",       &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("nd",                &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("irisIndex",         &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("autoExpo",          &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("useExpoFile",       &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("exposureTimeUs",    &params->debugConfig.emulateCamera, false);
        _ParamHideCondition("nbAcquisition",     &params->debugConfig.emulateCamera, false);

        _ParamHideCondition("dummy",     &params->debugConfig.emulateCamera, true);

         break;

    case CmdType::MeasureAE:
        _ParamAppend(new FormCmdParam("exposureTimeUs", &params->cmdMeasureConfig.exposureTimeUs, paramExposureTimeUs, this));
        _ParamAppend(new FormCmdParam("nbAcquisition", &params->cmdMeasureConfig.nbAcquisition, paramNbAcquisition, this));
        break;

    case CmdType::MeasureAECancel:
        _ParamAppend(new FormCmdParam(this));
        break;

    case CmdType::MeasureAEStatus:
        _ParamAppend(new FormCmdParam(this));
        break;

    case CmdType::ConvertRaw:
        _ParamAppend(new FormCmdParam("FileName", &params->cmdConvertRaw.fileName, this));
        break;
    }

    // configure back color
    switch(eCmdType)
    {
    case CmdType::CfgFileWrite:
    case CmdType::CfgFileRead:
    case CmdType::CfgFileStatus:
        _SetColor(1);
        break;

    case FormCmd::CmdType::DisplayStream:
    case FormCmd::CmdType::DisplayRaw:
    case FormCmd::CmdType::DisplayProcessed:
        _SetColor(2);
        break;

    case FormCmd::CmdType::CaptureSequence:
    case FormCmd::CmdType::CaptureSequenceCancel:
    case FormCmd::CmdType::CaptureSequenceStatus:
        _SetColor(3);
        break;

    case FormCmd::CmdType::SetConfig:
        _SetColor(8);
        break;

    case FormCmd::CmdType::AE:
        _SetColor(7);
        break;

    case FormCmd::CmdType::AEMeasArea:
        _SetColor(6);
        break;

    case FormCmd::CmdType::ROI:
        _SetColor(4);
        break;

    case FormCmd::CmdType::MeasureAE:
    case FormCmd::CmdType::MeasureAECancel:
    case FormCmd::CmdType::MeasureAEStatus:
        _SetColor(5);
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
    FormCmdParam::UpdateStatus status;

    for(int index = 0; index < mForms.count(); index ++)
    {
        status = mForms[index]->PleaseUpdate();

        switch(status)
        {
        case FormCmdParam::UpdateStatus::Disable:
            mForms[index]->SetEnable(false);
            break;

        case FormCmdParam::UpdateStatus::Hidden:
            mForms[index]->hide();
            break;

        case FormCmdParam::UpdateStatus::Normal:
        default:
            mForms[index]->show();
            mForms[index]->SetEnable(true);
            break;
        }
    }
}

void FormCmd::_SetColor(int index)
{
    QPalette pal;
    pal.setColor(QPalette::Background, _ColorMap[index]);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

void FormCmd::_ParamAppend(FormCmdParam* paramForm)
{
    mForms.append(paramForm);

    connect(paramForm, &FormCmdParam::on_Change,
            this, &FormCmd::UpdateUi);
}

FormCmdParam* FormCmd::_FindParamByName(QString paramName)
{
    FormCmdParam* paramForm = nullptr;
    int index = 0;

    while((index < mForms.count()) && (paramForm == nullptr))
    {
        paramForm = mForms.at(index);

        if(paramForm->mName != paramName)
        {
            paramForm = nullptr;
        }

        index ++;
    }

    return paramForm;
}

void FormCmd::_ParamHideCondition(QString paramName, bool *bCondition, bool bValue)
{
    FormCmdParam* paramForm = _FindParamByName(paramName);

    if(paramForm != nullptr)
    {
        paramForm->SetHideCondition(bCondition, bValue);
    }
}

void FormCmd::_ParamEnableCondition(QString paramName, bool *bCondition, bool bValue)
{
    FormCmdParam* paramForm = _FindParamByName(paramName);

    if(paramForm != nullptr)
    {
        paramForm->SetEnableCondition(bCondition, bValue);
    }
}

