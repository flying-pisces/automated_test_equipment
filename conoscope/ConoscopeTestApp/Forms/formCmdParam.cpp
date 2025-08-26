#include "formCmdParam.h"
#include "ui_formCmdParam.h"

// #define FORM_COLOR QColor(255, 100, 84)
#define FORM_COLOR QColor(240, 240, 240)

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())
#define CONVERT_TO_STRING(a) a.toUtf8().constData();

FormCmdParam::FormCmdParam(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    _SetType("", CmdType::Dummy);
}

FormCmdParam::FormCmdParam(QString name, int* pValue, ParamSetting config, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    mIntValue = pValue;

    _SetType(name, CmdType::Int);

    ui->nudInt->setMinimum((int)config.Minimum);
    ui->nudInt->setMaximum((int)config.Maximum);
    ui->nudInt->setSingleStep((int)config.SingleStep);
}

FormCmdParam::FormCmdParam(QString name, int* pValue, ParamSetting, std::map<int, QString>map, QWidget *parent) :
    QWidget(parent),
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
}

FormCmdParam::FormCmdParam(QString name, float* pValue, ParamSetting config, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormCmdParam)
{
    ui->setupUi(this);

    _SetColor();

    mFloatValue = pValue;

    _SetType(name, CmdType::Float);

    ui->nudFloat->setMinimum(config.Minimum);
    ui->nudFloat->setMaximum(config.Maximum);
    ui->nudFloat->setSingleStep(config.SingleStep);
}

FormCmdParam::FormCmdParam(QString name, bool* pValue, QWidget *parent) :
    QWidget(parent),
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
        lblWidth = ui->lblName->width();
        txtWidth = ui->txtValue->width();

        lblWidth -= 140;
        txtWidth += 130;

        ui->lblName->setFixedWidth(lblWidth);
        ui->txtValue->setFixedWidth(txtWidth);

        ui->txtValue->show();
        break;

    case CmdType::Enum:
        ui->cboValue->show();
        break;
    }

    bUpdating = false;
}

void FormCmdParam::PleaseUpdate()
{
    int index;
    bUpdating = true;

    switch(mType)
    {
    case CmdType::Bool:
        ui->chkValue->setChecked(*mBoolValue);
        break;

    case CmdType::Int:
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

    }

    bUpdating = false;
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

void FormCmdParam::on_txtValue_textChanged(const QString&)
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
        }
    }
}
