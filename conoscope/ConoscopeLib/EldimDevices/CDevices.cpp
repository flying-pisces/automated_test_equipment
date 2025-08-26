#include "CDevices.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>

CDevices::CDevices(QObject *parent, Camera *myCamera): QObject(parent)
//--------------------------------------------------------------------
{
  mI2CMessage = new CEXI2Message (this,myCamera) ;
  mMotor = new CISGWMotor (this,mI2CMessage);
  mSensor = new CISGWSensor (this,mI2CMessage);
}

bool CDevices::BiWheelIsPresent()
//---------------------------------
{
  if (mMotor->GetPresent())
  {
    // qDebug ("Motor Present");
    return (true) ;
  }
  else {
    // qDebug ("Motor Not Present");
    return (false) ;
   }

  // return (true) ;
}


void CDevices::BiWheelSelect(unsigned char ucMotorNumber)
//-------------------------------------------------
{

    BiWheelUnselect () ;

    if (ucMotorNumber == 0) {
      mSensor->SelectSensorBoard(2); mSensor->SetEnable(true);
      mSensor->SelectSensorBoard(1); mSensor->SetEventsSinkAddress(mMotor->MyI2CAddress());
      mMotor->SetMotorNumber(ucMotorNumber);
    }
    if (ucMotorNumber == 1) {
      mSensor->SelectSensorBoard(3); mSensor->SetEnable(true);
      mSensor->SelectSensorBoard(2); mSensor->SetEventsSinkAddress(mMotor->MyI2CAddress());
      mMotor->SetMotorNumber(ucMotorNumber);
    }
}

void CDevices::BiWheelUnselect()
//------------------------------
{
    // Disable each Sensor Board : power down led
    mSensor->SelectSensorBoard(1); mSensor->SetEnable(false); mSensor->SetEventsSinkAddress(0);
    mSensor->SelectSensorBoard(2); mSensor->SetEnable(false); mSensor->SetEventsSinkAddress(0);
    mSensor->SelectSensorBoard(3); mSensor->SetEnable(false); mSensor->SetEventsSinkAddress(0);
}

unsigned char CDevices::BiWheelPosition(unsigned char ucMotorNumber)
//------------------------------------------------------------
{
  unsigned char ucCurrentPosition ;

  BiWheelSelect (ucMotorNumber) ;
  ucCurrentPosition = mSensor->GetPosition() ;
  BiWheelUnselect () ;
  return (ucCurrentPosition) ;
}

#define BiWheelNbOfPositions  8
#define PositiveDirection     1
#define NegativeDirection     0

#define WAIT_DELAY_UNIT_MS    750

CDevices::Status_t CDevices::BiWheelGoto(unsigned char ucMotorNumber, unsigned char ucIndex, unsigned int& waitDelayMs)
//------------------------------------------------------------
{
    int intCurrentPosition ;
    int intDistance ;

    if(BiWheelIsPresent() == false)
    {
        return(CDevices::Status_t::Status_Error);
    }

    unsigned char status = mMotor->GetStatus();

    if((status == MOTOROperating) ||
       (status == MOTORRetriesBits) ||
       (status == MOTORStatusBits))
    {
        qDebug() << QString("    BiWheelGoto  ERROR 1");
        return(CDevices::Status_t::Status_Error);
    }

    if((ucIndex < 1) || (ucIndex > BiWheelNbOfPositions))
    {
        qDebug() << QString("    BiWheelGoto  ERROR 2");
        return(CDevices::Status_t::Status_Error);
    }

    mintTargetBWPosition = ucIndex;
    mintCurrentBWWheel = ucMotorNumber;

    intCurrentPosition =  BiWheelPosition(mintCurrentBWWheel) ;

    if (mintTargetBWPosition == intCurrentPosition)
    {
        qDebug() << QString("    BiWheelGoto  Done");
        return(CDevices::Status_t::Status_Done);
    }

    BiWheelSelect (ucMotorNumber);
    mMotor->SetEnabled(true);
    mMotor->SetEventsSinkAddress(0x00);

    // select direction
    intDistance = mintTargetBWPosition - intCurrentPosition ;

    // calculate the delay required to move the wheel
    waitDelayMs = WAIT_DELAY_UNIT_MS * ((intDistance > 0) ? intDistance : -intDistance);
    // qDebug() << QString(" >> BiWheelGoTo distance %1 -> %2").arg(intDistance).arg(waitDelayMs);

    if (intDistance > 0)
    {
        if (intDistance <= (BiWheelNbOfPositions / 2))
        {
            mMotor->SetDirection (PositiveDirection) ;
        }
        else
        {
            mMotor->SetDirection (NegativeDirection) ;
        }
    }
    else
    {
        if (intDistance >= -(BiWheelNbOfPositions / 2))
        {
            mMotor->SetDirection (NegativeDirection) ;
        }
        else
        {
            mMotor->SetDirection (PositiveDirection) ;
        }
    }

    mMotor->SetStartupStepPeriod (10100) ;
    mMotor->SetMinimumStepPeriod (1100) ;
    mMotor->SetRampUpIncrement (1000) ;

    mMotor->SetCount(4000) ;
    mMotor->SetStatus(MOTORIdle) ;
    mMotor->SetPosition(ucIndex) ;

    mMotor->Search();

    // qDebug() << QString("  BiWheelGoto  Processing");
    return(CDevices::Status_t::Status_Processing);
}

