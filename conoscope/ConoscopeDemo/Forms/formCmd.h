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
        Open,
        Setup,
        SetupStatus,
        Measure,
        ExportRaw,
        ExportProcessed,
        Close,
        Reset,
        SetConfig,
        AE,
        AEMeasArea,
        ROI,

        CfgFileWrite,
        CfgFileRead,
        CfgFileStatus,

        DisplayStream,
        DisplayRaw,
        DisplayProcessed,

        CaptureSequence,
        CaptureSequenceCancel,
        CaptureSequenceStatus,

        MeasureAE,
        MeasureAECancel,
        MeasureAEStatus,

        ConvertRaw,
    };
    Q_ENUM(CmdType)

public:
    explicit FormCmd(CmdType eCmdType, CmdParameters_t* params, QWidget *parent = 0);

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

    // QList<FormCmdParam*> mFormParams;
    QList<FormCmdParam*> mForms;

    void _SetColor(int index = 0);

    void _ParamAppend(FormCmdParam *paramForm);

    void _ParamHideCondition(QString paramName, bool* bCondition, bool bValue = true);

    void _ParamEnableCondition(QString paramName, bool* bCondition, bool bValue = true);

    FormCmdParam* _FindParamByName(QString paramName);
};

#endif // FORMCMD_H
