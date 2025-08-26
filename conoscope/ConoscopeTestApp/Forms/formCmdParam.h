#ifndef FORMCMDPARAM_H
#define FORMCMDPARAM_H

#include <QWidget>

namespace Ui {
class FormCmdParam;
}

class FormCmdParam : public QWidget
{
    Q_OBJECT

public:
    enum class CmdType {
        Bool,
        Int,
        Float,
        Text,
        Enum,
        Dummy
    };
    Q_ENUM(CmdType)

    class ParamSetting {
    public:
        ParamSetting(float minimum, float maximum, float singleStep)
        {
            Minimum = minimum;
            Maximum = maximum;
            SingleStep = singleStep;

        }

        float Minimum;
        float Maximum;
        float SingleStep;
    };

public:
    explicit FormCmdParam(QWidget *parent = 0);

    explicit FormCmdParam(QString name, int* pValue, ParamSetting config, QWidget *parent = 0);

    explicit FormCmdParam(QString name, int* pValue, ParamSetting config, std::map<int, QString>map, QWidget *parent = 0);

    explicit FormCmdParam(QString name, float* pValue, ParamSetting config, QWidget *parent = 0);

    explicit FormCmdParam(QString name, bool* pValue, QWidget *parent = 0);

    explicit FormCmdParam(QString name, std::string* pValue, QWidget *parent = 0);

    ~FormCmdParam();

    void PleaseUpdate();

public slots:
    void on_ValueChange(int index = 0);

private slots:
    void on_nudInt_valueChanged(int arg1);

    void on_nudFloat_valueChanged(double arg1);

    void on_chkValue_clicked();

    void on_txtValue_textChanged(const QString &arg1);

    void on_cboValue_currentIndexChanged(int index);

private:
    void _SetColor();

    void _SetType(QString name, CmdType eType);

private:
    Ui::FormCmdParam *ui;

    CmdType mType;

    int*         mIntValue;
    float*       mFloatValue;
    bool*        mBoolValue;
    std::string* mStrValue;

    bool bUpdating;

};

#endif // FORMCMDPARAM_H
