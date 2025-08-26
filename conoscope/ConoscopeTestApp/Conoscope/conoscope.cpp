#include "conoscope.h"
#include "conoscopeApi.h"

#include "cameraCmvCxp.h"

Conoscope::Conoscope(QObject *parent) : ConoscopeApi(parent)
{
    CameraCmvCxp* cameraCmvCxp;

    cameraCmvCxp = new CameraCmvCxp(this);
    cameraCmvCxp->SetScriptStatement(":/config_Adimec_S50_FS.js");

    mCamera = cameraCmvCxp;

    // connect(mCamera, &Camera::EventOccured,
    //        this, &AppController::on_camera_eventOccured);
}

Conoscope::~Conoscope()
{

}

void Conoscope::GetInfo()
{

}

ClassCommon::Error Conoscope::CmdOpen()
{
    ClassCommon::Error eError;

    // connect the camera
    QString cameraSerialNumber = "SN_0";
    QString cameraBoardSerialNumber = "CriticalLink_0";

    QString ipAddr = "TBD_ipAddr";
    QString port = "TBD_port";
    QString connectionConfig = "CXP6_X2";

    eError = mCamera->Register(cameraSerialNumber, cameraBoardSerialNumber);

    eError = mCamera->ConfigureConnection(ipAddr, port, connectionConfig);

    // connect the camera
    eError = mCamera->Connect();

    return eError;
}

void Conoscope::CmdSetup()
{

}

void Conoscope::CmdSetupStatus()
{

}

void Conoscope::CmdMeasure()
{

}

void Conoscope::CmdMeasureStatus()
{

}

void Conoscope::CmdExportRaw()
{

}

void Conoscope::CmdExportProcessed()
{

}

void Conoscope::CmdClose()
{

}

void Conoscope::CmdReset()
{

}

