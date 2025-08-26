#ifndef FORMCMD_H
#define FORMCMD_H

#include <QWidget>

#include <QMetaEnum>
#include "formTypes.h"
#include "formCmdParam.h"

namespace Ui {
class FormCmd;
}

class FormCmd : public QWidget
{
    Q_OBJECT

public:
    enum class CmdType {
        SetConfig,
        Reset,
        Test1,
        Test2,
        TestSetup,
        ProcessRawData,

        TestMeasureAE,
        TestCapture,
        TestCaptureSequence,

        TestAbort
    };
    Q_ENUM(CmdType)

public:
    explicit FormCmd(CmdType eCmdType, CmdParameters_t* mParameters, QWidget *parent = 0);
    ~FormCmd();

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = FormCmd::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

    static QString GetEnumString(const QMetaObject &mo, const char* enumName, int enumValue)
    {
        int index = mo.indexOfEnumerator(enumName);
        if(index == -1)
        {
            return "error";
        }
        else
        {
            QMetaEnum metaEnum = mo.enumerator(index);
            return metaEnum.valueToKey(static_cast<int>(enumValue));
        }
    }

    void PleaseUpdate();

signals:
    void ExecuteCmd();

public slots:
    void UpdateUi();

private:
    Ui::FormCmd *ui;

    CmdParameters_t* mParameters;

    QList<FormCmdParam*> mForms;

    void _SetColor(int index = 0);
};

#endif // FORMCMD_H
