#include "PipelineCompute.h"
#include "PipelineDefectCorrector.h"

//#define ONE_LOOP

#define ONE_LOOP_1
#define ONE_LOOP_2
#define ONE_LOOP_3

#define OMP_PARAL

#define MAX_VALUE(a, b) ((a) < (b)) ? (b) : (a)
#define MIN_VALUE(a, b) ((a) > (b)) ? (b) : (a)

// don't use the size define but the configuration structure instead
// so it is possible to change image size without recompiling

#define _IMAGE_HEIGHT                           m_ImageConfiguration.image_height
#define _IMAGE_WIDTH                            m_ImageConfiguration.image_width

#define _TOP_HEIGHT                             m_ImageConfiguration.top_height
#define _TOP_WIDTH                              m_ImageConfiguration.top_width
#define _TOP_VERTICAL_OFFSET                    m_ImageConfiguration.top_vertical_offset
#define _TOP_HORIZONTAL_OFFSET                  m_ImageConfiguration.top_horizontal_offset

#define _LEFT_HEIGHT                            m_ImageConfiguration.left_height
#define _LEFT_WIDTH                             m_ImageConfiguration.left_width
#define _LEFT_VERTICAL_OFFSET                   m_ImageConfiguration.left_vertical_offset
#define _LEFT_HORIZONTAL_OFFSET                 m_ImageConfiguration.left_horizontal_offset

#define _RIGHT_HEIGHT                           m_ImageConfiguration.right_height
#define _RIGHT_WIDTH                            m_ImageConfiguration.right_width
#define _RIGHT_VERTICAL_OFFSET                  m_ImageConfiguration.right_vertical_offset
#define _RIGHT_HORIZONTAL_OFFSET                m_ImageConfiguration.right_horizontal_offset

#define _BIAS_CORNER_AREA_HEIGHT                m_ImageConfiguration.bias_corner_area_height
#define _BIAS_CORNER_AREA_WIDTH                 m_ImageConfiguration.bias_corner_area_width

#define _ACTIVE_HORIZONTAL_OFFSET               m_ImageConfiguration.active_horizontal_offset
#define _ACTIVE_VERTICAL_OFFSET                 m_ImageConfiguration.active_vertical_offset
#define _ACTIVE_WIDTH                           m_ImageConfiguration.active_width
#define _ACTIVE_HEIGTH                          m_ImageConfiguration.active_height

#define _ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET  m_ImageConfiguration.active_central_area_horizontal_offset
#define _ACTIVE_CENTRAL_AREA_VERTICAL_OFFSET    m_ImageConfiguration.active_central_area_vertical_offset
#define _ACTIVE_CENTRAL_AREA_WIDTH              m_ImageConfiguration.active_central_area_width
#define _ACTIVE_CENTRAL_AREA_HEIGHT             m_ImageConfiguration.active_central_area_height

#define INDEX(j, i) ((j*_IMAGE_WIDTH) + i)

#define T_INDEX(x) x + _ACTIVE_VERTICAL_OFFSET
#define B_INDEX(x) (_IMAGE_HEIGHT - 1) - x
#define L_INDEX(x) x + _ACTIVE_HORIZONTAL_OFFSET
#define R_INDEX(x) (_ACTIVE_HORIZONTAL_OFFSET + _ACTIVE_WIDTH - 1) - x

#define TL_INDEX(j, i) INDEX((T_INDEX(j)), (L_INDEX(i)))
#define TR_INDEX(j, i) INDEX((T_INDEX(j)), (R_INDEX(i)))
#define BL_INDEX(j, i) INDEX((B_INDEX(j)), (L_INDEX(i)))
#define BR_INDEX(j, i) INDEX((B_INDEX(j)), (R_INDEX(i)))

#define MAX_ANGLE 70

#define HISTOGRAM if(histogram != NULL) histogram

#define debugPrint(text)     DebugLogger::Print(text)

#define INSTANCE(instance) PipelineCompute* instance = getInstance()

PipelineCompute* PipelineCompute::m_instance = NULL;

PipelineCompute::PipelineCompute()
{
    m_logger = NULL;
}

PipelineCompute::~PipelineCompute()
{
}

PipelineCompute* PipelineCompute::getInstance()
{
    if(m_instance == NULL)
    {
        m_instance = new PipelineCompute();
    }

    return m_instance;
}

void PipelineCompute::SetLogger(Logger* logger)
{
    INSTANCE(instance);
    instance->m_logger = logger;
}

void PipelineCompute::SetImageConfiguration(
        ImageConfiguration& imageConfiguration)
{
    INSTANCE(instance);
    instance->m_ImageConfiguration = imageConfiguration;
    instance->m_ImageConfigurationRef = imageConfiguration;

    PipelineDefectCorrector::SetImageConfiguration(imageConfiguration);
}

int PipelineCompute::GetCalibratedDataSize(short calibratedDataRadius)
{
    INSTANCE(instance);
    return instance->_GetCalibratedDataSize(calibratedDataRadius);
}

Error_t PipelineCompute::ComputeRawData(
        int16*                         rawData,
        const Pipeline_RawDataParam*   param,
        Histogram*                     pHistogram,
        Pipeline_ResultRawDataParam&   resultParam)
{
    INSTANCE(instance);
    return instance->_ComputeRawData(
                rawData,
                param,
                pHistogram,
                resultParam);
}

Error_t PipelineCompute::ComputeKLibData(
        const Pipeline_KLibDataParam& param,
        Pipeline_CalibrationParam*    calibration,
        const Pipeline_DataIn*        rawData,
        Pipeline_DataOut*             calibDataOut,
        int16*                        calibratedData)
{
    INSTANCE(instance);
    return instance->_ComputeKLibData(
                param,
                calibration,
                rawData,
                calibDataOut,
                calibratedData);
}

