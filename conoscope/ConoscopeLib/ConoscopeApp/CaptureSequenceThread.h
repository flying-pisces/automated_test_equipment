#ifndef CAPTURESEQUENCETHREAD_H
#define CAPTURESEQUENCETHREAD_H

#include <QThread>

#include <QMutex>
#include <QWaitCondition>

#include <QSemaphore>

#include "conoscopeTypes.h"

#include <QMap>
#include "ConoscopeAppWorker.h"

#include "ConoscopeAppHelper.h"

typedef struct
{
    bool bSaturatedCapture;
} CaptureSequenceResult_t;

class CaptureSequenceThread : public QThread, public ConoscopeAppHelper
{
    Q_OBJECT

public:
    CaptureSequenceThread(QObject *parent = nullptr);
    ~CaptureSequenceThread();

    static void Initialise(QList<Filter_t>  *filterList,
                           QMap<Filter_t, int> *exposureTimeList,
                           QMap<Filter_t, CaptureSequenceBuffer_t> *bufferList)
    {
        mFilterList = filterList;
        mExposureTimeList = exposureTimeList;
        mBufferList = bufferList;

        mResult.bSaturatedCapture = false;

        // initialise semaphores
        mSemaSetup.acquire(mSemaSetup.available());
        mSemaSetup.release();

        mSemaProcessing.acquire(mSemaProcessing.available());
        mSemaProcessing.release();

        ConoscopeAppHelper::mCancelRequest = false;
    }

    static void Cancel()
    {
        ConoscopeAppHelper::mCancelRequest = true;
    }

    void GetError(ClassCommon::Error& eError, int& index)
    {
        eError = meError;
        index = instanceIndex;
    }

    static CaptureSequenceResult_t GetResult()
    {
        return mResult;
    }

protected:
    void run() override;

private:
    static int instanceCounter;

    static QList<Filter_t> *mFilterList;
    static QMap<Filter_t, int> *mExposureTimeList;
    static QMap<Filter_t, CaptureSequenceBuffer_t> *mBufferList;
    static CaptureSequenceResult_t mResult;

    int instanceIndex;

    ClassCommon::Error _SetupStatus(bool &canRetry);
    ClassCommon::Error _ProcessSetup(Filter_t eFilter, CaptureSequenceConfig_t& config);
    ClassCommon::Error _ProcessMeasure(Filter_t eFilter, CaptureSequenceConfig_t& config, CaptureSequenceBuffer_t& buffer);
    ClassCommon::Error _ProcessExport(Filter_t eFilter, CaptureSequenceConfig_t& config, CaptureSequenceBuffer_t &buffer);
    ClassCommon::Error _Capture(MeasureConfigWithCropFactor_t config);

    ClassCommon::Error meError;

    static bool mErrorOccurs;

    void CheckStaticError();

public:
    static QSemaphore mSemaSetup;
    static QSemaphore mSemaProcessing;
};

#endif // CAPTURESEQUENCETHREAD_H
