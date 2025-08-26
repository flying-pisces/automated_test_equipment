#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QMetaEnum>
#include <QTimer>

#include "classcommon.h"
#include "appControllerWorker.h"

#include "appResource.h"

#include "FrameBuffer.h"

#define INIT_STATE_MACHINE

class AppController : public ClassThreadCommon
{
    Q_OBJECT

public:
    enum class Event {
        CmdReset,
        CmdResetDone,

        Test1,
        Test2,
        TestSetup,
        ProcessRawData,
        TestMeasureAE,
        TestCapture,
        TestCaptureSequence,

        TestDone,

        Error,
    };
    Q_ENUM(Event)

    enum class State {
        Undefined,  /*< worker is not started */
        Idle,       /*< worker is started */
        Error,

        Resetting,

        Test1,
        Test2,
        TestSetup,
        ProcessRawData,
        TestMeasureAE,
        TestCapture,
        TestCaptureSequence,
    };
    Q_ENUM(State)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = AppController::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    /*!
     *  \brief  constructor
     *  \param  parent
     *  \return none
     */
    explicit AppController(QObject *parent = nullptr);

    /*!
     *  \brief  destructor
     *  \param  none
     *  \return none
     */
    ~AppController();

    /*!
     *  \brief  start the thread execution
     *  \param  none
     *  \return none
     */
    void Start();

    /*!
     *  \brief  stop the thread execution
     *  \param  none
     *  \return error
     */
    ClassCommon::Error Stop();

    void StopThread();

public:
    QString GetVersion();

    void CmdSetConfig(ConoscopeSettings_t& config);
    void CmdGetConfig(ConoscopeSettings_t& config);

    void CmdGetConfig(SetupConfig_t& setupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig);
    void CmdGetConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig, ConoscopeDebugSettings_t& conoscopeSettings);

    void CmdReset();

    void Test1();
    void Test2();
    void TestSetup();
    void ProcessRawData();
    void TestMeasureAE();
    void TestCapture();
    void TestCaptureSequence();

    void TestAbort();

private:
    bool SendRequest(AppControllerWorker::Request event);

    ClassCommon::Error ChangeState(State eState);

    ClassCommon::Error ProcessStateMachine(Event eEvent);

    State m_state;

    AppControllerWorker* m_worker;

public slots:
    void on_worker_jobDone(int value, int error);

    void on_log(QString message);

    void on_status(QString message);

signals:
    void RawBufferReady();

protected:
    void run();

};

#endif // APPCONTROLLER_H
