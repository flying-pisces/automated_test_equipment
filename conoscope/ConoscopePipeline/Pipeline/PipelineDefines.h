#ifndef PIPELINEDEFINES_H
#define PIPELINEDEFINES_H

//#define ERROR_OK 0
/*
#define ERROR_FLAT_FIELD_NULL -1
#define ERROR_FLAT_FIELD_DATA_NULL -2
#define ERROR_INPUT_DATA_NULL -3
#define ERROR_INPUT_DATA_NONE -4
#define ERROR_CALIBRATION_NULL -5
#define ERROR_CAPTURE_AREA_NULL -6
#define ERROR_OPTICAL_AXIS_NULL -7
#define ERROR_LINEARIZATION_COEF_NULL -8
#define ERROR_PRECOMPUTED_DATA -9
#define ERROR_SENSOR_TEMPERATURE_DEPENDANCE_NULL - 10
#define ERROR_CONVERSION_FACTOR_NULL -11
#define ERROR_SENSOR_TEMPERATURE_NULL -12
#define ERROR_DATA_SENSOR_TEMPERATURE_NULL -13
*/

typedef enum
{
    Error_Ok = 0,
    Error_FlatFieldNull = -1,
    Error_FlatFieldDataNull = -2,
    Error_InputDataNull = -3,
    Error_InputDataNone = -4,
    Error_CalibrationNull = -5,
    Error_CaptureAreaNull = -6,
    Error_OpticalAxisNull = -7,
    Error_LinearizationCoefNull = -8,
    Error_PrecomputedData = -9,
    Error_SensorTemperatureDependanceNull = -10,
    Error_ConversionFactorNull = -11,
    Error_SensorTemperatureNull = -12,
    Error_DataSensorTemperatureNull = -13,
    Error_OpticalAxisOutOfActiveArea = -14,
} Error_t;

// #define LARGE_MEMORY_CONOSCOPE

#ifdef LARGE_MEMORY_CONOSCOPE
#define FLAT_FIELD_INVERSE_TYPE double
#else
#define FLAT_FIELD_INVERSE_TYPE float
#endif

#define SENSOR_SATURATION 13104

// #define SQUARE(Value) ((Value)*(Value))

#define USE_NEW_IMPLEMENTATION

#endif /* PIPELINEDEFINES_H */
