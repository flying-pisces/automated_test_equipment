#ifndef CONOSCOPE_H
#define CONOSCOPE_H


#include <QSharedPointer>
#include <QRect>
#include <QJsonObject>

#include "conoscopeApi.h"

#include "classcommon.h"
#include "toolTypes.h"
#include "imageConfigurationConst.h"

#include "camera.h"

class Conoscope : public ConoscopeApi
{
    Q_OBJECT

public:
    Conoscope(QObject *parent = nullptr);

    ~Conoscope();

public:
    void GetInfo();

    Error CmdOpen();

    void CmdSetup();

    void CmdSetupStatus();

    void CmdMeasure();

    void CmdMeasureStatus();

    void CmdExportRaw();

    void CmdExportProcessed();

    void CmdClose();

    void CmdReset();

private:
    Camera* mCamera;

};

#endif // CONOSCOPE_H
