
#define __CISGWMOTORC__
#include "CISGWMotor.h"

CISGWMotor::CISGWMotor(QObject *parent,CEXI2Message *myI2CMessage) : QObject(parent)
//------------------------------------------------------------------------------------
{
    mI2CMessage = myI2CMessage ;
}

float CISGWMotor::VPower()
//---------------------
{
  float ADCPower ;
  float ADCReference ;
  float ADCZero ;
  float fResult ;

  ADCPower = mI2CMessage->GetSCommand(I2C_ADDRESS,EXI2C_ADC_POWER) ;
  ADCReference = mI2CMessage->GetSCommand(I2C_ADDRESS,EXI2C_ADC_REFERENCE) ;

  ADCZero = mI2CMessage->GetSCommand(I2C_ADDRESS,EXI2C_ADC_ZERO) ;

  fResult = VREF * VPOWER_RATIO * ADCPower / ADCReference ;

  return (fResult) ;
}

float CISGWMotor::VLogic()
//---------------------
{
  float ADCReference ;
  float fResult ;



  ADCReference = mI2CMessage->GetSCommand(I2C_ADDRESS,EXI2C_ADC_REFERENCE) ;

  fResult = VREF*(1024.0f/ADCReference) ;

  return (fResult) ;

}

bool CISGWMotor::GetPresent()
//---------------------------
{
  return ( mI2CMessage->GetPresent(I2C_ADDRESS) ) ;
}

void CISGWMotor::SetDefaultParameters()
//-------------------------------------
{
  mI2CMessage->SetDefaultParameters(I2C_ADDRESS);
}

unsigned char CISGWMotor::GetDirection()
//------------------------------
{
  return (mI2CMessage->GetBParameter(I2C_ADDRESS, EXI2C_DIRECTION)) ;
}

void CISGWMotor::SetDirection(unsigned char ucDirection)
//----------------------------------------------
{
   mI2CMessage->SetBParameter(I2C_ADDRESS, EXI2C_DIRECTION, ucDirection) ;

}

float CISGWMotor::GetDS1624Temperature(unsigned char I2CAddress)
//-------------------------------------------------------
{
  return ( mI2CMessage->GetDS1624Temperature(I2CAddress) ) ;
}

unsigned char CISGWMotor::MyI2CAddress()
//------------------------------
{
  return (I2C_ADDRESS) ;
}

quint16 CISGWMotor::GetCount()
//----------------------------
{
  return (mI2CMessage->GetSParameter(I2C_ADDRESS, EXI2C_COUNT)) ;
}

void    CISGWMotor::SetCount (quint16 usCount)
//-------------------------------------------
{
  mI2CMessage->SetSParameter(I2C_ADDRESS, EXI2C_COUNT,usCount) ;
}

quint16 CISGWMotor::GetStartupStepPeriod()
//----------------------------------------
{
  return (mI2CMessage->GetSParameter(I2C_ADDRESS, EXI2C_STARTUP_STEP_PERIOD)) ;

}

void    CISGWMotor::SetStartupStepPeriod(quint16 usPeriod)
//---------------------------------------------------------
{
  mI2CMessage->SetSParameter(I2C_ADDRESS, EXI2C_STARTUP_STEP_PERIOD,usPeriod) ;
}

quint16 CISGWMotor::GetMinimumStepPeriod()
//----------------------------------------
{
  return (mI2CMessage->GetSParameter(I2C_ADDRESS, EXI2C_MINIMUM_STEP_PERIOD)) ;

}

void    CISGWMotor::SetMinimumStepPeriod(quint16 usPeriod)
//---------------------------------------------------------
{
  mI2CMessage->SetSParameter(I2C_ADDRESS, EXI2C_MINIMUM_STEP_PERIOD,usPeriod) ;
}

quint16 CISGWMotor::GetRampUpIncrement()
//--------------------------------------
{
  return(mI2CMessage->GetSParameter(I2C_ADDRESS, EXI2C_RAMP_UP_INCREMENT)) ;
}

