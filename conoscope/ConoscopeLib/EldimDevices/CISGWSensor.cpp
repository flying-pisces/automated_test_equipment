#define __CISGWSENSORC__

#include "CISGWSensor.h"



CISGWSensor::CISGWSensor(QObject *parent,CEXI2Message *myI2CMessage) : QObject(parent)
//------------------------------------------------------------------------------------
{
    mI2CMessage = myI2CMessage ;
    mI2CAddress = I2C_ADDRESS1 ; // by default !

}


float CISGWSensor::VPower()
//---------------------
{
  float ADCPower ;
  float ADCReference ;
  float ADCZero ;
  float fResult ;

  ADCPower = mI2CMessage->GetSCommand(mI2CAddress,EXI2C_ADC_POWER) ;
  ADCReference = mI2CMessage->GetSCommand(mI2CAddress,EXI2C_ADC_REFERENCE) ;

  ADCZero = mI2CMessage->GetSCommand(mI2CAddress,EXI2C_ADC_ZERO) ;

  fResult = VREF * VPOWER_RATIO * ADCPower / ADCReference ;
  return (fResult) ;
}

float CISGWSensor::VLogic()
//---------------------
{
  float ADCReference ;
  float fResult ;



  ADCReference = mI2CMessage->GetSCommand(mI2CAddress,EXI2C_ADC_REFERENCE) ;

  fResult = VREF*(1024.0f/ADCReference) ;

  return (fResult) ;
}

float CISGWSensor::GetADCChannel(unsigned char ucChannel) {
//--------------------------------------------------

  return (mI2CMessage-> GetSCommand(mI2CAddress, ucChannel)) ;

}

bool CISGWSensor::GetPresent()
//----------------------------
{
  return ( mI2CMessage->GetPresent(mI2CAddress) ) ;
}

unsigned char CISGWSensor::MyI2CAddress()
//-------------------------------
{
  return (mI2CAddress) ;
}

void CISGWSensor::SetDefaultParameters()
//-------------------------------------
{
  mI2CMessage->SetDefaultParameters(mI2CAddress);
}


quint16 CISGWSensor::GetLEDA() {
//-------------------------------
  return (GetADCChannel(EXI2C_ADC_LEDA)) ;
}

quint16 CISGWSensor::GetLEDB() {
//-------------------------------
  return (GetADCChannel(EXI2C_ADC_LEDB)) ;
}

quint16 CISGWSensor::GetLED0() {
//-------------------------------
  return (GetADCChannel(EXI2C_ADC_LED0)) ;
}

quint16 CISGWSensor::GetLED1() {
//-------------------------------
  return (GetADCChannel(EXI2C_ADC_LED1)) ;
}

quint16 CISGWSensor::GetLED2() {
//-------------------------------
  return (GetADCChannel(EXI2C_ADC_LED2)) ;
}

quint16 CISGWSensor::GetLED3() {
//-------------------------------
  return (GetADCChannel(EXI2C_ADC_LED3)) ;
}


bool CISGWSensor::GetEnable() {
//----------------------------
  if (mI2CMessage->GetBParameter(mI2CAddress, EXI2C_MODE))
    return (true) ;
  else
    return (false) ;
}

void  CISGWSensor::SetEnable(bool blnEnable) {
//---------------------------------------------
  unsigned char ucMode ;

  if (blnEnable)  ucMode = 1 ;
  else            ucMode = 0 ;

  mI2CMessage->SetBParameter(mI2CAddress, EXI2C_MODE, ucMode) ;
}

unsigned char CISGWSensor::GetPosition() {
//-------------------------------------
  return (mI2CMessage->GetBParameter(mI2CAddress, EXI2C_POSITION)) ;
}


unsigned char CISGWSensor::GetEventsSinkAddress()
//---------------------------------------
{
  return (mI2CMessage->GetEventsSinkAddress(mI2CAddress)) ;
}

void CISGWSensor::SetEventsSinkAddress(unsigned char mData)
//--------------------------------------------------
{
  mI2CMessage->SetEventsSinkAddress(mI2CAddress,mData) ;
}

// 1,2 or 3
void CISGWSensor::SelectSensorBoard(unsigned char ucSensorBoardNumber)
//-------------------------------------------------------------------
{
  if (ucSensorBoardNumber == 1) mI2CAddress = I2C_ADDRESS1 ;
  if (ucSensorBoardNumber == 2) mI2CAddress = I2C_ADDRESS2 ;
  if (ucSensorBoardNumber == 3) mI2CAddress = I2C_ADDRESS3 ;
}