Error_t PipelineCompute::_ComputeRawData(
        int16*                       rawData,
        const Pipeline_RawDataParam* param,
        Histogram*                   pHistogram,
        Pipeline_ResultRawDataParam& resultParam)
{
    appendLogFile("_ComputeRawData");

    Error_t res = Error_Ok;

    //statistics variables
    int16 maxBinaryValue = 0;
    bool  saturationOccurs;
    float saturationScore;

    DarkOffset    darkOffset;
    DarkImageInfo darkImageInfo;

    MeasurementValue<float> fullSensor;

    int32 iDefects = 0;
    int16 darkCurrentBiasValue = 0;

    // check input parameters validity
    if(rawData == NULL)
    {
        res = Error_InputDataNone;
    }

    if(res == Error_Ok)
    {
        _SetImageConfiguration(param->imageSize);
    }

    //first correct defective pixels before any additional process
    if(res == Error_Ok)
    {
        iDefects = ComputeWrongBands(rawData, INACTIVE_AREA_THRESHOLD, &param->imageSize);
    }

    if(res == Error_Ok)
    {
        if((param->sensorDefects_correctionEnabled == true) &&
           (param->sensorDefectEnable == true))
        {
            appendLogFile("Defect correction");
            PipelineDefectCorrector::Correct(rawData, param->imageSize, &param->sensorDefects_pixels);
        }
    }

    float saturationLevel = 0.0;
    bool saturationFlag = false;

    if(res == Error_Ok)
    {
        // check if the capture is saturated
        // compare all pixels with sensorSaturation value

        uint16_t pixelMax = 0;
        // uint16_t saturationValue = 4095;
        uint16_t saturationValue = param->bias_sensorSaturation;

        for (int i = 0; i < param->imageSize.nbPixels; i++)
        {
             if((uint16_t)rawData[i] >= saturationValue)
             {
                saturationFlag = true;
             }

             if((uint16_t)rawData[i] > pixelMax)
             {
                 pixelMax = (uint16_t)rawData[i];
             }
        }

        // calculate the saturation level
        saturationLevel = (float)pixelMax / (float)saturationValue;
    }

    if(res == Error_Ok)
    {
        // then extract max value + bias compensation if requested in the pipeline

        // test with force bias compensation off with flag
        darkCurrentBiasValue = 0;

        int16 iDarkCurrentBiasValue = 0;

        if(param->bias_compensationEnabled == true)
        {
            appendLogFile("Bias substraction");
            iDarkCurrentBiasValue = ComputedBiasSubtraction(rawData, param->lastOffSet);
            darkCurrentBiasValue = iDarkCurrentBiasValue;
        }
        else
        {
            appendLogFile("Bias substraction disabled");
            darkCurrentBiasValue = ComputedBiasSubtraction(rawData, param->lastOffSet);
        }

        ApplyBias(rawData, param->imageSize.nbPixels, iDarkCurrentBiasValue, fullSensor);

        DarkOffsetCalculation(rawData, param->imageSize, pHistogram, darkOffset, maxBinaryValue);

        saturationOccurs = (maxBinaryValue >= param->bias_sensorSaturation);
        saturationScore  = (float)maxBinaryValue/(float)param->bias_sensorSaturation;
    }

    if(res == Error_Ok)
    {
        if(param->darkMeasurementEnable == true)
        {
            if(param->darkMeasurement.dataSize != 0)
            {
                int16* darkArray = new int16[param->darkMeasurement.dataSize];
                memcpy(&darkArray[0],
                       param->darkMeasurement.pData,
                       param->darkMeasurement.dataSize);

                // disable scaling factor as no proportionality between exposure time due to electronic offset
                // if exposure time is different from dark measurement, do not apply correction (5% tolerance)

                float usExposureTimeUs     = (float)param->recipe_usExposureTime;
                float usExposureTimeUsDark = (float)param->darkMeasurement.usExposureTime;
                float fScaling             = usExposureTimeUs / usExposureTimeUsDark;

                if((fScaling <= 1.05) &&
                   (fScaling >= 0.95))
                {
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
                    for (int i = 0; i < param->imageSize.nbPixels; i++)
                    {
                        rawData[i] = (int16)(rawData[i] - darkArray[i]);
                        //rawData[i] = (int16)((float)rawData[i] - fScaling*(float)darkArray[i]);
                    }

                    darkImageInfo.deltaBiasCount = (int16)((float)darkCurrentBiasValue - fScaling * param->darkMeasurement.biasCompensationCount);

                    darkImageInfo.deltaTemp = param->sensorTemperature.die.averaged - param->darkMeasurement.sensorTemperature.die.averaged;

                    darkImageInfo.deltaTime = param->timeStamp - param->darkMeasurement.timeStamp;
                }
                else
                {
                    darkImageInfo.deltaBiasCount = -1;
                    darkImageInfo.deltaTemp = -1;
                    darkImageInfo.deltaTime = -1;
                }
                delete[] darkArray;
            }
        }
        else
        {
            darkImageInfo.deltaBiasCount = 0;
            darkImageInfo.deltaTemp = 0;
            darkImageInfo.deltaTime = 0;
        }
    }

    if(res == Error_Ok)
    {
        if(param->prnuEnable == true)
        {
            if(param->prnuCorrectionEnabled == true)
            {
                appendLogFile("PRNU correction");

                SensorPrnuCorrection(rawData,
                                     param->imageSize.nbPixels,
                                     param->prnuData,
                                     param->prnuScaleFactor);
            }
            else
            {
                appendLogFile("PRNU correction skipped (no PRNU calibration present)");
            }
        }
        else
        {
            appendLogFile("PRNU correction disabled");
        }
    }
    // End Pipeline test

    // fill output
    resultParam.imageSize            = param->imageSize;
    resultParam.maxBinaryValue       = maxBinaryValue;
    resultParam.saturationOccurs     = saturationOccurs;
    resultParam.saturationScore      = saturationScore;

    resultParam.darkOffset           = darkOffset;
    resultParam.fullSensor           = fullSensor;
    resultParam.iDefects             = iDefects;
    resultParam.darkImageInfo        = darkImageInfo;
    resultParam.darkCurrentBiasValue = darkCurrentBiasValue;

    resultParam.timeStamp            = param->timeStamp;

    resultParam.sensorTemperature    = param->sensorTemperature;

    resultParam.saturationFlag       = saturationFlag;
    resultParam.saturationLevel      = saturationLevel;

    return res;
}

int PipelineCompute::_GetCalibratedDataSize(short calibratedDataRadius)
{
    // output is a square picture
    return (2 * calibratedDataRadius + 1)*(2 * calibratedDataRadius + 1);
}

