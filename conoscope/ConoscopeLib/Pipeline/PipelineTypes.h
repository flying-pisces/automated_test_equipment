#ifndef PIPELINETYPES_H
#define PIPELINETYPES_H

#include <stdlib.h>
#include <vector>

#include "Types.h"

#define RETURN_ITEM_OPTION_ERROR_DESC "ErrorDescription"

#define SQUARE(Value) ((Value)*(Value))

class ImageSize
{
public:
    ImageSize()
    {
        Set(0, 0);
    }

    ImageSize(int imageWidth, int imageHeight, int imageOffsetX = 0, int imageOffsetY = 0)
    {
        Set(imageWidth, imageHeight, imageOffsetX, imageOffsetY);
    }

    ImageSize(const ImageSize& a)
    {
        Set(a.width, a.height, a.offsetX, a.offsetY);
    }

    int32 width;
    int32 height;
    int32 nbPixels;
    int32 offsetX;
    int32 offsetY;

    // this may be better to use an accessor
    // but this is done this way to avoid to make the calculation
    // each time nbPixel is retrieved
    void Set(int imageWidth, int imageHeight, int imageOffsetX = 0, int imageOffsetY = 0)
    {
        width    = imageWidth;
        height   = imageHeight;
        nbPixels = width * height;
        offsetX = imageOffsetX;
        offsetY = imageOffsetY;
    }

    void Set(ImageSize img)
    {
        Set(img.width, img.height, img.offsetX, img.offsetY);
    }
};

class Point
{
public:
    short x;
    short y;

    Point()
    {
        x = 0;
        y = 0;
    }

    Point(const Point& value)
    {
        x = value.x;
        y = value.y;
    }
};

typedef enum  {
    DefectType_0 = 0,
    DefectType_1 = 1,
    DefectType_2 = 2,
    DefectType_3 = 3
} DefectType_t;

class Defect
{
public:
    Point coord;
    DefectType_t type;

    Defect()
    {
        coord.x = 0;
        coord.y = 0;
    }

    Defect(const Defect& defect)
    {
        coord = defect.coord;
        type = defect.type;
    }
};

class SensorTemperatureDie
{
public:
    float current;
    float averaged;

    SensorTemperatureDie()
    {
        current = 0;
        averaged = 0;
    }

    SensorTemperatureDie(const SensorTemperatureDie& die)
    {
        current  = die.current;
        averaged = die.averaged;
    }
};

class SensorTemperature
{
public:
    SensorTemperatureDie die;
    float heatsink;

    SensorTemperature()
    {
        heatsink = 0;
    }

    SensorTemperature(const SensorTemperature& temperature)
    {
        die      = temperature.die;
        heatsink = temperature.heatsink;
    }
};

class DarkMeasurementData
{
public:
    int                 usExposureTime;
    SensorTemperature   sensorTemperature;
    time_t              timeStamp;
    float               biasCompensationCount;

    int16* pData;
    int dataSize;

    DarkMeasurementData()
    {

    }

    DarkMeasurementData(const DarkMeasurementData& dataMatrix)
    {
        usExposureTime = dataMatrix.usExposureTime;

        pData = dataMatrix.pData;
        dataSize = dataMatrix.dataSize;

        sensorTemperature = dataMatrix.sensorTemperature;
        timeStamp = dataMatrix.timeStamp;
        biasCompensationCount = dataMatrix.biasCompensationCount;
    }

#ifdef REMOVED
    reDarkMeasurementData(const MeasurementData& dataMatrix)
    {
        usExposureTime = dataMatrix.recipe.usExposureTime;

        data.clear();
        for(int index = 0; index < (int)dataMatrix.data.data.size(); index ++)
        {
            data.push_back(dataMatrix.data.data.at(index));
        }
        sensorTemperature     = dataMatrix.data.sensorTemperature;
        timeStamp             = dataMatrix.data.timeStamp;
        biasCompensationCount = dataMatrix.data.biasCompensationCount;
    }
#endif
};

template<typename T>
class MeasurementValue
{
public:
    MeasurementValue()
    {
        Reset();
    }

    MeasurementValue(const MeasurementValue& val)
    {
        value = val.value;
        square = val.square;
        count = val.count;
    }

