#include "CoaXpressController.h"
// #include "tools/Logger.h"
#define PRINT
//#include "tools/types.h"
#include <QTime>

CoaXpressController::CoaXpressController(QObject *parent) : QObject(parent)
{
    mGentl = NULL;
    mGrabber = NULL;
}

CoaXpressController::~CoaXpressController()
{
    if(mGrabber != NULL)
    {
        delete(mGrabber);
        mGrabber = NULL;
    }

    if(mGentl != NULL)
    {
        delete(mGentl);
        mGentl = NULL;
    }
}

void CoaXpressController::Initialise()
{
    try
    {
        if(mGentl == NULL)
        {
            mGentl = new Euresys::EGenTL;

            GenTL::TL_HANDLE tl = mGentl->tlOpen();
            uint32_t numCards = mGentl->tlGetNumInterfaces(tl);

            PRINT("Initialise", QString("numCards %1").arg(numCards));

            for (uint32_t n = 0; n < numCards; ++n)
            {
                std::string id = mGentl->tlGetInterfaceID(tl, n);

                QString test = QString::fromUtf8(id.c_str());

                PRINT("Initialise", QString("%1").arg(test));
            }

            PRINT("GenTL", "Created");
        }
    }
    catch(...)
    {
        PRINT("GenTL", "Creation FAILURE");
    }
}

void CoaXpressController::Grab()
{
    if(mGentl == NULL)
    {
        Initialise();
    }

    try
    {
        if(mGrabber == NULL)
        {
            mGrabber = new CoaXpressGrabber(*mGentl, this);

            // connect the signal on the signal
            connect(mGrabber, &CoaXpressGrabber::FrameCaptured,
                    this, &CoaXpressController::FrameCaptured);

            PRINT("Grabber", "Created");
        }
    }
    catch(...)
    {
        PRINT("Grabber", "creation FAILURE");

        delete (mGrabber);
        mGrabber = NULL;
    }
}

void CoaXpressController::Acquire(int acquisitionNumber)
{
    if(mGrabber == NULL)
    {
        Grab();
    }

    if(mGrabber != NULL)
    {
        PRINT("Acquire", "Start");

        mGrabber->Start(acquisitionNumber);

        PRINT("Acquire", "Done");
    }
}

void CoaXpressController::Configure()
{
    if(mGrabber == NULL)
    {
        Grab();
    }

//    mGrabber->Configure();

    delete(mGrabber);
    mGrabber = NULL;
}

void CoaXpressController::GetTemperature(CameraSettings &settings)
{
    if(mGrabber == NULL)
    {
        Grab();
    }

    mGrabber->GetTemperature(settings);
}

