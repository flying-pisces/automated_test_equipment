#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpacerItem>

#include "appController.h"

#include "formTypes.h"
#include "formCmd.h"

#include "DialogStream.h"
#include "FrameBuffer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnGetVersion();
    void on_GeneralPurpose_clicked();

    void on_btnCmdSetConfig();
    void on_btnCmdReset();

    void on_btnTest1();
    void on_btnTest2();
    void on_btnTestSetup();
    void on_btnProcessRawData();
    void on_btnTestMeasureAE();
    void on_btnTestCapture();
    void on_btnTestCaptureSequence();

    void on_btnTestAbort();

private:
    Ui::MainWindow  *ui;
    AppController   *mController;

    // all parameters appended to be used to set ui
    // ConoscopeSettings_t      mSettings;
    CmdParameters_t          mCmdParameters;
    ConoscopeDebugSettings_t mDebugSettings;

    QList<FormCmd*> mFormList;

    FormCmd *formCmdSetConfig;
    FormCmd *formCmdReset;
    FormCmd *formTest1;
    FormCmd *formTest2;
    FormCmd *formTestSetup;
    FormCmd *formTestMeasureAE;
    FormCmd *formTestCapture;
    FormCmd *formTestCaptureSequence;
    FormCmd *formProcessRawData;

    FormCmd *formTestAbort;

    QSpacerItem *verticalSpacer;

    void PleaseUpdateCmdControl();

#ifdef REMOVED
    DialogStream * dialogStream;
    void _OpenMenuWindow(QWidget* window, int xOffset, int yOffset, bool left);
    void _CloseMenuWindow(QWidget* window);

    FrameBuffer mFrameBuffer;
#endif

    int txtLogCount = 0;

private slots:
    void onLog(QString message);
    void onStatus(QString message);
    void onStateChange(int state);
#ifdef REMOVED
    void dialogStreamFinished();
#endif
};

#endif // MAINWINDOW_H
