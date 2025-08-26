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



