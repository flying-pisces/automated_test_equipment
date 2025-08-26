#ifndef CMOTOR_H
#define CMOTOR_H

#include <QObject>
#include "CEXI2Message.h"

class CISGWMotor : public QObject
{
    Q_OBJECT
public:

  #define  MOTORIdle                    0
  #define  MOTORSuccess                 0x10
  #define  MOTOROperating               0x20
  #define  MOTORError                   0x40
  #define  MOTORStatusBits              0x70
  #define  MOTORRetriesBits             15

  #define  MOTORCW                      0
  #define  MOTORCCW                     1

  explicit CISGWMotor(QObject *parent = 0,CEXI2Message *myI2CMessage = 0);

  float   VPower() ;
  float   VLogic() ;


  float   GetDS1624Temperature(unsigned char I2CAddress) ;

  unsigned char   MyI2CAddress ();
  bool    GetPresent () ;
  void    SetDefaultParameters () ;
  unsigned char   GetDirection() ;
  void    SetDirection (unsigned char ucDirection) ;
  quint16 GetCount() ;
  void    SetCount (quint16 usCount) ;
  quint16 GetStartupStepPeriod() ;
  void    SetStartupStepPeriod(quint16 usPeriod) ;
  quint16 GetMinimumStepPeriod() ;
  void    SetMinimumStepPeriod(quint16 usPeriod) ;
  quint16 GetRampUpIncrement() ;
  void    SetRampUpIncrement(quint16 usPeriod) ;
  unsigned char   GetPosition () ;
  void    SetPosition (unsigned char ucPosition) ;
  bool    GetEnabled() ;
  void    SetEnabled(bool ubValue) ;
  void    Move() ;
  void    Search() ;
  quint32 GetLTimer() ;
  quint16 GetCaptureWindowsSize() ;
  quint16 GetOverrideCount() ;
  void    SetOverrideCount (quint16 usCount) ;
  unsigned char   GetStatus();
  void    SetStatus(unsigned char mData);

  unsigned char   GetMotorNumber();
  void    SetMotorNumber(unsigned char mData);

  unsigned char   GetEventsSinkAddress();
  void    SetEventsSinkAddress(unsigned char mData);

signals:

public slots:

private:

  #ifdef __CISGWMOTORC__
    #define  I2C_ADDRESS    (0x3E/2)


    #define  VREF           2.5f
    #define  VPOWER_RATIO   11.0f

    #define EXI2C_SET_DEFAULTS  		0x20	// No returned value
    #define EXI2C_MOVE			0x21	// No returned value
    #define EXI2C_SEARCH			0x22	// No returned value

    //---- Others Parameters
    #define EXI2C_LTIMER      		0x20  // LONG
    #define EXI2C_DIRECTION   		0x21  // UBYTE
    #define EXI2C_COUNT 			0x22  // SHORT
    #define EXI2C_MOTOR_NUMBER			0x23  // UBYTE
    #define EXI2C_POSITION		0x24  // UBYTE
    #define EXI2C_ENABLED			0x25  // UBYTE
    #define EXI2C_STARTUP_STEP_PERIOD	0x26  // SHORT
    #define EXI2C_MINIMUM_STEP_PERIOD	0x27  // SHORT
    #define EXI2C_RAMP_UP_INCREMENT	0x28  // SHORT




    //---- Reserved Commands
    #define EXI2C_ADC_REFERENCE           0x00
    #define EXI2C_ADC_ZERO		0x01
    #define EXI2C_ADC_POWER		0x02

    //---- Reserved Events
    #define EXI2C_DEFAULT			0x00
  #endif
  CEXI2Message *mI2CMessage ;

};

#endif // CMOTOR_H
