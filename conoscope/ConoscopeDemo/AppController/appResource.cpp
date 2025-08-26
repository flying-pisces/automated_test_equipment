#include "appResource.h"

AppResource* AppResource::mInstance;

AppResource* AppResource::GetInstance()
{
    if(mInstance == nullptr)
    {
        mInstance = new AppResource();
    }

    return mInstance;
}

AppResource* AppResource::Instance()
{
    return GetInstance();
}

bool AppResource::IsMaskEnable(LogMask_t mask)
{
    return mAppConfig.mLogMasks.IsPresent(mask);
}

void AppResource::GetDisplayStreamConfig(ProcessingConfig_t &config)
{
    mConfig->GetDisplayStreamConfig(config);
}

void AppResource::Close()
{
    delete(mConfig);
}
