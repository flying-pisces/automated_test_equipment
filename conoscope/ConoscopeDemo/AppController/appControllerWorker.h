#ifndef APPCONTROLLERWORKER_H
#define APPCONTROLLERWORKER_H

#include "classcommon.h"
#include "conoscopeTypes.h"
#include "FrameBuffer.h"

#include <vector>

class AppControllerWorker : public ClassCommon
{
    Q_OBJECT

public:
    enum class Request
    {
        CmdOpen,
        CmdSetup,
        CmdMeasure,

        CmdMeasureAE,
        CmdMeasureAECancel,

        CmdExportRaw,
        CmdExportProcessed,

        CmdClose,
        CmdReset,

        CmdSetConfig,

        CmdCfgFileWrite,
        CmdCfgFileRead,

        CmdStream,

        CmdExportRawBuffer,
        CmdExportProcessedBuffer,

        CmdCaptureSequence,
        CmdCaptureSequenceCancel,

        EventCaptureSequenceDone,
        EventCaptureSequenceCancel,
        EventCaptureSequenceError,

        EventMeasureAEDone,
        EventMeasureAECancel,
        EventMeasureAEError,

        CmdConvertRaw,
    };
    Q_ENUM(Request)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = AppControllerWorker::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

    typedef struct
    {
        SetupConfig_t         cmdSetupConfig;
        MeasureConfig_t       cmdMeasureConfig;
        ProcessingConfig_t    cmdProcessingConfig;
        ConoscopeSettings_t   cmdSetConfig;
        CaptureSequenceConfig_t cmdCaptureSequence;
        ConvertRaw_t          cmdConvertRaw;

        struct
        {
            bool    bStreamProcessedData;
            bool    autoExposure;
            float   autoExposurePixelMax;
        } applicationConfig;
    } Parameters_t;

    Parameters_t params;

public:
    /*!
     *  \brief  constructor
     *  \param  parent
     *  \return none
     */
    explicit AppControllerWorker(QObject *parent = nullptr);

    /*!
     *  \brief  destructor
     *  \param  none
     *  \return none
     */
    ~AppControllerWorker();

    void CancelCmd();

    std::vector<int16_t> mBuffer;

    FrameBuffer* pFrameBuffer;

private:
    ClassCommon::Error _CmdOpen();
    ClassCommon::Error _CmdSetup();
    ClassCommon::Error _CmdMeasure();

    ClassCommon::Error _CmdMeasureAE();
    ClassCommon::Error _CmdMeasureAECancel();

    ClassCommon::Error _CmdExportRaw();
    ClassCommon::Error _CmdExportProcessed();
    ClassCommon::Error _CmdClose();
    ClassCommon::Error _CmdReset();

    ClassCommon::Error _CmdCfgFileWrite();
    ClassCommon::Error _CmdCfgFileRead();

    ClassCommon::Error _CmdStream();
    ClassCommon::Error _CmdExportRawBuffer();
    ClassCommon::Error _CmdExportProcessedBuffer();

#define ROI_ON_DISPLAY
#ifdef ROI_ON_DISPLAY
    void _CropBuffer(int RoiXRight, int RoiXLeft, int RoiYBottom, int RoiYTop, std::vector<int16_t> &buffer, int &height, int &width);
#endif

    ClassCommon::Error _CmdCaptureSequence();
    ClassCommon::Error _CmdCaptureSequenceCancel();

    ClassCommon::Error _CmdConvertRaw();

    bool bCancelCommand;

    typedef struct
    {
        int targetMax;

        int thresholdUp;
        int thresholdDown;

        int saturation;

        float decreasingFactor;
        float increasingFactor;
        float noiseLevelRatio;

        float sngNoise;

    } AutoExposureParam_t;

    bool _ProcessAutoExposure(AutoExposureParam_t& param, int intMaximum, int &IntegrationTime);

    bool SendWarning(QString message);

public slots:
    void OnWorkRequest(int value);

signals:
    void WorkDone(int value, int error);

    void RawBufferReady(int width, int height, int whiteLevel);
    void RawBufferAeRoi(int AEMeasAreaWidth,
                        int AEMeasAreaHeight,
                        int AEMeasAreaX,
                        int AEMeasAreaY);

    void PleaseUpdateExposureTime(int exposureTimeUs);

    void WarningMessage(QString message);
};

#endif // APPCONTROLLERWORKER_H
