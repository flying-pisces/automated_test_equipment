#ifndef CEXI2MESSAGE_H
#define CEXI2MESSAGE_H

#include <QObject>
#include <QCoreApplication>

#include "camera/Camera.h"

#define  I2C_DATA_SIZE  32
#define  I2C_BUS_DMD_FPGA    0x3


class CEXI2Message : public QObject
{
    Q_OBJECT
public:
  explicit CEXI2Message(QObject *parent = 0,Camera *myCamera = 0);
  bool    GetPresent (unsigned char I2CAddress) ;

  unsigned char GetBParameter (unsigned char I2CAddress, unsigned char Index) ;
  void          SetBParameter (unsigned char I2CAddress, unsigned char Index, unsigned char mData) ;

  qint16  GetSParameter (unsigned char I2CAddress, unsigned char Index) ;
  void    SetSParameter (unsigned char I2CAddress, unsigned char Index, qint16 mData) ;

  qint32  GetLParameter (unsigned char I2CAddress, unsigned char Index) ;
  void    SetLParameter (unsigned char I2CAddress, unsigned char Index, qint32 mData) ;

  void    SendCommand   (unsigned char I2CAddress, unsigned char Index) ;

  qint16  GetSCommand  (unsigned char I2CAddress, unsigned char Index) ;

  float   GetDS1624Temperature (unsigned char I2CAddress) ;

  unsigned char   GetEventsSinkAddress (unsigned char I2CAddress) ;
  void    SetEventsSinkAddress (unsigned char I2CAddress, unsigned char mData) ;

  quint16 GetArbitrationLostCount (unsigned char I2CAddress) ;
  void    SetArbitrationLostCount (unsigned char I2CAddress, quint16 mData) ;

  quint16 GetBusErrorCount (unsigned char I2CAddress) ;
  void    SetBusErrorCount (unsigned char I2CAddress, quint16 mData) ;

  unsigned char   GetStatus (unsigned char I2CAddress) ;
  void            SetStatus (unsigned char I2CAddress, unsigned char mData) ;

  void            SetDefaultParameters (unsigned char I2CAddress) ;









signals:

public slots:


private:


  Camera *mCamera ;
  unsigned char   DataTX [I2C_DATA_SIZE] ;
  unsigned char   DataRX [I2C_DATA_SIZE] ;


  #define EXI2CM_STORE                0x00
  #define EXI2CM_FETCH                0x01
  #define EXI2CM_COMMAND              0x02
  #define EXI2CM_EVENT                0x03
  #define EXI2CM_BYTE                 0x08
  #define EXI2CM_SHORT                0x10
  #define EXI2CM_LONG                 0x20

  #define EXI2C_EVENTS_SINK           0x00
  #define EXI2C_ARLO_COUNT            0x01
  #define EXI2C_BERR_COUNT            0x02
  #define EXI2C_STATUS                0x03

  #define EXI2C_SetDefaultParameters  0x20

  #define EXI2CM_WHO_IS_TALKING	DataTX[0]
  #define EXI2CM_OPCODE_OUT	DataTX[1]
  #define EXI2CM_INDEX_OUT  	DataTX[2]
  #define EXI2CM_PTR_DATA_OUT   (&DataTX[3])

  #define EXI2CM_UBYTE_OUT	EXI2CM_PTR_DATA_OUT[0]

  #define EXI2CM_UBYTE_IN	EXI2CM_PTR_DATA_OUT[0]


  void  CopyAndSwap (unsigned char *ubDataDest,unsigned char *ubDataSource ,unsigned char ubLength) ;


};

#endif // CEXI2MESSAGE_H
