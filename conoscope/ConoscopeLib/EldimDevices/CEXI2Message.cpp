#include "CEXI2Message.h"

CEXI2Message::CEXI2Message(QObject *parent,Camera *myCamera) : QObject(parent)
//-----------------------------------------------------------------------------
{
  mCamera = myCamera ;
  EXI2CM_WHO_IS_TALKING = 0x1C/2 ; //  ISSCAN ! :)
}


bool CEXI2Message::GetPresent(unsigned char I2CAddress)
//--------------------------------------------
{
  return (mCamera->HW_I2CIsDeviceReady(I2C_BUS_DMD_FPGA,I2CAddress)) ;
}

unsigned char CEXI2Message::GetBParameter(unsigned char I2CAddress, unsigned char Index)
//-------------------------------------------------------------
{

  EXI2CM_OPCODE_OUT = EXI2CM_FETCH + EXI2CM_BYTE ;
  EXI2CM_INDEX_OUT = Index ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 3, DataRX, 2) ; // ST7 cannot send only one byte

  return (DataRX[0]) ;
}

void CEXI2Message::SetBParameter(unsigned char I2CAddress, unsigned char Index, unsigned char mData)
//---------------------------------------------------------------------------
{
  EXI2CM_OPCODE_OUT = EXI2CM_STORE + EXI2CM_SHORT ;
  EXI2CM_INDEX_OUT = Index ;

  DataTX [3] = mData ;
  DataTX [4] = mData ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 5, DataRX, 0) ;
}


qint16 CEXI2Message::GetSParameter(unsigned char I2CAddress, unsigned char Index)
//----------------------------------------------------------------
{
  quint16 Result ;

  EXI2CM_OPCODE_OUT = EXI2CM_FETCH + EXI2CM_SHORT ;
  EXI2CM_INDEX_OUT = Index ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 3, DataRX, 2) ;

  CopyAndSwap ((unsigned char *)&Result,DataRX,2) ;

  return (Result) ;
}

void CEXI2Message::SetSParameter(unsigned char I2CAddress, unsigned char Index, qint16 mData)
//------------------------------------------------------------------------------
{
  EXI2CM_OPCODE_OUT = EXI2CM_STORE + EXI2CM_SHORT ;
  EXI2CM_INDEX_OUT = Index ;

  CopyAndSwap (EXI2CM_PTR_DATA_OUT,(unsigned char *)&mData,2) ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 5, DataRX, 0) ;
}


qint32 CEXI2Message::GetLParameter(unsigned char I2CAddress, unsigned char Index)
//----------------------------------------------------------------
{
  quint32 Result ;

  EXI2CM_OPCODE_OUT = EXI2CM_FETCH + EXI2CM_LONG ;
  EXI2CM_INDEX_OUT = Index ;


  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 3, DataRX, 4) ;

  CopyAndSwap ((unsigned char *)&Result,DataRX,4) ;

  return (Result) ;
}

void CEXI2Message::SetLParameter(unsigned char I2CAddress, unsigned char Index, qint32 mData)
//------------------------------------------------------------------------------
{
  EXI2CM_OPCODE_OUT = EXI2CM_STORE + EXI2CM_LONG ;
  EXI2CM_INDEX_OUT = Index ;

  CopyAndSwap (EXI2CM_PTR_DATA_OUT,(unsigned char *)&mData,4) ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 7, DataRX, 0) ;
}

void CEXI2Message::SendCommand(unsigned char I2CAddress, unsigned char Index)
//-----------------------------------------------------------
{
  EXI2CM_OPCODE_OUT = EXI2CM_COMMAND ;
  EXI2CM_INDEX_OUT = Index ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 3, DataRX, 0) ;

}

qint16 CEXI2Message::GetSCommand(unsigned char I2CAddress, unsigned char Index)
//-------------------------------------------------------------
{
  quint16 Result ;

  EXI2CM_OPCODE_OUT = EXI2CM_COMMAND + EXI2CM_SHORT ;
  EXI2CM_INDEX_OUT = Index ;

  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(DataTX, 3, DataRX, 2) ;

  CopyAndSwap ((unsigned char *)&Result,DataRX,2) ;

  return (Result) ;
}

float CEXI2Message::GetDS1624Temperature(unsigned char I2CAddress)
//--------------------------------------------------------
{
  #define DS1624_READ_TEMPERATURE			0xAA

  unsigned char ubData[2];
  unsigned char ubResult [2];
  float fResult ;

  ubData [0] =  DS1624_READ_TEMPERATURE ;


  mCamera->HW_I2CSelectDevice(I2C_BUS_DMD_FPGA,I2CAddress) ;
  mCamera->HW_I2CSendAndReceiveData(ubData,1,ubResult, 2) ;

  fResult = (ubResult [1] + ubResult [0]*256.0f)*(125.0f/32000.0f) ;

  return (fResult) ;
}

unsigned char CEXI2Message::GetEventsSinkAddress(unsigned char I2CAddress)
//--------------------------------------------------------
{
  // Attention : mult by 2 : not the same adress system in the EZContast System
  return (GetBParameter(I2CAddress, EXI2C_EVENTS_SINK)/2) ;
}

void CEXI2Message::SetEventsSinkAddress(unsigned char I2CAddress, unsigned char mData)
//-------------------------------------------------------------------
{
  // Attention : mult by 2 : not the same adress system in the EZContast System
  SetBParameter(I2CAddress, EXI2C_EVENTS_SINK, 2*mData) ;
}

quint16 CEXI2Message::GetArbitrationLostCount(unsigned char I2CAddress)
//-------------------------------------------------------------
{
  return (GetSParameter(I2CAddress, EXI2C_ARLO_COUNT)) ;
}

void CEXI2Message::SetArbitrationLostCount(unsigned char I2CAddress, quint16 mData)
//-------------------------------------------------------------------------
{
  SetSParameter(I2CAddress, EXI2C_ARLO_COUNT, mData) ;
}

quint16 CEXI2Message::GetBusErrorCount(unsigned char I2CAddress)
//------------------------------------------------------
{
  return (GetSParameter(I2CAddress, EXI2C_BERR_COUNT)) ;
}

void CEXI2Message::SetBusErrorCount(unsigned char I2CAddress, quint16 mData)
//------------------------------------------------------------------
{
  SetSParameter(I2CAddress, EXI2C_BERR_COUNT, mData) ;
}

unsigned char CEXI2Message::GetStatus(unsigned char I2CAddress)
//---------------------------------------------
{
  return (GetBParameter(I2CAddress, EXI2C_STATUS)) ;
}

void CEXI2Message::SetStatus(unsigned char I2CAddress, unsigned char mData)
//---------------------------------------------------------
{
  SetBParameter(I2CAddress, EXI2C_STATUS, mData) ;
}

void CEXI2Message::SetDefaultParameters(unsigned char I2CAddress)
//-------------------------------------------------------
{
  SendCommand(I2CAddress,EXI2C_SetDefaultParameters );
}

void CEXI2Message::CopyAndSwap(unsigned char *ubDataDest, unsigned char *ubDataSource, unsigned char ubLength)
//------------------------------------------------------------------------------------
{
  unsigned char ubIndex ;

  for (ubIndex = 0 ; ubIndex < ubLength ; ubIndex ++) {
    ubDataDest [ubIndex] = ubDataSource [(ubLength-1)-ubIndex] ;
  }
}

