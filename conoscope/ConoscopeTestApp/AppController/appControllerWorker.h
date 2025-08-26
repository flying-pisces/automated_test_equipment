#ifndef APPCONTROLLERWORKER_H
#define APPCONTROLLERWORKER_H

#include "classcommon.h"
#include "conoscopeTypes.h"
#include "FrameBuffer.h"

class AppControllerWorker : public ClassCommon
{
    Q_OBJECT

public:
    enum class Request
    {
        CmdReset,
        Test1,
        Test2,
        TestSetup,
        ProcessRawData,
        TestMeasureAE,
        TestCapture,
        TestCaptureSequence,
    };
    Q_ENUM(Request)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = AppControllerWorker::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

    typedef struct
    {
        SetupConfig_t       cmdSetupConfig;
        MeasureConfig_t     cmdMeasureConfig;
        ProcessingConfig_t  cmdProcessingConfig;
        ConoscopeSettings_t cmdSetConfig;
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

    QByteArray mRawDataBuffer;

    FrameBuffer* pFrameBuffer;

private:
    ClassCommon::Error _CmdReset();

    ClassCommon::Error _Test1();
    ClassCommon::Error _Test1Sub(int testIndex);

    ClassCommon::Error _Test2();
    ClassCommon::Error _Test2Sub(int testIndex);

    ClassCommon::Error _TestSetup();
    ClassCommon::Error _TestSetupSub(int testIndex);

    ClassCommon::Error _TestMeasureAE();
    ClassCommon::Error _TestMeasureAESub(int testIndex);
    ClassCommon::Error _MeasureAE(MeasureConfig_t &measureConfig);

    ClassCommon::Error _TestCapture();
    ClassCommon::Error _TestCaptureSub(int testIndex);

    ClassCommon::Error _TestCaptureSequence();
    ClassCommon::Error _TestCaptureSequenceSub(int testIndex, CaptureSequenceConfig_t config);

    ClassCommon::Error _ProcessRawData();
    ClassCommon::Error _ProcessRawData(QString directoryPath);

    ClassCommon::Error _GetSetup();
    void _UpdateSetupTemperature(SetupConfig_t& setupConfig);

    bool bCancelCommand;

public slots:
    void OnWorkRequest(int value);

signals:
    void WorkDone(int value, int error);

    void RawBufferReady();
};

#endif // APPCONTROLLERWORKER_H