unsigned char  CDevices::CurrentBiWheelStatus ()
//--------------------------------------
{
  if (BiWheelIsPresent ())
    return (mMotor->GetStatus()) ;
  else
    return (0xFF) ;
}

bool  CDevices::BiWheelMotorBusy ()
//-----------------------------------
{
    unsigned char ucStatus ;

    ucStatus = mMotor->GetStatus() ;

    return (((ucStatus == MOTOROperating) || (ucStatus == MOTORRetriesBits) ||(ucStatus ==MOTORStatusBits))) ;
}

#define LOCAL_TIMEOUT_MS  10000
#define LOCAL_WAIT_MS     250

bool  CDevices::BiWheelWaitForReady(int intNbOfretries = 5, unsigned int waitDelayMs = -1)
{
    CDevices::Status_t wheelGotoStatus;
    QElapsedTimer timer;

    bool bDone = false;
    bool bRet = true;

    do
    {
        // wait for the wheel to move in position
        // there is not polling possible in this version
        if(waitDelayMs > 0)
        {
            QThread::msleep(waitDelayMs);
        }

        timer.restart();

        int CheckTimer = 0;

        while((BiWheelMotorBusy()) && (timer.elapsed() < LOCAL_TIMEOUT_MS) )
        {
            qDebug() << QString("BiWheelWaitForReady BUSY loop");

            CheckTimer = timer.elapsed();

            QThread::msleep(LOCAL_WAIT_MS);
        }

        if (timer.elapsed() > LOCAL_TIMEOUT_MS)
        {
            qDebug() << QString("BiWheelWaitForReady  error 1");

            // something wrong!
            BiWheelUnselect();
        }

        // the motor is stopped : now control its position
        if (mintTargetBWPosition == BiWheelPosition(mintCurrentBWWheel))
        {
            // qDebug() << QString("BiWheelWaitForReady  position ok");
            bDone = true;

            // success case
            bRet = true;
        }
        else
        {
            intNbOfretries --;

            if(intNbOfretries == 0)
            {
                // process done
                bDone = true;

                // error case
                bRet = false;
            }
            else
            {
                qDebug() << QString("  BiWheelGoto  retry");

                // reprogram the move
                wheelGotoStatus = BiWheelGoto (mintCurrentBWWheel, mintTargetBWPosition, waitDelayMs);

                switch(wheelGotoStatus)
                {
                case Status_Done:
                    bDone = true;
                    bRet  = true;
                    break;
                case Status_Processing:
                    bDone = false;
                    break;
                case Status_Error:
                default:
                    bDone = true;
                    // error case
                    bRet = false;
                    break;
                }
            }
        }
    } while(bDone == false);

    qDebug() << QString("BiWheelWaitForReady  %1").arg(bRet);

    return bRet;
}

#ifdef CHECK_WHEEL_INTEGRITY
unsigned char CDevices::GetMotorStatus()
{
    unsigned char status = mMotor->GetStatus();

    return status;
}
#endif
