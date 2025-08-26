#ifndef HWTOOL_H
#define HWTOOL_H

#define SWAP(Value) ((((Value) & 0xFF) << 8) | (((Value) >> 8) & 0xFF))
#define SET(Target, Bit) ((Target) | (Bit))
#define CLEAR(Target, Bit) ((Target) & (~(Bit)))
#define INVERT(Target, Bit) ((Target) ^ (Bit))
#define ISBITSET(Target, Bit) ((Target) & (Bit))
#define ISBITCLEAR(Target, Bit) (!((Target) & (Bit)))

#define MAX(First, Second) (((First) >= (Second)) ? (First) : (Second))
#define MIN(First, Second) (((First) <= (Second)) ? (First) : (Second))

#define HIGH_BYTE(Value)  (UCHAR)(((Value) >> 8) & 0xFF)
#define LOW_BYTE(Value)   (UCHAR)((Value) & 0xFF)

#define BIT0      0x01
#define BIT1      0x02
#define BIT2      0x04
#define BIT3      0x08
#define BIT4      0x10
#define BIT5      0x20
#define BIT6      0x40
#define BIT7      0x80

#define CStr(Value)             QString::number (Value)
#define FormatFloat(Value)     (float)((int)(Value*1000.0f))/1000.0f

#endif // HWTOOL_H