Error_t PipelineCompute::_ComputeKLibData(
        const Pipeline_KLibDataParam&    param,
        const Pipeline_CalibrationParam* calibration,
        const Pipeline_DataIn*           rawData,
        Pipeline_DataOut*                calibDataOut,
        int16*                           calibratedData)
{
    Error_t     eError = Error_Ok;
#ifdef FLOAT_CALIBRATION_FACTOR
    // note: this is no use
    float*  floatCalibratedData;
#endif
    bool   mFlatFieldPrecomputed = false;

    float  conversionFactor = 0;
    int16* pCalibratedData = NULL;
    int16* rawDataArray = NULL;

    // check parameters
    if(calibration == NULL)
    {
        eError = Error_CalibrationNull;
    }

    if(rawData == NULL)
    {
        eError = Error_InputDataNull;
    }

    if(rawData->dataSize == 0)
    {
        eError = Error_InputDataNone;
    }

    // retrieve (and allocate a flat field buffer) flat field buffer
    int16* flatFieldbuffer = NULL;

    if (eError == Error_Ok)
    {
        flatFieldbuffer = (int16*)calibration->flatField->data();

        if (flatFieldbuffer == NULL)
        {
            eError = Error_FlatFieldDataNull;
        }
    }

    if(param.linearisation != Pipeline_Linearisation_None)
    {
        // check optical center
#define CHECK_ACTIVE_AREA
#ifdef CHECK_ACTIVE_AREA
        int  active_width              = param.activeArea.width;
        int  active_height             = param.activeArea.height;
        int  active_horizontal_offset  = param.activeArea.offsetX;
        int  active_vertical_offset    = param.activeArea.offsetY;

        // long lSize = (long)(2 * calibration->calibratedDataRadius + 1);
        long fMaxAngle = calibration->maximumIncidentAngle;

        float sensorRadius = _GetRadius(fMaxAngle, &calibration->linearizationCoefficients);

        // 1- check if center matches with active area
        // 2- check if center matches with sensor area
        // because center can matches 1 and not 2

        if((calibration->captureArea_OpticalAxis.x < sensorRadius + active_horizontal_offset) ||
           (calibration->captureArea_OpticalAxis.x > (active_horizontal_offset + active_width - sensorRadius)))
        {
            eError = Error_OpticalAxisOutOfActiveArea;
        }

        if((calibration->captureArea_OpticalAxis.y < sensorRadius + active_vertical_offset) ||
           (calibration->captureArea_OpticalAxis.y > (active_vertical_offset + active_height - sensorRadius)))
        {
            eError = Error_OpticalAxisOutOfActiveArea;
        }

        if(eError == Error_OpticalAxisOutOfActiveArea)
        {
#endif
            if((calibration->captureArea_OpticalAxis.x < sensorRadius) ||
               (calibration->captureArea_OpticalAxis.x > (2 * active_horizontal_offset + active_width - sensorRadius)))
            {
                eError = Error_OpticalAxisNull;
            }

            if((calibration->captureArea_OpticalAxis.y < sensorRadius) ||
               (calibration->captureArea_OpticalAxis.y > (2 * active_vertical_offset + active_height - sensorRadius)))
            {
                eError = Error_OpticalAxisNull;
            }
#ifdef CHECK_ACTIVE_AREA
        }
#endif
    }

    if(eError == Error_Ok)
    {
        _SetImageConfiguration(param.imageSize);
    }

    if (eError == Error_Ok)
    {
        // this is not generic at all and should be modified
        rawDataArray = rawData->pData;

        // allocate a buffer for output calibration data
        if(calibratedData != NULL)
        {
            // buffer must be allocated by the caller but have to check size matches
            pCalibratedData = (int16*)calibratedData;
        }

#ifdef FLOAT_CALIBRATION_FACTOR
        // to move after
        floatCalibratedData = new float[(2 * calibration->calibratedDataRadius + 1)*(2 * calibration->calibratedDataRadius + 1)];
#endif

        if(param.linearisation == Pipeline_Linearisation_MXAndFlatField)
        {
            MXLinearizeAndFlatField(
                rawDataArray,
                param.imageSize.width,
                (long)(2 * calibration->calibratedDataRadius + 1),
                calibration->maximumIncidentAngle,
                calibration->captureArea_OpticalAxis,
                &calibration->linearizationCoefficients,
                flatFieldbuffer,
                (long)(2 * calibration->calibratedDataRadius + 1)*(2 * calibration->calibratedDataRadius + 1),
                (int)flatFieldbuffer[(2 * calibration->calibratedDataRadius + 2)*calibration->calibratedDataRadius],
                pCalibratedData,
                &mFlatFieldPrecomputed,
                param.applyFlatField);
        }
        else if(param.linearisation == Pipeline_Linearisation_MX)
        {
            // Linearize
            // we have just linearization : apply old function
            MXLinearize(
                rawDataArray,
                param.imageSize.width,
                (long)(2 * calibration->calibratedDataRadius + 1),
                calibration->maximumIncidentAngle,
                calibration->captureArea_OpticalAxis,
                &calibration->linearizationCoefficients,
                EXCLUDED_VALUE,
                pCalibratedData);
        }
        // if(param.linearisation == Pipeline_Linearisation_None)
        else
        {
            short dataRadius = calibration->calibratedDataRadius;

            // in that case, pCalibratedData does not contains any data
            // int lineLength = (2 * calibration->calibratedDataRadius + 1);
            int lineLength = (2 * dataRadius + 1);

            if((lineLength <= param.imageSize.height) &&
               (lineLength <= param.imageSize.width))
            {
                int heightOffset = (param.imageSize.height - lineLength) / 2;
                int widthOffset  = (param.imageSize.width - lineLength) / 2;

                int16* inputPtr = rawDataArray;
                int16* outputPtr = pCalibratedData;

                // for(int lineIndex = 0; lineIndex < 2 * calibration->calibratedDataRadius + 1; lineIndex ++)
                for(int lineIndex = 0; lineIndex < lineLength; lineIndex ++)
                {
                    int outputOffset = lineIndex * lineLength;
                    int inputOffset = widthOffset + ((lineIndex + heightOffset) * param.imageSize.width);

                    memcpy(&outputPtr[outputOffset],
                           &inputPtr[inputOffset],
                           lineLength * sizeof(int16_t));
                }
            }
            /*
            else
            {
                lineLength = MIN_VALUE(param.imageSize.height, param.imageSize.width);
            }
            */
        }

        if(param.conversionFactorCorrection == true)
        {
            conversionFactor = rawData->conversionFactor;

            //Adjust Exposure time based on difference between absolute calibration measurmeent and current measurement
            // test correction temperature flag
            if (calibration->sensorTemperatureDependancy_Enabled)
            {
                conversionFactor /=
                    (1 - (calibration->conversionFactor_SensorTemperature.die.averaged - rawData->sensorTemperature.die.averaged) *
                    calibration->sensorTemperatureDependancy_Slope);
            }

            //Correct with absolute value
            conversionFactor *= calibration->conversionFactor_Value;
        }

        // delete[] rawDataArray;

        if (calibration->maximumIncidentAngle != MAX_ANGLE)
        {
            RestrictToViewingAngle(
                calibration->maximumIncidentAngle,
                calibration->calibratedDataRadius,
                pCalibratedData);
        }

        calibDataOut->conversionFactor      = conversionFactor;
        calibDataOut->isCalibrated          = param.isCalibrated;
        calibDataOut->maximumIncidentAngle  = calibration->maximumIncidentAngle;
        calibDataOut->radius                = calibration->calibratedDataRadius;
        calibDataOut->saturationScore       = ((float)rawData->maxBinaryValue / (float)param.sensorSaturationValue); // SENSOR_SATURATION);

#ifdef FLOAT_CALIBRATION_FACTOR
        delete[] floatCalibratedData;
#endif
    }

    return eError;
}

