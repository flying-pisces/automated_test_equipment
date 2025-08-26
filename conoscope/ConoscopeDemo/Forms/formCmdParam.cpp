#include "formCmdParam.h"
#include "ui_formCmdParam.h"

// #define FORM_COLOR QColor(255, 100, 84)
#define FORM_COLOR QColor(240, 240, 240)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

FormCmdParam::FormCmdParam(QWidget *parent) :
    QWidget(parent),
    mName(""),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    _SetType("", CmdType::Dummy);
}

FormCmdParam::FormCmdParam(QString name, QWidget *parent) :
    QWidget(parent),
    mName(name),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    _SetType("", CmdType::Dummy);
}

FormCmdParam::FormCmdParam(QString name, int* pValue, ParamSetting config, QWidget *parent) :
    QWidget(parent),
    mName(name),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    // keep the value into the threshold
    // update input else ui does not match the setting
    if(*pValue < (int)config.Minimum)
    {
        *pValue = (int)config.Minimum;
    }
    else if(*pValue > (int)config.Maximum)
    {
        *pValue = (int)config.Maximum;
    }

    mIntValue = pValue;

    _SetType(name, CmdType::Int);

    ui->nudInt->setMinimum((int)config.Minimum);
    ui->nudInt->setMaximum((int)config.Maximum);
    ui->nudInt->setSingleStep((int)config.SingleStep);

    ui->nudInt->setFocusPolicy(Qt::StrongFocus);
    ui->nudInt->installEventFilter( this );
}

FormCmdParam::FormCmdParam(QString name, int* pValue, std::map<int, QString>map, QWidget *parent):
    QWidget(parent),
    mName(name),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    mIntValue = pValue;

    _SetType(name, CmdType::Enum);

    // populate the combo box
    for(std::map<int, QString>::iterator it = map.begin(); it != map.end(); ++it)
    {
        ui->cboValue->addItem(QString("%1").arg(it->second), (int)it->first);
    }

    ui->cboValue->setFocusPolicy(Qt::StrongFocus);
    ui->cboValue->installEventFilter(this);
}

FormCmdParam::FormCmdParam(QString name, float* pValue, ParamSetting config, QWidget *parent) :
    QWidget(parent),
    mName(name),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    mFloatValue = pValue;

    _SetType(name, CmdType::Float);

    ui->nudFloat->setMinimum(config.Minimum);
    ui->nudFloat->setMaximum(config.Maximum);
    ui->nudFloat->setSingleStep(config.SingleStep);

    ui->nudFloat->setFocusPolicy(Qt::StrongFocus);
    ui->nudFloat->installEventFilter( this );
}

FormCmdParam::FormCmdParam(QString name, bool* pValue, QWidget *parent) :
    QWidget(parent),
    mName(name),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    mBoolValue = pValue;

    ui->chkValue->setChecked(*mBoolValue);

    _SetType(name, CmdType::Bool);
}

FormCmdParam::FormCmdParam(QString name, std::string* pValue, QWidget *parent) :
    QWidget(parent),
    mName(name),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    mStrValue = pValue;

    ui->txtValue->setText(CONVERT_TO_QSTRING((*mStrValue)));

    _SetType(name, CmdType::Text);
}

FormCmdParam::~FormCmdParam()
{
    delete ui;
}

void FormCmdParam::_SetColor()
{
#ifdef FORM_COLOR
    QPalette pal;
    pal.setColor(QPalette::Background, FORM_COLOR);
    this->setAutoFillBackground(true);
    this->setPalette(pal);
#endif
}

void FormCmdParam::_SetType(QString name, CmdType eType)
{
    int lblWidth;
    int txtWidth;

    ui->lblName->setText(name);
    ui->chkValue->setText("");

    ui->chkValue->hide();
    ui->nudInt->hide();
    ui->nudFloat->hide();
    ui->txtValue->hide();
    ui->cboValue->hide();

    mType = eType;

    switch(eType)
    {
    case CmdType::Bool:
        ui->chkValue->show();
        break;

    case CmdType::Int:
        ui->nudInt->show();
        break;

    case CmdType::Float:
        ui->nudFloat->show();
        break;

    case CmdType::Text:
        lblWidth = 60;
        txtWidth = 200;

        ui->lblName->setFixedWidth(lblWidth);
        ui->txtValue->setFixedWidth(txtWidth);

        ui->txtValue->show();
        break;

    case CmdType::Enum:
        ui->cboValue->show();
        break;

    case CmdType::Dummy:
        break;
    }

    bUpdating = false;
}

