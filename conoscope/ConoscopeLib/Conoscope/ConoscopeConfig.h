#ifndef CONOSCOPECONFIG_H
#define CONOSCOPECONFIG_H

#include "classcommon.h"
#include "conoscopeTypes.h"

class ConoscopeConfig : public ClassCommon
{
    Q_OBJECT

public:
    ConoscopeConfig(QObject *parent = nullptr);

    ~ConoscopeConfig();

    void SetConfig(SetupConfig_t& config);
    void GetConfig(SetupConfig_t& config);

    void SetConfig(MeasureConfig_t& config);
    void GetConfig(MeasureConfig_t& config);

    void SetConfig(MeasureConfigWithCropFactor_t &config);

    void SetConfig(ProcessingConfig_t& config);
    void GetConfig(ProcessingConfig_t& config);

    void SetConfig(ConoscopeDebugSettings_t& config);
    void GetConfig(ConoscopeDebugSettings_t& config);

    void SetConfig(ConoscopeSettings_t& config);
    void GetConfig(ConoscopeSettings_t& config);

    void GetConfig(ConoscopeSettingsI_t& config);

    void SetConfig(CaptureSequenceConfig_t& config);
    void GetConfig(CaptureSequenceConfig_t& config);

    bool bSaveConfig; // save settings in json file

private:
    QString mFileName;

    SetupConfig_t             mCmdSetupConfig;
    MeasureConfig_t           mCmdMeasureConfig;
    ProcessingConfig_t        mCmdProcessingConfig;

    ConoscopeDebugSettings_t  mConoscopeDebugSettings;
    ConoscopeSettings_t       mConoscopeSettings;
    ConoscopeSettingsI_t      mConoscopeSettingsI;

    CaptureSequenceConfig_t   mCaptureSequenceConfig;

    void _Default();
    bool _Load();
    void _Save();

    template<typename T>
    void _CheckThreshold(T& value, T min, T max)
    {
        if(value < min)
        {
            value = min;
        }
        else if(value > max)
        {
            value = max;
        }
    }
};

#endif // CONOSCOPECONFIG_H