int PipelineCompute::MXLinearizeAndFlatField(
        int16*  pintSource,
        long    lSourceWidth,
        long    lSize,
        float   fMaxAngle,
        Point   center,
        const LinearizationCoef*  plinearizationFactor,
        int16*  pintFlatField,
        long    lSize2,
        long    lScaleFactor,
        int16*  pintTarget,
        bool*   flatFieldPrecomputed,
        bool    applyFlatField)
{
    long    lSourceRow, lSourceColumn;
    long    lTargetHeight, lTargetWidth;
    long    lTargetRow, lTargetColumn, lTargetAxis, lTargetRadius;

    float   fAlpha, fBeta;
    float   fRow, fColumn;
    float   fPixel;
    double  dPixel;

    int16 * psPixel;

    precomputedData_t* pPrecomputedData = NULL;

    // Linearization
    // -------------
    lTargetHeight = lSize;
    lTargetWidth  = lSize;
    lTargetAxis   = (lTargetWidth - 1) / 2;
    lTargetRadius = lTargetAxis;

    PrecomputeLinearizationTables(
                lTargetWidth,
                lTargetHeight,
                lTargetRadius,
                lTargetAxis,
                plinearizationFactor,
                fMaxAngle,
                center,
                &pPrecomputedData);

    PrecomputeFlatFieldInverse(
                pintFlatField,
                lScaleFactor,
                lSize2,
                flatFieldPrecomputed,
                applyFlatField,
                pPrecomputedData);

    //---- Linearization computation ----
    for(lTargetRow = 0; lTargetRow < lTargetHeight; lTargetRow++, pintTarget += lTargetWidth)
    {
        long lRowOffset = lTargetWidth * lTargetRow;

        for(lTargetColumn = 0; lTargetColumn < lTargetWidth; lTargetColumn++)
        {
            long lPosition = lRowOffset + lTargetColumn;

            if (pPrecomputedData[lPosition].linearizationRadius)
            {
                fRow    = pPrecomputedData[lPosition].linearizationTableY;
                fColumn = pPrecomputedData[lPosition].linearizationTableX;

                lSourceRow    = (long)(fRow);
                lSourceColumn = (long)(fColumn);
                fAlpha = fRow    - lSourceRow;
                fBeta  = fColumn - lSourceColumn;

                psPixel = pintSource + lSourceWidth * lSourceRow + lSourceColumn;
                fPixel = (1 - fBeta) * psPixel[0] + fBeta * psPixel[1];
                psPixel = psPixel + lSourceWidth;
                fPixel = (1 - fAlpha) * fPixel + fAlpha * ((1 - fBeta) * psPixel[0] + fBeta * psPixel[1]);

                dPixel = (double)fPixel * pPrecomputedData[lPosition].flatFieldInverse;

                if (dPixel < SHORT_MAXIMUM)
                {
                    pintTarget[lTargetColumn] = (short)dPixel;
                }
                else
                {
                    pintTarget[lTargetColumn] = SHORT_MAXIMUM;
                }
            }
            else
            {
                // "Outside" pixels are zeroed
                pintTarget[lTargetColumn] = 0;
            }
        }
    }

    // delete allocated data
    if(pPrecomputedData != NULL)
    {
        delete (pPrecomputedData);
        pPrecomputedData = NULL;
    }

    return 0;
}

float CorrectFactor(float factor, float reductionFactor, int exp)
{
    float res = 0;

    if (factor != 0)
    {
        res = factor * pow(reductionFactor, exp);
    }

    return res;
}

#define CORRECT_FACTOR(a, b)  CorrectFactor(pLinearizationFactor->A##a, b, (a))

