#ifndef FORMCMDPARAM_H
#define FORMCMDPARAM_H

#include <QWidget>

#include <QEvent>
#include <QAbstractSpinBox>
#include <QComboBox>

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

    enum class UpdateStatus{
        Normal,
        Hidden,
        Disable
    };
    Q_ENUM(UpdateStatus)

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

    class Condition {

    public:
        Condition(bool* condition,
                  bool  conditionValue)
        {
            pCondition = condition; // condition (boolean) indicating if the control must be enable or not
            mConditionValue = conditionValue;
        }

        bool IsHidden()
        {
            bool bCondition = *pCondition;

            if (mConditionValue != bCondition)
            {
                return true;
            }
            return false;
        }

    private:
        bool*  pCondition; // condition (boolean) indicating if the control must be enable or not
        bool   mConditionValue;
    };

public:
    explicit FormCmdParam(QWidget *parent = 0);

    explicit FormCmdParam(QString name, QWidget *parent = 0);

    explicit FormCmdParam(QString name, int* pValue, ParamSetting config, QWidget *parent = 0);

    explicit FormCmdParam(QString name, int* pValue, std::map<int, QString>map, QWidget *parent = 0);

    explicit FormCmdParam(QString name, float* pValue, ParamSetting config, QWidget *parent = 0);

    explicit FormCmdParam(QString name, bool* pValue, QWidget *parent = 0);

    explicit FormCmdParam(QString name, std::string* pValue, QWidget *parent = 0);

    ~FormCmdParam();

    void SetHideCondition(bool* bCondition, bool bValue = true);

    void SetEnableCondition(bool* bCondition, bool bValue = true);

    UpdateStatus PleaseUpdate();

    void SetEnable(bool bEnable);

    QString mName;

public slots:
    void on_ValueChange(int index = 0);

signals:
    void on_Change();

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

    bool         bUpdating;

    // condition indicating if the control must be enable or not
    QVector<Condition> mHideConditionList;

    // condition indicating if the control must be enable or not
    QVector<Condition> mEnableConditionList;

    bool eventFilter(QObject * object, QEvent * event)
    {
        if(event->type() == QEvent::Wheel)
        {
            event->ignore();
            return true;
        }

        return QWidget::eventFilter(object, event);
    }
};

#endif // FORMCMDPARAM_H
