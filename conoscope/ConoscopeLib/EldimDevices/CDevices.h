#ifndef CDEVICES_H
#define CDEVICES_H

#include <QObject>
#include "camera/Camera.h"
#include "EldimDevices/CEXI2Message.h"
#include "EldimDevices/CISGWMotor.h"
#include "EldimDevices/CISGWSensor.h"

class CDevices : public QObject
{
  Q_OBJECT

public:

    typedef enum {
        Status_Done,
        Status_Processing,
        Status_Error,
        Status_Unknown
    } Status_t;

public:
  explicit CDevices(QObject *parent = 0,Camera *myCamera = 0);

  bool BiWheelIsPresent () ;


  void  BiWheelSelect(unsigned char ucMotorNumber);
  void  BiWheelUnselect();
  unsigned char BiWheelPosition(unsigned char ucMotorNumber);
  Status_t  BiWheelGoto(unsigned char ucMotorNumber, unsigned char ucIndex, unsigned int &waitDelay);
  unsigned char CurrentBiWheelStatus();
  bool  BiWheelWaitForReady(int intNbOfretries, unsigned int waitDelayMs);

#ifdef CHECK_WHEEL_INTEGRITY
    unsigned char GetMotorStatus();
#endif

signals:

public slots:

private:
  CISGWMotor      *mMotor ;
  CISGWSensor     *mSensor ;
  CEXI2Message    *mI2CMessage ;
  int             mintTargetBWPosition ;
  int             mintCurrentBWWheel ;
  bool BiWheelMotorBusy();
};

#endif // CDEVICES_H
