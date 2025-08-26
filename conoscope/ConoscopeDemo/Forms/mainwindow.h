#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QWidget>

#include "appController.h"

#include "formTypes.h"
#include "formCmd.h"

#include "DialogStream.h"
#include "FrameBuffer.h"

namespace Ui {
class MainWindow;
}

typedef enum
{
    StatusColor_Ok,
    StatusColor_Processing,
    StatusColor_Error
} StatusColor_t;

typedef enum
{
    WindowAlignment_LeftFromWindow,
    WindowAlignment_RightFromWindow,
    WindowAlignment_FromWindow
} WindowAlignment_t;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnCmdOpen();
    void on_btnCmdSetup();
    void on_btnCmdSetupStatus();
    void on_btnCmdMeasure();
    void on_btnCmdExportRaw();
    void on_btnCmdExportProc();
    void on_btnCmdClose();
    void on_btnCmdReset();

    void on_btnCmdSetConfig();

    void on_btnCmdCfgFileWrite();
    void on_btnCmdCfgFileRead();
    void on_btnCmdCfgFileStatus();

    void on_btnDisplayStream();
    void on_btnDisplayRaw();
    void on_btnDisplayProcessed();

    void on_btnCaptureSequence();
    void on_btnCaptureSequenceCancel();
    void on_btnCaptureSequenceStatus();

    void on_btnMeasureAE();
    void on_btnMeasureAECancel();
    void on_btnMeasureAEStatus();

    void on_btnConvertRaw();

private:
    Ui::MainWindow  *ui;
    AppController   *mController;

    // all parameters appended to be used to set ui
    // ConoscopeSettings_t      mSettings;
    CmdParameters_t          mCmdParameters;
    // ConoscopeDebugSettings_t mDebugSettings;

    QList<FormCmd*> mFormList;

    FormCmd *formCmdOpen;
    FormCmd *formCmdSetup;
    FormCmd *formCmdSetupStatus;
    FormCmd *formCmdMeasure;
    FormCmd *formCmdExportRaw;
    FormCmd *formCmdExportProc;
    FormCmd *formCmdClose;
    FormCmd *formCmdReset;
    FormCmd *formCmdSetConfig;
    FormCmd *formCmdSetAE;
    FormCmd *formCmdSetAEMeasArea;
    FormCmd *formCmdSetRoi;

    FormCmd *formCmdCfgFileWrite;
    FormCmd *formCmdCfgFileRead;
    FormCmd *formCmdCfgFileStatus;

    FormCmd *formDisplayStream;

    FormCmd *formDisplayRaw;
    FormCmd *formDisplayProcessed;

    FormCmd *formCaptureSequence;
    FormCmd *formCaptureSequenceCancel;
    FormCmd *formCaptureSequenceStatus;

    FormCmd *formCmdMeasureAE;
    FormCmd *formCmdMeasureAECancel;
    FormCmd *formCmdMeasureAEStatus;

    FormCmd *formCmdConvertRaw;

    QSpacerItem *verticalSpacer;

    void PleaseUpdateCmdControl();

    DialogStream * dialogStream;
    void _OpenWindow(QWidget* window, int xOffset, int yOffset, WindowAlignment_t alignment);
    void _CloseWindow(QWidget* window);

    void SetStatusColor(StatusColor_t eColor);

    void UpdateFormsVisibility(AppController::State eState);

    FrameBuffer mFrameBuffer;

    int txtLogCount = 0;

    void DisplayVersion();

#define V_LAYOUT

    // command panel with scrollbar
    QWidget *central;
    QScrollArea *scrollArea;
    QVBoxLayout *verticalLayout;

private slots:
    void onLog(QString message);
    void onStatus(QString message);
    void onStateChange(int state);
    void onWarningMessage(QString message);
    void dialogStreamFinished();
    void onPleaseUpdateExposureTime(int exposureTimeUs);

protected:
    virtual void closeEvent(QCloseEvent* event);
};

#endif // MAINWINDOW_H