void    CISGWMotor::SetRampUpIncrement(quint16 usPeriod)
//------------------------------------------------------
{
  mI2CMessage->SetSParameter(I2C_ADDRESS, EXI2C_RAMP_UP_INCREMENT,usPeriod) ;
}

unsigned char   CISGWMotor::GetPosition ()
//--------------------------------
{
  return (mI2CMessage->GetBParameter(I2C_ADDRESS, EXI2C_POSITION)) ;
}

void    CISGWMotor::SetPosition (unsigned char ucPosition)
//------------------------------------------------
{
  mI2CMessage->SetBParameter(I2C_ADDRESS, EXI2C_POSITION,ucPosition) ;
}

bool    CISGWMotor::GetEnabled()
//------------------------------
{
 unsigned char ucResult ;

  ucResult = mI2CMessage->GetBParameter(I2C_ADDRESS, EXI2C_ENABLED) ;
  if (ucResult == 1)  return (true) ;
  else                return (false) ;

}

void    CISGWMotor::SetEnabled(bool ubValue)
//------------------------------------------
{
  if (ubValue) {
     mI2CMessage->SetBParameter(I2C_ADDRESS, EXI2C_ENABLED,1) ;
  }
  else {
     mI2CMessage->SetBParameter(I2C_ADDRESS, EXI2C_ENABLED,0) ;
   }

}

void    CISGWMotor::Move()
//------------------------
{
  mI2CMessage->SendCommand(I2C_ADDRESS, EXI2C_MOVE) ;
}

void    CISGWMotor::Search()
//--------------------------
{
  mI2CMessage->SendCommand (I2C_ADDRESS, EXI2C_SEARCH) ;
}

quint32 CISGWMotor::GetLTimer()
//-----------------------------
{
  return (mI2CMessage->GetLParameter(I2C_ADDRESS, EXI2C_LTIMER)) ;

}

quint16 CISGWMotor::GetCaptureWindowsSize()
//-----------------------------------------
{
  return (0) ;
  //CaptureWindowSize = mI2CMessage->SParameter(I2C_ADDRESS, isfwmotorCaptureWindowSize)

}

quint16 CISGWMotor::GetOverrideCount()
//------------------------------------
{
//OverrideCount = mI2CMessage->SParameter(I2C_ADDRESS, isfwmotorOverrideCount)
  return (0) ;
}

//void    CISGWMotor::SetOverrideCount (quint16 usCount)
void    CISGWMotor::SetOverrideCount (quint16)
//----------------------------------------------------
{
  //mI2CMessage->SParameter(I2C_ADDRESS, isfwmotorOverrideCount) = lngCount

}

#define STATUS_MASK 0x70

unsigned char CISGWMotor::GetStatus()
//---------------------------
{
    unsigned char status = 0;
    status = mI2CMessage->GetStatus(I2C_ADDRESS);
    status = status  & STATUS_MASK;
    return status;
}

void CISGWMotor::SetStatus(unsigned char mData)
//-------------------------------------
{
  mI2CMessage->SetStatus(I2C_ADDRESS,mData) ;
}

unsigned char   CISGWMotor::GetMotorNumber()
//----------------------------------
{
  return (mI2CMessage->GetBParameter(I2C_ADDRESS, EXI2C_MOTOR_NUMBER)) ;
}

void    CISGWMotor::SetMotorNumber (unsigned char ucPosition)
//--------------------------------------------------
{
  mI2CMessage->SetBParameter(I2C_ADDRESS, EXI2C_MOTOR_NUMBER,ucPosition) ;
}

unsigned char  CISGWMotor::GetEventsSinkAddress()
//---------------------------------------
{
  return (mI2CMessage->GetEventsSinkAddress(I2C_ADDRESS)) ;
}

void  CISGWMotor::SetEventsSinkAddress(unsigned char mData)
//--------------------------------------------------
{
  mI2CMessage->SetEventsSinkAddress(I2C_ADDRESS,mData) ;
}