void FormCmdParam::SetHideCondition(bool* bCondition, bool bValue)
{
    // create a condition
    Condition condition(bCondition, bValue);

    mHideConditionList.append(condition);
}

void FormCmdParam::SetEnableCondition(bool* bCondition, bool bValue)
{
    Condition condition(bCondition, bValue);

    mEnableConditionList.append(condition);
}

FormCmdParam::UpdateStatus FormCmdParam::PleaseUpdate()
{
    int index;
    bUpdating = true;

    switch(mType)
    {
    case CmdType::Bool:
        ui->chkValue->setChecked(*mBoolValue);
        break;

    case CmdType::Int:
        if(*mIntValue < (int)ui->nudInt->minimum())
        {
            *mIntValue = (int)ui->nudInt->minimum();
        }
        else if(*mIntValue > (int)ui->nudInt->maximum())
        {
            *mIntValue = (int)ui->nudInt->maximum();
        }

        ui->nudInt->setValue(*mIntValue);
        break;

    case CmdType::Float:
        ui->nudFloat->setValue(*mFloatValue);
        break;

    case CmdType::Text:
        ui->txtValue->setText(CONVERT_TO_QSTRING((*mStrValue)));
        break;

    case CmdType::Enum:
        index = ui->cboValue->findData(*mIntValue);
        if( index != -1 )
        {
            ui->cboValue->setCurrentIndex(index);
        }
        break;

    default:
        break;
    }

    bUpdating = false;

    UpdateStatus status = UpdateStatus::Normal;

    // parse all the condition and hide if necesary
    for(int condIndex = 0; condIndex < mHideConditionList.count(); condIndex ++)
    {
        if(mHideConditionList[condIndex].IsHidden() == true)
        {
            status = UpdateStatus::Hidden;
        }
    }

    if(status == UpdateStatus::Normal)
    {
        // indicate if the param must be enable or not depending on the configuration
        for(int condIndex = 0; condIndex < mEnableConditionList.count(); condIndex ++)
        {
            if(mEnableConditionList[condIndex].IsHidden() == true)
            {
                status = UpdateStatus::Disable;
            }
        }
    }

    return status;
}

void FormCmdParam::SetEnable(bool bEnable)
{
    ui->lblName->setEnabled(bEnable);
    ui->chkValue->setEnabled(bEnable);

    ui->chkValue->setEnabled(bEnable);
    ui->nudInt->setEnabled(bEnable);
    ui->nudFloat->setEnabled(bEnable);
    ui->txtValue->setEnabled(bEnable);
    ui->cboValue->setEnabled(bEnable);
}

void FormCmdParam::on_nudInt_valueChanged(int)
{
    on_ValueChange();
}

void FormCmdParam::on_nudFloat_valueChanged(double)
{
    on_ValueChange();
}

void FormCmdParam::on_chkValue_clicked()
{
    on_ValueChange();
}

void FormCmdParam::on_txtValue_textChanged(const QString &)
{
    on_ValueChange();
}

void FormCmdParam::on_cboValue_currentIndexChanged(int index)
{
    on_ValueChange(index);
}

void FormCmdParam::on_ValueChange(int index)
{
    if(bUpdating == false)
    {
        switch(mType)
        {
        case CmdType::Bool:
            *mBoolValue = ui->chkValue->isChecked();
            emit on_Change();
            break;

        case CmdType::Int:
            *mIntValue = ui->nudInt->value();
            break;

        case CmdType::Float:
            *mFloatValue = (float)ui->nudFloat->value();
            break;

        case CmdType::Text:
            *mStrValue = CONVERT_TO_STRING(ui->txtValue->text());
            break;

        case CmdType::Enum:
            // retrieve the item data matching the selected index
            *mIntValue = (ui->cboValue->itemData(index)).toInt();
            break;

        default:
            break;
        }
    }
}
