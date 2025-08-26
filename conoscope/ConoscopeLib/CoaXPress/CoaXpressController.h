#ifndef COAXPRESSCONTROLLER_H
#define COAXPRESSCONTROLLER_H

#include <QObject>
#include <qstring.h>

#include <EGenTL.h>
#include <EGrabber.h>

#include "CoaXpressTypes.h"

#define USE_CUSTOM_GRABBER
#ifdef USE_CUSTOM_GRABBER
#include "CoaXpressGrabber.h"
#endif

#include "toolTypes.h"

class CoaXpressController : public QObject
{
    Q_OBJECT
public:
    explicit CoaXpressController(QObject *parent = nullptr);

    ~CoaXpressController();

    void Initialise();

    void Grab();

    void Acquire(int acquisitionNumber = 1);

    void Configure();

    void GetTemperature(CameraSettings &settings);

signals:
    void FrameCaptured(int frameIndex);

public slots:

private:
    Euresys::EGenTL*  mGentl;
    CoaXpressGrabber* mGrabber;
};

#endif // COAXPRESSCONTROLLER_H
