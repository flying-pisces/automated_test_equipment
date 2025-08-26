#ifndef CONOSCOPEAPPHELPER_H
#define CONOSCOPEAPPHELPER_H

// configuration
#define SET_TEMPERATURE

#include "classcommon.h"
#include "camera.h"

#include "Conoscope.h"

#include "conoscopeTypes.h"

class ConoscopeAppHelper : public ClassCommon
{
    Q_OBJECT

public:
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

        int minExposureTimeUs; // minimum exposure time
        int maxExposureTimeUs; // maximum exposure time
    } AutoExposureParam_t;

protected:
    ConoscopeAppHelper(QObject *parent = nullptr);
    ~ConoscopeAppHelper();

    int _GetExposureTime(int exposureTimeUs, int granularity, int min = 0, int max = 0);

    ClassCommon::Error _CaptureAutoExposure(MeasureConfigWithCropFactor_t& config, std::vector<int16_t>& mBuffer, float autoExposurePixelMax);

    bool _ProcessAutoExposure(AutoExposureParam_t& param, int intMaximum, int& IntegrationTime);

    static bool mCancelRequest;

    void LogInFile(QString message);
    void LogInApp(QString message);

    QString mLogHeader;
    QString mLogAppHeader;

    ClassCommon::Error _WaitForSetup();
};

#endif // CONOSCOPEAPPHELPER_H
