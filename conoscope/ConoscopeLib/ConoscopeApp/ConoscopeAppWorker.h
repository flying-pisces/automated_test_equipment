#ifndef CONOSCOPEAPPWORKER_H
#define CONOSCOPEAPPWORKER_H

#include "classcommon.h"
#include <string>

#include "conoscopeLib.h"
#include "ConoscopeStaticTypes.h"

#ifdef APP_HELPER
#include "ConoscopeAppHelper.h"
#endif

typedef struct
{
    bool      bGenerateXYZ; // indicate whether YXZ file must be generated
} CaptureSequenceOption_t;

typedef struct
{
    std::vector<int16_t>* data;

    double convFactX;
    double convFactY;
    double convFactZ;
} CaptureSequenceBuffer_t;

class ConoscopeAppWorker : public ConoscopeAppHelper
{
    Q_OBJECT

public:
    enum class Request
    {
        CmdCaptureSequence,
        CmdMeasureAE
    };
    Q_ENUM(Request)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = ConoscopeAppWorker::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    /*!
     *  \brief  constructor
     *  \param  parent
     *  \return none
     */
    explicit ConoscopeAppWorker(QObject *parent = nullptr);

    /*!
     *  \brief  destructor
     *  \param  none
     *  \return none
     */
    ~ConoscopeAppWorker();

    static MeasureConfig_t mMeasureConfig;
    static MeasureStatus_t mMeasureStatus;

    static CaptureSequenceConfig_t    mCaptureSequenceConfig;
    static CaptureSequenceStatus_t    mCaptureSequenceStatus;
    static ProcessingConfig_t         mCaptureSequenceExportConfig;

    ClassCommon::Error CaptureSequenceCancel();

    ClassCommon::Error MeasureAECancel();

public:
    static ConoscopeDebugSettings_t mDebugSettings;
    static ConoscopeSettings_t      mSettings;

private:
    void _CapturingSequenceFileName(CaptureSequenceConfig_t& config, SomeInfo_t& info, QString &FileName, QString &appendPart);

    ClassCommon::Error _CmdCapturingSequence();

    ClassCommon::Error _CmdCapturingSequenceEmulate();

#ifndef MULTITHREAD_CAPTURE_SEQUENCE
    ClassCommon::Error _Capturing(Filter_t eFilter, CaptureSequenceConfig_t &config, CaptureSequenceBuffer_t &buffer);

    ClassCommon::Error _Capture(MeasureConfigWithCropFactor_t config);
#endif /* MULTITHREAD_CAPTURE_SEQUENCE */

    ClassCommon::Error _ReadCapture(QString fileName, CaptureSequenceBuffer_t &buffer, CaptureSequenceConfig_t &config, QString &capturePath);

    ClassCommon::Error _ReadConversionFactor(QString filePath, CaptureSequenceBuffer_t &buffer, CaptureSequenceConfig_t &config);

    void _ReadExposureTimeFile(QMap<Filter_t, int>& exposureTimeList);

    void _ReadCapturesFile(QMap<Filter_t, QString>& fileList);

    void _ReadExposureExportOption(ProcessingConfig_t& processingConfig, CaptureSequenceOption_t &option);

    void _SaveExposureExportOption(QString fileName, ProcessingConfig_t& processingConfig, CaptureSequenceOption_t &option);

    QString DisplayCmdExportProcessedError();

    typedef enum
    {
        ComposeMode_Normal,
        ComposeMode_Append
    } ComposeMode_t;

    ClassCommon::Error _Compose(std::vector<int16_t> &mBuffer, std::vector<float_t> &output, double conversionFactor, ComposeMode_t mode = ComposeMode_Normal, QString fileName = "");
    ClassCommon::Error _ComposeComponents(QString fileName, QMap<Filter_t, CaptureSequenceBuffer_t> &bufferList, QString appendFileName = "");

    std::vector<int16_t> mBuffer1;
    std::vector<int16_t> mBuffer2;
    std::vector<int16_t> mBuffer3;
    std::vector<int16_t> mBuffer4;
    std::vector<int16_t> mBuffer5;

    std::vector<float_t>  mCompose;

    ClassCommon::Error _CmdMeasureAE();

    void _WriteCaptureSequenceInfo(QString fileName, QMap<Filter_t, int> &exposureTimeList, SomeInfo_t& info);

public slots:
    void OnWorkRequest(int value);

signals:
    void WorkDone(int value, int error);
};

#endif // CONOSCOPEAPPWORKER_H
