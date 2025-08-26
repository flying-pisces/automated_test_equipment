#ifndef PIPELINECOMPUTE_H
#define PIPELINECOMPUTE_H

#include "vcruntime.h" // for NULL

#include "defines.h"

#include "PipelineTypes.h"
#include "PipelineComputeTypes.h"
#include "imageConfiguration.h"

#include "logger.h"

class PipelineCompute
{
private:
    static PipelineCompute* m_instance;
    static PipelineCompute* getInstance();
    PipelineCompute();
    ~PipelineCompute();

    ImageConfiguration m_ImageConfigurationRef;
    ImageConfiguration m_ImageConfiguration;

    Logger* m_logger;

    void appendLogFile(QString text){
        m_logger->Append(text);
    }

public:
    static void SetLogger(Logger *logger);

    static void SetImageConfiguration(ImageConfiguration &imageConfiguration);

    static int GetCalibratedDataSize(short calibratedDataRadius);

    static Error_t ComputeRawData(
            int16*                              rawData,
            const Pipeline_RawDataParam *param,
            Histogram*                          pHistogram,
            Pipeline_ResultRawDataParam &resultParam);

    static Error_t ComputeKLibData(const Pipeline_KLibDataParam &param,
            Pipeline_CalibrationParam *calibration,
            const Pipeline_DataIn *rawData,
            Pipeline_DataOut *calibDataOut,
            int16*                              calibratedData);

private:
    Error_t _ComputeRawData(int16* rawData,
            const Pipeline_RawDataParam *param,
            Histogram *pHistogram,
            Pipeline_ResultRawDataParam &resultParam);

    int _GetCalibratedDataSize(short calibratedDataRadius);

    Error_t _ComputeKLibData(const Pipeline_KLibDataParam &param,
            const Pipeline_CalibrationParam *calibration,
            const Pipeline_DataIn *rawData,
            Pipeline_DataOut *calibDataOut,
            int16* calibratedData);

private:
    static int MXLinearizeAndFlatField(int16*  pintSource,
            long    lSourceWidth,
            long    lSize,
            float   fMaxAngle,
            Point center,
            const LinearizationCoef *plinearizationFactor,
            int16*  pintFlatField,
            long    lSize2,
            long    lScaleFactor,
            int16*  pintTarget,
            bool*   flatFieldPrecomputed,
            bool    applyFlatField);

    static Error_t PrecomputeLinearizationTables(long    lTargetWidth,
            long    lTargetHeight,
            long    lTargetRadius,
            long    lTargetAxis,
            const LinearizationCoef *pLinearizationFactor,
            float   fMaxAngle,
            Point center,
            precomputedData_t** ppPrecomputedData);

    static Error_t PrecomputeFlatFieldInverse(
            int16*  pint16Denominator,
            long    lScaleFactor,
            long    lSize,
            bool*   flatFieldPrecomputed,
            bool    applyFlatField,
            precomputedData_t* pPrecomputedData);

    static Error_t MXLinearize(int16* pintSource,
            long    lSourceWidth,
            long    lSize,
            float   fMaxAngle,
            Point center,
            const LinearizationCoef *pLinearizationFactor,
            long    lExcluded,
            int16 * pintTarget);

    static void RestrictToViewingAngle(
            float  maximumIncidentAngle,
            short  calibratedDataRadius,
            int16* calibratedData);

#ifdef REMOVED
    static int16* GetFlatFieldbuffer(const std::vector<Base64Binary> &flatField);
#endif

    int ComputeWrongBands(int16* rawData,
            int16 threshold,
            const ImageSize *pSize);

    int ComputedBiasSubtraction(
            int16* rawData,
            int16* offsetData);

    void ApplyBias(int16* rawData,
            int rawDataSize,
            int iDarkCurrentBiasValue,
            MeasurementValue<float> &fullSensor);

    void DarkOffsetCalculation(int16* rawData,
            const ImageSize &imageSize,
            Histogram* histogram,
            DarkOffset &darkOffset,
            int16& maxBinaryValue);

    void SensorPrnuCorrection(
            int16* rawData,
            unsigned int rawDataSize,
            std::vector<char> *gainArr,
            float scaleFactor);

    void _SetImageConfiguration(const ImageSize &imageSize);

    float _GetRadius(float fMaxAngle, const LinearizationCoef *pLinearizationFactor);
};

#endif PIPELINECOMPUTE_H