    void Reset()
    {
        value = 0;
        square = 0;
        count = 0;
    }

    void Push(T value_)
    {
        count ++;
#ifndef CUMULATIVE_AVERAGE
        value += value_;
        square += SQUARE(value_);
#else
        Stormhold_AddToAverage<T>(
                    count,
                    (T)value_,
                    &value);

        Stormhold_AddToAverage<T>(
                    count,
                    (T)SQUARE(value_),
                    &square);
#endif
    }

    T GetAverage()
    {
#ifndef CUMULATIVE_AVERAGE
        T res = 0;

        if (count == 0)
        {
            res = value;
        }
        else
        {
            res = value / count;
        }

        return res;
#else
        return value;
#endif
    }

    T GetStdDev()
    {
#ifndef CUMULATIVE_AVERAGE
        T res = 0;

        if(count == 0)
        {
            res = sqrt((double)(square-SQUARE(value)));

            res = CalculateStdDev(value, square);
        }
        else
        {
            double average = GetAverage();
            double squareAv = square / count;

            res = CalculateStdDev(average, squareAv);
        }

        return res;
#else
        return CalculateStdDev(value, square);
#endif
    }

private:
    T value;
    T square;
    int count;

    T CalculateStdDev(T va, T sq)
    {
        return sqrt((double)(sq-SQUARE(va)));
    }
};

class DarkOffset
{
public:
    MeasurementValue<float> fBias;
    MeasurementValue<float> fActive;

    DarkOffset()
    {
    }

    DarkOffset(const DarkOffset& val)
    {
        fBias = val.fBias;
        fActive = val.fActive;
    }
};

class DarkImageInfo
{
public:
    float deltaTime;
    float deltaTemp;
    float deltaBiasCount;

    DarkImageInfo()
    {
        deltaTime = 0;
        deltaTemp = 0;
        deltaBiasCount = 0;
    }

    DarkImageInfo(const DarkImageInfo& info)
    {
        deltaTime = info.deltaTime;
        deltaTemp = info.deltaTemp;
        deltaBiasCount = info.deltaBiasCount;
    }
};

typedef enum
{
    Pipeline_Linearisation_None,
    Pipeline_Linearisation_MXAndFlatField,
    Pipeline_Linearisation_MX
} Pipeline_Linearisation_t;

class LinearizationCoef
{
public:
    float A1;
    float A3;
    float A5;
    float A7;
    float A9;

    LinearizationCoef()
    {

    }

    LinearizationCoef(const LinearizationCoef& value)
    {
        A1 = value.A1;
        A3 = value.A3;
        A5 = value.A5;
        A7 = value.A7;
        A9 = value.A9;
    }
};

class Pipeline_CalibrationParam
{
public:
    bool                          sensorTemperatureDependancy_Enabled;
    float                         sensorTemperatureDependancy_Slope;

    Point                         captureArea_OpticalAxis;

    float                         maximumIncidentAngle;
    short                         calibratedDataRadius;

    LinearizationCoef             linearizationCoefficients;

    // std::vector<Base64Binary>     flatField;
    std::vector<char>*             flatField;

    float                         conversionFactor_Value;
    SensorTemperature             conversionFactor_SensorTemperature;

    Pipeline_CalibrationParam()
    {

    }

    Pipeline_CalibrationParam(const Pipeline_CalibrationParam& calib)
    {
        sensorTemperatureDependancy_Enabled = calib.sensorTemperatureDependancy_Enabled;
        sensorTemperatureDependancy_Slope   = calib.sensorTemperatureDependancy_Slope;

        captureArea_OpticalAxis             = calib.captureArea_OpticalAxis;

        maximumIncidentAngle                = calib.maximumIncidentAngle;
        calibratedDataRadius                = calib.calibratedDataRadius;
        linearizationCoefficients           = calib.linearizationCoefficients;

        flatField                           = calib.flatField;

        conversionFactor_Value              = calib.conversionFactor_Value;
        conversionFactor_SensorTemperature  = calib.conversionFactor_SensorTemperature;
    }
};

class Pipeline_RawDataParam
{
public:
    ImageSize                        imageSize;

