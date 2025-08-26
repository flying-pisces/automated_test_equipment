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
        Open,
        OpenDone,

        Setup,
        SetupDone,

        Measure,
        MeasureDone,
        MeasureNonFatalError,

        ExportRaw,
        ExportProcessed,
        ExportDone,

        Close,
        CloseDone,

        Reset,
        ResetDone,

        CfgFileRead,
        CfgFileWrite,
        CmdCfgFileDone,

        Stream,
        StreamDone,

        DisplayRaw,
        DisplayProcessed,

        CaptureSequence,
        CaptureSequenceCancel,
        CaptureSequenceDone,

        MeasureAE,
        MeasureAECancel,
        MeasureAEDone,

        ConvertRaw,
        ConvertRawDone,

        Error,
    };
    Q_ENUM(Event)

    enum class State {
        Undefined,  /*< worker is not started */
        Idle,       /*< worker is started */

        Opening,
        Opened,

        SettingUp,
        Ready,

        Measuring,
        MeasureDone,

        MeasuringAE,
        MeasuringAECanceling,

        ExportRaw,
        ExportProcessing,

        CfgFileReading,
        CfgFileWriting,

        Closing,

        Reseting,

        Error,

        Streaming,
        StreamStoping,

        DisplayRaw,
        DisplayProcessed,

        CapturingSequence,
        CaptureSequenceCanceling,

        ConvertingRaw,
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

    void SetRawDataBuffer(FrameBuffer *pFrameBuffer);

    QString GetVersion();

    void CmdOpen();
    void CmdSetup(SetupConfig_t &config);
    void CmdSetupStatus(SetupStatus_t& status);
    void CmdMeasure(MeasureConfig_t& config);
    void CmdExportRaw();
    void CmdExportProcessed(ProcessingConfig_t& config);

    void CmdClose();
    void CmdReset();

    void CmdSetConfig(ConoscopeSettings_t &config);
    void CmdSetConfig(ConoscopeDebugSettings_t &config);

    void CmdGetConfig(ConoscopeSettings_t &config);
    void CmdGetConfig(SetupConfig_t& setupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig);
    ClassCommon::Error CmdGetConfig(SetupConfig_t& cmdSetupConfig, MeasureConfig_t& cmdMeasureConfig, ProcessingConfig_t& cmdProcessingConfig, ConoscopeDebugSettings_t &conoscopeSettings, CaptureSequenceConfig_t &captureSequenceConfig);

    void CmdCfgFileWrite();
    void CmdCfgFileRead();
    void CmdCfgFileStatus();

    void DisplayStream(int exposureTimeUs, bool bStreamProcessedData, bool autoExposure, float autoExposurePixelMax);
    void DisplayStreamStop();

    void DisplayRaw();
    void DisplayProcessed(ProcessingConfig_t& config);

    void CaptureSequence(CaptureSequenceConfig_t& config);
    void CaptureSequenceCancel();
    void CaptureSequenceStatus();

    void MeasureAE(MeasureConfig_t& config);
    void MeasureAECancel();
    void MeasureAEStatus();

    void ConvertRaw(ConvertRaw_t& param);

    void RegisterCallback();

    QString GetErrorMessage();

private:
    bool SendRequest(AppControllerWorker::Request event);

    bool SendWarning(QString message);

    ClassCommon::Error ChangeState(State eState);

    ClassCommon::Error ProcessStateMachine(Event eEvent);

    State m_state;

    AppControllerWorker* m_worker;

    static void _LogCallback(char* message);

    static void _EventCallback(ConoscopeEvent_t event, QString errorDescription);

    static void _WarningCallback(QString message);

public slots:
    void on_worker_jobDone(int value, int error);

    void on_log(QString message);

    void on_status(QString message);

signals:
    void RawBufferReady(int width, int height, int whiteLevel);

    void RawBufferAeRoi(int AEMeasAreaWidth,
                        int AEMeasAreaHeight,
                        int AEMeasAreaX,
                        int AEMeasAreaY);

    void PleaseUpdateExposureTime(int exposureTimeUs);

protected:
    void run();

};

#endif // APPCONTROLLER_H