Error_t PipelineCompute::PrecomputeLinearizationTables(
        long lTargetWidth,
        long lTargetHeight,
        long lTargetRadius,
        long lTargetAxis,
        const LinearizationCoef*  pLinearizationFactor,
        float fMaxAngle,
        Point center,
        precomputedData_t** ppPrecomputedData)
{
    float   fReductionFactor, fB0, fB2, fB4, fB6, fB8, fCorrection;
    long    lRadiusSquare, lMaximumRadiusSquare;
    double  dRadiusSquare;
    long    lTargetRow, lTargetColumn;
    float   fRow, fColumn;
    Error_t eError = Error_Ok;

    precomputedData_t* pPrecomputedData = new precomputedData_t[lTargetHeight*lTargetWidth]();
    *ppPrecomputedData = pPrecomputedData;

    if (pPrecomputedData != NULL)
    {
        lMaximumRadiusSquare = SQUARE(lTargetRadius);
        fReductionFactor = fMaxAngle / (lTargetRadius * 90); // Enable conversion between nRadiusResult (in Pixels) to fReducedAngle

        fB0 = CORRECT_FACTOR(1, fReductionFactor);
        fB2 = CORRECT_FACTOR(3, fReductionFactor);
        fB4 = CORRECT_FACTOR(5, fReductionFactor);
        fB6 = CORRECT_FACTOR(7, fReductionFactor);
        fB8 = CORRECT_FACTOR(9, fReductionFactor);

#ifdef ONE_LOOP_2
/*
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
        for(int index = 0; index < (lTargetHeight * lTargetWidth); index ++)
        {
            lTargetRow = index / lTargetWidth;
            // lTargetColumn = index % lTargetWidth;
            lTargetColumn = index - (lTargetRow * lTargetWidth);
*/
        int index = 0;

        for (lTargetRow = 0; lTargetRow < lTargetHeight; lTargetRow++)
        {
            #pragma omp parallel for num_threads(4)
            for (lTargetColumn = 0; lTargetColumn < lTargetWidth; lTargetColumn++)
            {
                index = lTargetWidth*lTargetRow + lTargetColumn;

#else
        int index = 0;

        for (lTargetRow = 0; lTargetRow < lTargetHeight; lTargetRow++)
        {
            for (lTargetColumn = 0; lTargetColumn < lTargetWidth; lTargetColumn++)
            {
                index = lTargetWidth*lTargetRow + lTargetColumn;
#endif

                lRadiusSquare = SQUARE(lTargetRow - lTargetAxis) + SQUARE(lTargetColumn - lTargetAxis);
                pPrecomputedData[index].linearizationRadius = (lRadiusSquare <= (lMaximumRadiusSquare + 1));

                if (pPrecomputedData[index].linearizationRadius)
                {
                    dRadiusSquare = (double)lRadiusSquare;
                    fCorrection = fB0 + dRadiusSquare * (fB2
                        + dRadiusSquare * (fB4
                            + dRadiusSquare * (fB6
                                + dRadiusSquare * (fB8))));

                    fRow    = center.y + fCorrection * (lTargetRow    - lTargetAxis);
                    fColumn = center.x + fCorrection * (lTargetColumn - lTargetAxis);

                    pPrecomputedData[index].linearizationTableY = fRow;
                    pPrecomputedData[index].linearizationTableX = fColumn;
                }
#ifndef ONE_LOOP_2
            }
#else
            }
#endif
        }
    }
    else
    {
        eError = Error_PrecomputedData;
    }

    return eError;
}

Error_t PipelineCompute::PrecomputeFlatFieldInverse(
        int16*   pint16Denominator,
        long     lScaleFactor,
        long     lSize,
        bool*    flatFieldPrecomputed,
        bool     applyFlatField,
        precomputedData_t *pPrecomputedData)
{
    long lIndex;

    if(!*flatFieldPrecomputed)
    {
        if(applyFlatField == true)
        {
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
            for (lIndex = 0; lIndex < lSize; lIndex++)
            {
                if (pint16Denominator[lIndex] != 0)
                {
                    pPrecomputedData[lIndex].flatFieldInverse = (double)lScaleFactor / ((double)pint16Denominator[lIndex]);
                }
                else
                {
                    pPrecomputedData[lIndex].flatFieldInverse = 0;
                }
            }
        }
        else
        {
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
            for (lIndex = 0; lIndex < lSize; lIndex++)
            {
                pPrecomputedData[lIndex].flatFieldInverse = 1;
            }
        }

        *flatFieldPrecomputed = true;
    }

    return Error_Ok;
}

Error_t PipelineCompute::MXLinearize(
    int16* pintSource,
    long lSourceWidth,
    long lSize,
    float fMaxAngle,
    Point center,
    const LinearizationCoef*  pLinearizationFactor,
    long lExcluded,
    int16 * pintTarget)
{
    //----------------------------------------------------------

    // fMaximumAngle is the Maximum incident angle in the destination (pvarResult)
    // EDF structure (fMaximumAngle is given in °).
    // fA1 and fA3 are the order 1 and 3 coefficients of a polynomial
    // approximating the angular law. This law is given by :
    //      Radius(in varSource) = fA1 * ReducedAngle + fA3 * ReducedAngle^3
    //      with ReducedAngle = Angle/90
    // In order to compute the required pixel location in varData from a given
    // pixel location in pvarTarget, fCorrection is used :
    //      fCorrection = Radius(in varSource) / fRadius
    long   lSourceRow, lSourceColumn;

    long   lTargetHeight, lTargetWidth;
    long   lTargetRow, lTargetColumn, lTargetAxis, lTargetRadius;

    long   lRadiusSquare, lMaximumRadiusSquare;
    double dRadiusSquare;

    float  fReductionFactor, fB0, fB2, fB4, fB6, fB8, fCorrection;

    float  fAlpha, fBeta;
    float  fRow, fColumn;
    float  fPixel;

    int16 * psPixel;

    // Linearization
    // -------------

    lTargetHeight = lSize;
    lTargetWidth = lSize;
    lTargetAxis = (lTargetWidth - 1) / 2;
    lTargetRadius = lTargetAxis;

    lMaximumRadiusSquare = SQUARE(lTargetRadius);

    fReductionFactor = fMaxAngle / (lTargetRadius * 90); // Enable conversion between nRadiusResult (in Pixels) to fReducedAngle

    fB0 = CORRECT_FACTOR(1, fReductionFactor);
    fB2 = CORRECT_FACTOR(3, fReductionFactor);
    fB4 = CORRECT_FACTOR(5, fReductionFactor);
    fB6 = CORRECT_FACTOR(7, fReductionFactor);
    fB8 = CORRECT_FACTOR(9, fReductionFactor);

    //---- Linearization computation ----
#ifdef ONE_LOOP
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
    for(int index = 0; index < (lTargetHeight * lTargetWidth); index ++)
    {
        lTargetRow = index / lTargetWidth;
        // lTargetColumn = index % lTargetWidth;
        lTargetColumn = index - (lTargetRow * lTargetWidth);

        if((lTargetColumn == 0) && (index != 0))
        {
            pintTarget += lTargetWidth;
        }
#else
    for(lTargetRow = 0; lTargetRow < lTargetHeight; lTargetRow++, pintTarget += lTargetWidth)
    {
        for(lTargetColumn = 0; lTargetColumn < lTargetWidth; lTargetColumn++)
        {
#endif
            lRadiusSquare = SQUARE(lTargetRow - lTargetAxis) + SQUARE(lTargetColumn - lTargetAxis);

            if (lRadiusSquare <= (lMaximumRadiusSquare + 1))
            {
                dRadiusSquare = (double)lRadiusSquare;
                //fCorrection = 1;
                fCorrection = fB0 + dRadiusSquare * (fB2
                    + dRadiusSquare * (fB4
                        + dRadiusSquare * (fB6
                            + dRadiusSquare * (fB8))));

// #define DEBUG_CORRECTION
#ifdef DEBUG_CORRECTION
                fCorrection = 1;
#endif

                fRow    = center.y + fCorrection * (lTargetRow    - lTargetAxis);
                fColumn = center.x + fCorrection * (lTargetColumn - lTargetAxis);

                fAlpha = (float)fmod((double)fRow, 1);
                fBeta = (float)fmod((double)fColumn, 1);
                lSourceRow = (long)(fRow - fAlpha);
                lSourceColumn = (long)(fColumn - fBeta);

                // for debug change the way to set the pointer
                // psPixel = pintSource + lSourceWidth * lSourceRow + lSourceColumn;

                int pixelIndex = lSourceWidth * lSourceRow + lSourceColumn;
                psPixel = &pintSource[pixelIndex];

                if ((psPixel[0] != (int16)lExcluded) && (psPixel[1] != (int16)lExcluded))
                {
                    fPixel = (1 - fBeta) * psPixel[0] + fBeta * psPixel[1];
                    psPixel = psPixel + lSourceWidth;
                    if ((psPixel[0] != (int16)lExcluded) && (psPixel[1] != (int16)lExcluded))
                    {
                        fPixel = (1 - fAlpha) * fPixel + fAlpha * ((1 - fBeta) * psPixel[0] + fBeta * psPixel[1]);
                        pintTarget[lTargetColumn] = (int16)fPixel;
                    }
                    else
                    {
                        pintTarget[lTargetColumn] = (int16)lExcluded;
                    }
                }
                else
                {
                    pintTarget[lTargetColumn] = (int16)lExcluded;
                }
            }
            else
            {
                pintTarget[lTargetColumn] = 0; // "Outside" pixels are zeroed
            }
#ifndef ONE_LOOP
        }
#endif
    }

    return Error_Ok;
}

void PipelineCompute::RestrictToViewingAngle(
        float  maximumIncidentAngle,
        short  calibratedDataRadius,
        int16* pCalibratedData)
{
    //Restrict to specified ViewingAngle removing values over mCalibration.maximumIncidentAngle
    double dRadius;
    long lCount = 0;

    dRadius = (double)calibratedDataRadius / maximumIncidentAngle * (double)maximumIncidentAngle;
    short sRadiusTheo = calibratedDataRadius;

#ifdef ONE_LOOP_3
/*
    int length = 2 * sRadiusTheo + 1;
    int height = 2 * sRadiusTheo + 1;

    int rowIndex;
    int colIndex;

#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
    for(int index = 0; index < length * height; index ++)
    {
        rowIndex = index / length;
        // colIndex = index % length;
        colIndex = index - (rowIndex * length);

        // if(sqrt((colIndex - sRadiusTheo)*(colIndex - sRadiusTheo) + (rowIndex - sRadiusTheo)*(rowIndex - sRadiusTheo)) > dRadius)
        if(((colIndex - sRadiusTheo)*(colIndex - sRadiusTheo) + (rowIndex - sRadiusTheo)*(rowIndex - sRadiusTheo)) > dRadius * dRadius)
        {
            pCalibratedData[index] = 0;
        }
    }
*/

    for(int iRow = 0; iRow < (2 * sRadiusTheo + 1); iRow++)
    {
#pragma omp parallel for num_threads(4)
        for(int iCol = 0; iCol < (2 * sRadiusTheo + 1); iCol++)
        {
            // if (sqrt((iCol - sRadiusTheo)*(iCol - sRadiusTheo) + (iRow - sRadiusTheo)*(iRow - sRadiusTheo)) > dRadius)
            if(((iCol - sRadiusTheo)*(iCol - sRadiusTheo) + (iRow - sRadiusTheo)*(iRow - sRadiusTheo)) > dRadius * dRadius)
            {
                pCalibratedData[lCount] = 0;
            }
            lCount++;
        }
    }

#else
    for(int iRow = 0; iRow < (2 * sRadiusTheo + 1); iRow++)
    {
        for(int iCol = 0; iCol < (2 * sRadiusTheo + 1); iCol++)
        {
            if (sqrt((iCol - sRadiusTheo)*(iCol - sRadiusTheo) + (iRow - sRadiusTheo)*(iRow - sRadiusTheo)) > dRadius)
            {
                pCalibratedData[lCount] = 0;
            }
            lCount++;
        }
    }
#endif
}

int PipelineCompute::ComputeWrongBands(
        int16* rawData,
        int16 threshold,
        const ImageSize* pSize)
{
    // return the number of pixels above threshold in the inactiva area
    int32 iDefects = 0;

    int maxPixel = 0;
    int minPixel = rawData[0];

    // TODO since the inactive area is fixed and set according to a defined image size
    // the image size must match

    if((pSize->height != _IMAGE_HEIGHT) ||
       (pSize->width  != _IMAGE_WIDTH))
    {
        // there is a mismatch between the image capture and the settings
        return 0;
    }

    for(int lineIndex = 0; lineIndex < _IMAGE_HEIGHT; lineIndex ++)
    {
        // pointer on the first pixel of the line
        int16* line = &rawData[lineIndex * pSize->width];

        for(int pixelIndex = 0; pixelIndex < _IMAGE_WIDTH; pixelIndex ++)
        {
            // top inactive part
            if((lineIndex  >= _TOP_VERTICAL_OFFSET)   && (lineIndex  < _TOP_VERTICAL_OFFSET + _TOP_HEIGHT) &&
               (pixelIndex >= _TOP_HORIZONTAL_OFFSET) && (pixelIndex < _TOP_HORIZONTAL_OFFSET + _TOP_WIDTH))
            {
                if(line[pixelIndex] > threshold)
                {
                    iDefects++;
                }

                maxPixel = MAX_VALUE(maxPixel, line[pixelIndex]);
                minPixel = MIN_VALUE(minPixel, line[pixelIndex]);
            }
            // left inactive
            else if((lineIndex  >= _LEFT_VERTICAL_OFFSET)   && (lineIndex  < _LEFT_VERTICAL_OFFSET + _LEFT_HEIGHT) &&
                    (pixelIndex >= _LEFT_HORIZONTAL_OFFSET) && (pixelIndex < _LEFT_HORIZONTAL_OFFSET + _LEFT_WIDTH))
            {
                if(line[pixelIndex] > threshold)
                {
                    iDefects++;
                }

                maxPixel = MAX_VALUE(maxPixel, line[pixelIndex]);
                minPixel = MIN_VALUE(minPixel, line[pixelIndex]);
            }
            // right inactive
            else if((lineIndex  >= _RIGHT_VERTICAL_OFFSET)   && (lineIndex  < _RIGHT_VERTICAL_OFFSET + _RIGHT_HEIGHT) &&
                    (pixelIndex >= _RIGHT_HORIZONTAL_OFFSET) && (pixelIndex < _RIGHT_HORIZONTAL_OFFSET + _RIGHT_WIDTH))
            {
                if(line[pixelIndex] > threshold)
                {
                    iDefects++;
                }

                maxPixel = MAX_VALUE(maxPixel, line[pixelIndex]);
                minPixel = MIN_VALUE(minPixel, line[pixelIndex]);
            }
        }
    }

    return iDefects;
}

typedef enum
{
    BiasCorner_TL,     // top left
    BiasCorner_TR,     // top right
    BiasCorner_BL,     // bottom left
    BiasCorner_BR,     // bottom right
    BiasCorner_Count   // number of corners
} BiasCorner_t;

// using this define may simplify the calculation but the calculation is not done this way in the genuine implementation
// #define NO_ARRAY

int PipelineCompute::ComputedBiasSubtraction(
        int16* rawData,
        int16* offsetData)
{
    // TODO please evaluate processing time
    int iDarkCurrentBiasValue = 0;

    double BiasMax[BiasCorner_Count] = {0};
    double BiasMin[BiasCorner_Count] = {0};

#ifndef NO_ARRAY
    MeasurementValue<double> BiasValue[BiasCorner_Count];
#else
    reMeasurementValue<double> BiasValue;
#endif

    if(offsetData)
    {
        for(int i=0; i < _BIAS_CORNER_AREA_WIDTH; i++)
        {
            for (int j=0; j < _BIAS_CORNER_AREA_HEIGHT; j++)
            {
#ifndef NO_ARRAY
                BiasValue[BiasCorner_TL].Push((double)rawData[TL_INDEX(j, i)] -
                                              (double)offsetData[TL_INDEX(j, i)]);
                BiasValue[BiasCorner_TR].Push((double)rawData[TR_INDEX(j, i)] -
                                              (double)offsetData[TR_INDEX(j, i)]);
                BiasValue[BiasCorner_BL].Push((double)rawData[BL_INDEX(j, i)] -
                                              (double)offsetData[BL_INDEX(j, i)]);
                BiasValue[BiasCorner_BR].Push((double)rawData[BR_INDEX(j, i)] -
                                              (double)offsetData[BR_INDEX(j, i)]);
#else
                BiasValue.Push((double)rawData[TL_INDEX(j, i)] -
                               (double)offsetData[TL_INDEX(j, i)]);
                BiasValue.Push((double)rawData[TR_INDEX(j, i)] -
                               (double)offsetData[TR_INDEX(j, i)]);
                BiasValue.Push((double)rawData[BL_INDEX(j, i)] -
                               (double)offsetData[BL_INDEX(j, i)]);
                BiasValue.Push((double)rawData[BR_INDEX(j, i)] -
                               (double)offsetData[BR_INDEX(j, i)]);
#endif
            }
        }
    }
    else
    {
        for(int i=0; i<_BIAS_CORNER_AREA_WIDTH; i++)
        {
            for(int j=0; j<_BIAS_CORNER_AREA_HEIGHT; j++)
            {
                if (i==0 && j==0)
                {
                    // first step initialise min and max variables
                    BiasMax[BiasCorner_TL] = rawData[TL_INDEX(j, i)];
                    BiasMax[BiasCorner_TR] = rawData[TR_INDEX(j, i)];
                    BiasMax[BiasCorner_BL] = rawData[BL_INDEX(j, i)];
                    BiasMax[BiasCorner_BR] = rawData[BR_INDEX(j, i)];

                    BiasMin[BiasCorner_TL] = rawData[TL_INDEX(j, i)];
                    BiasMin[BiasCorner_TR] = rawData[TR_INDEX(j, i)];
                    BiasMin[BiasCorner_BL] = rawData[BL_INDEX(j, i)];
                    BiasMin[BiasCorner_BR] = rawData[BR_INDEX(j, i)];
                }
                else
                {
                    // update min and max
                    BiasMax[BiasCorner_TL] = MAX_VALUE(BiasMax[BiasCorner_TL], rawData[TL_INDEX(j, i)]);
                    BiasMax[BiasCorner_TR] = MAX_VALUE(BiasMax[BiasCorner_TR], rawData[TR_INDEX(j, i)]);
                    BiasMax[BiasCorner_BL] = MAX_VALUE(BiasMax[BiasCorner_BL], rawData[BL_INDEX(j, i)]);
                    BiasMax[BiasCorner_BR] = MAX_VALUE(BiasMax[BiasCorner_BR], rawData[BR_INDEX(j, i)]);

                    BiasMin[BiasCorner_TL] = MIN_VALUE(BiasMin[BiasCorner_TL], rawData[TL_INDEX(j, i)]);
                    BiasMin[BiasCorner_TR] = MIN_VALUE(BiasMin[BiasCorner_TR], rawData[TR_INDEX(j, i)]);
                    BiasMin[BiasCorner_BL] = MIN_VALUE(BiasMin[BiasCorner_BL], rawData[BL_INDEX(j, i)]);
                    BiasMin[BiasCorner_BR] = MIN_VALUE(BiasMin[BiasCorner_BR], rawData[BR_INDEX(j, i)]);
                }

#ifndef NO_ARRAY
                BiasValue[BiasCorner_TL].Push((double)rawData[TL_INDEX(j, i)]);
                BiasValue[BiasCorner_TR].Push((double)rawData[TR_INDEX(j, i)]);
                BiasValue[BiasCorner_BL].Push((double)rawData[BL_INDEX(j, i)]);
                BiasValue[BiasCorner_BR].Push((double)rawData[BR_INDEX(j, i)]);
#else
                BiasValue.Push((double)rawData[TL_INDEX(j, i)]);
                BiasValue.Push((double)rawData[TR_INDEX(j, i)]);
                BiasValue.Push((double)rawData[BL_INDEX(j, i)]);
                BiasValue.Push((double)rawData[BR_INDEX(j, i)]);
#endif
            }
        }
    }

#ifndef NO_ARRAY
    for(int index = 0; index < BiasCorner_Count; index ++)
    {
        iDarkCurrentBiasValue += BiasValue[index].GetAverage();
    }

    iDarkCurrentBiasValue /= BiasCorner_Count;
#else
    iDarkCurrentBiasValue = BiasValue.GetAverage();
#endif

    return iDarkCurrentBiasValue;
}

void PipelineCompute::ApplyBias(
        int16* rawData,
        int rawDataSize,
        int iDarkCurrentBiasValue,
        MeasurementValue<float> &fullSensor)  // TODO this param can be removed
{

#ifdef ONE_LOOP_1
#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
    for(int index = 0; index < rawDataSize; index ++)
    {
        rawData[index] -= iDarkCurrentBiasValue;
    }

    fullSensor.Reset();

#ifdef OMP_PARAL
#pragma omp parallel for num_threads(4)
#endif
    for(int index = 0; index < rawDataSize; index ++)
    {
        fullSensor.Push(rawData[index]);
    }
#else
    fullSensor.Reset();

    for(int index = 0; index < rawDataSize; index ++)
    {
        rawData[index] -= iDarkCurrentBiasValue;

        fullSensor.Push(rawData[index]);
    }
#endif
}

void PipelineCompute::DarkOffsetCalculation(
        int16* rawData,
        const ImageSize& imageSize,
        Histogram* histogram,
        DarkOffset &darkOffset,
        int16& maxBinaryValue)
{
    HISTOGRAM->Reset();

    int16* line;

    darkOffset.fActive.Reset();
    darkOffset.fBias.Reset();

    for(int j = _ACTIVE_VERTICAL_OFFSET; j < imageSize.height; j ++)
    {
        line = &rawData[j * imageSize.width];

        // bias corners (test for TOP and BOTTOM)
        if((j < (_ACTIVE_VERTICAL_OFFSET + _BIAS_CORNER_AREA_HEIGHT)) ||
           (j >= (imageSize.height - _BIAS_CORNER_AREA_HEIGHT)))
        {
            // LEFT area
            for(int i = _ACTIVE_HORIZONTAL_OFFSET; i < _ACTIVE_HORIZONTAL_OFFSET + _BIAS_CORNER_AREA_WIDTH; i++)
            {
                if (line[i] > maxBinaryValue)
                {
                    maxBinaryValue = line[i];
                }

                darkOffset.fBias.Push((float)line[i]);

                HISTOGRAM->Add(line[i]);
            }

            // RIGHT area
            for(int i = imageSize.width - _ACTIVE_HORIZONTAL_OFFSET - _BIAS_CORNER_AREA_WIDTH;
                i < imageSize.width - _ACTIVE_HORIZONTAL_OFFSET; i ++)
            {
                if (line[i] > maxBinaryValue)
                {
                    maxBinaryValue = line[i];
                }

                darkOffset.fBias.Push((float)line[i]);

                HISTOGRAM->Add(line[i]);
            }
        }

        // Active central area
        for(int i = _ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET;
            i < (_ACTIVE_CENTRAL_AREA_WIDTH + _ACTIVE_CENTRAL_AREA_HORIZONTAL_OFFSET); i++)
        {
            if(line[i] > maxBinaryValue)
            {
                maxBinaryValue = line[i];
            }

            darkOffset.fActive.Push((float)line[i]);

            HISTOGRAM->Add(line[i]);
        }
    }
}

void PipelineCompute::SensorPrnuCorrection(
        int16* rawData,
        unsigned int rawDataSize,
        std::vector<char>* gainArr,
        float scaleFactor)
{
    // Implement New PRNU correction here.
    if((gainArr != NULL) &&
       (gainArr->size() >= rawDataSize * sizeof(int16)))
    {
        appendLogFile("Apply New PRNU Correction");

        int16* pGainArr = (int16*)&gainArr->at(0);

#pragma omp parallel for num_threads(4)
        for(int Index=0; Index < (int)rawDataSize; Index++)
        {
            // rawData[Index] = (int16)round(rawData[Index] * (1 + ((float)(pGainArr[Index]) * scaleFactor)));
            float tmp = (float)(pGainArr[Index]) * scaleFactor;
            rawData[Index] = (int16)round(rawData[Index] * (1 + tmp));
        }
    }
}

void PipelineCompute::_SetImageConfiguration(const ImageSize& imageSize)
{
    // check if the image size is not typical
    if((imageSize.width   != _IMAGE_WIDTH) ||
       (imageSize.height  != _IMAGE_HEIGHT) ||
       (imageSize.offsetX != 0) ||
       (imageSize.offsetY != 0))
    {
        // param->imageSize.offsetX
        // param->imageSize.offsetY
        m_ImageConfiguration.image_horizontal_offset      = 0;
        m_ImageConfiguration.image_vertical_offset        = 0;
        m_ImageConfiguration.image_width                  = imageSize.width;
        m_ImageConfiguration.image_height                 = imageSize.height;

        m_ImageConfiguration.active_horizontal_offset     = 0;
        m_ImageConfiguration.active_vertical_offset       = 0;
        m_ImageConfiguration.active_width                 = imageSize.width;
        m_ImageConfiguration.active_height                = imageSize.height;

        if(m_ImageConfigurationRef.bias_corner_area_width * 2 > m_ImageConfiguration.image_width)
        {
            m_ImageConfiguration.bias_corner_area_width = 0;
        }
        else
        {
            m_ImageConfiguration.bias_corner_area_width = m_ImageConfigurationRef.bias_corner_area_width;
        }

        if(m_ImageConfigurationRef.bias_corner_area_height * 2 > m_ImageConfiguration.bias_corner_area_height)
        {
            m_ImageConfiguration.bias_corner_area_height = 0;
        }
        else
        {
            m_ImageConfiguration.bias_corner_area_height = m_ImageConfigurationRef.bias_corner_area_height;
        }

        m_ImageConfiguration.active_central_area_horizontal_offset = 0;
        m_ImageConfiguration.active_central_area_vertical_offset   = 0;

        m_ImageConfiguration.UpdateSettings();
    }
    else
    {
        // typical value
        m_ImageConfiguration = m_ImageConfigurationRef;
    }

    PipelineDefectCorrector::SetImageConfiguration(m_ImageConfiguration);
}

float PipelineCompute::_GetRadius(float fMaxAngle, const LinearizationCoef* pLinearizationFactor)
{
    // calculate the radius of the sensor to be used 
    // (depending on reduction factors calculated during calibration)

    float  fReductionFactor, fB0, fB2, fB4, fB6, fB8;
    float sensorRadius;

    fReductionFactor = fMaxAngle / 90;

    fB0 = CORRECT_FACTOR(1, fReductionFactor);
    fB2 = CORRECT_FACTOR(3, fReductionFactor);
    fB4 = CORRECT_FACTOR(5, fReductionFactor);
    fB6 = CORRECT_FACTOR(7, fReductionFactor);
    fB8 = CORRECT_FACTOR(9, fReductionFactor);

    sensorRadius = fB0 + fB2 + fB4 + fB6 + fB8;

    return sensorRadius;
}