    // step 1 - DEFECT PIXELS
    bool                             sensorDefectEnable;
    bool                             sensorDefects_correctionEnabled;

    std::vector<Defect>              sensorDefects_pixels;

    // step 2 - BIAS
    bool                             bias_compensationEnabled;
    short                            bias_sensorSaturation;
    // step 2 - BIAS - output
    int16*                           lastOffSet;

    // step 3 - DarkImage
    bool                             darkMeasurementEnable;
    DarkMeasurementData              darkMeasurement;

    // step 3 - DarkImage current capture
    int                              recipe_usExposureTime;
    SensorTemperature                sensorTemperature;
    time_t                           timeStamp;

    // step 4 - PRNU
    bool                             prnuEnable;
    float                            prnuScaleFactor;
    bool                             prnuCorrectionEnabled;
    std::vector<char>*               prnuData;

    Pipeline_RawDataParam()
    {
        sensorDefectEnable = false;
        sensorDefects_correctionEnabled = false;
        bias_compensationEnabled = false;
        bias_sensorSaturation = 0;
        lastOffSet = NULL;
        darkMeasurementEnable = false;
        recipe_usExposureTime = 0;
        prnuEnable = false;
        prnuScaleFactor = 0;
        prnuCorrectionEnabled = false;
    }
};

class Pipeline_ResultRawDataParam
{
public:
    ImageSize               imageSize;
    int16                   maxBinaryValue;
    bool                    saturationOccurs;
    float                   saturationScore;
    DarkOffset              darkOffset;
    MeasurementValue<float> fullSensor;
    int32                   iDefects;
    DarkImageInfo           darkImageInfo;
    int16                   darkCurrentBiasValue;
    time_t                  timeStamp;
    SensorTemperature       sensorTemperature;
    bool                    saturationFlag;
    float                   saturationLevel;

    Pipeline_ResultRawDataParam()
    {
        maxBinaryValue = 0;
        iDefects = 0;
        darkCurrentBiasValue = 0;
    }
};

class Pipeline_KLibDataParam
{
public:
    ImageSize         imageSize;
    ImageSize         activeArea;

    Pipeline_Linearisation_t linearisation;

    bool  conversionFactorCorrection;
    bool  isCalibrated;
    short sensorSaturationValue;
    bool  applyFlatField;

    // callback for memory allocation
    int16* (*OutputMemoryAllocator)(int memorySize);

    Pipeline_KLibDataParam()
    {
        linearisation = Pipeline_Linearisation_None;
        conversionFactorCorrection = false;
        isCalibrated = false;
        sensorSaturationValue = 0; // SENSOR_SATURATION;
        applyFlatField = true;
        OutputMemoryAllocator = NULL;

        imageSize.Set(3584, 2528);
    }
};

class Pipeline_DataIn
{
public:
    float             conversionFactor;
    short             maxBinaryValue;
    SensorTemperature sensorTemperature;

    int16* pData;
    int dataSize;

    Pipeline_DataIn()
    {

    }

    Pipeline_DataIn(const Pipeline_DataIn& dataMatrix)
    {
        conversionFactor      = dataMatrix.conversionFactor;
        maxBinaryValue        = dataMatrix.maxBinaryValue;
        sensorTemperature     = dataMatrix.sensorTemperature;

        pData = dataMatrix.pData;
        dataSize = dataMatrix.dataSize;
    }
};

class Pipeline_DataOut
{
public:
    bool              isCalibrated;                     // <- set during process
    float             conversionFactor;                 // <- updated parameter
    float             maximumIncidentAngle;             // <- set during process
    short             radius;                           // <- set during process
    float             saturationScore;                  // <- set during process

    Pipeline_DataOut()
    {

    }

    Pipeline_DataOut(const Pipeline_DataOut& dataMatrix)
    {
        isCalibrated          = dataMatrix.isCalibrated;
        conversionFactor      = dataMatrix.conversionFactor;
        maximumIncidentAngle  = dataMatrix.maximumIncidentAngle;
        radius                = dataMatrix. radius;
        saturationScore       = dataMatrix.saturationScore;
    }
};

#endif
