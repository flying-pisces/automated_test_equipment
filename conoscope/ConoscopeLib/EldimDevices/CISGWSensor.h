#ifndef CISGWSENSOR_H
#define CISGWSENSOR_H

#include <QObject>
#include "CEXI2Message.h"

class CISGWSensor : public QObject
{
    Q_OBJECT
public:



  explicit CISGWSensor(QObject *parent = 0,CEXI2Message *myI2CMessage = 0);

  float   GetADCChannel(unsigned char ucChannel);
  float   VPower();
  float   VLogic();
  bool    GetPresent();
  unsigned char   MyI2CAddress ();
  void    SetDefaultParameters();
  quint16 GetLEDA();
  quint16 GetLEDB();
  quint16 GetLED0();
  quint16 GetLED1();
  quint16 GetLED2();
  quint16 GetLED3();

  bool GetEnable();
  void  SetEnable(bool blnEnable) ;
  unsigned char GetPosition();
  unsigned char GetEventsSinkAddress();
  void SetEventsSinkAddress(unsigned char mData);

  void SelectSensorBoard(unsigned char ucSensorBoardNumber);

signals:

public slots:

private:

  unsigned char   mI2CAddress ;
  #ifdef __CISGWSENSORC__
    #define  I2C_ADDRESS1    (0x3C/2)
    #define  I2C_ADDRESS2    (0x3A/2)
    #define  I2C_ADDRESS3    (0x38/2)

    #define  VREF           2.5f
    #define  VPOWER_RATIO   11.0f

      //---- Reserved Commands
    #define EXI2C_ADC_REFERENCE   0x00
    #define EXI2C_ADC_ZERO        0x01
    #define EXI2C_ADC_POWER       0x02

      //---- Reserved Events
    #define EXI2C_DEFAULT         0x00

    #define EXI2C_ADC_LEDA        0x3
    #define EXI2C_ADC_LEDB        0x4
    #define EXI2C_ADC_LED0        0x5
    #define EXI2C_ADC_LED1        0x6
    #define EXI2C_ADC_LED2        0x7
    #define EXI2C_ADC_LED3        0x8

    #define EXI2C_MODE            0x21  // UBYTE (One bit used)
    #define EXI2C_POSITION	0x22  // UBYTE
  #endif
  CEXI2Message *mI2CMessage ;

};

#endif // CISGWSENSOR_H
