#ifndef CONOSCOPEAPPPROCESS_H
#define CONOSCOPEAPPPROCESS_H

// configuration
#define SET_TEMPERATURE

#include "classcommon.h"
#include "camera.h"

#include "Conoscope.h"

#include "conoscopeTypes.h"
#include "PipelineLib.h"

#include "TempMonitoring.h"

// #include "Cdevices.h"

#include <QApplication>
#include <QDateTime>

#include "CfgHelper.h"

class ConoscopeAppProcess : public ClassCommon
{
    Q_OBJECT

public:
    static void SetConoscope(Conoscope* conoscope);

    static void Delete();

    static QString CmdGetPipelineVersion();

    static ClassCommon::Error CmdOpen();
    static ClassCommon::Error CmdSetup(SetupConfig_t &config);
    static ClassCommon::Error CmdSetupStatus(SetupStatus_t &status);
    static ClassCommon::Error CmdMeasure(MeasureConfigWithCropFactor_t &config);
    static ClassCommon::Error CmdExportRaw();
    static ClassCommon::Error CmdExportRaw(std::vector<uint16_t> &buffer);
    static ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config);
    static ClassCommon::Error CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &buffer, bool bSaveImage = false);
    static ClassCommon::Error CmdClose();
    static ClassCommon::Error CmdReset();

    static ClassCommon::Error CmdCfgFileWrite();
    static ClassCommon::Error CmdCfgFileRead();
    static ClassCommon::Error CmdCfgFileStatus(CfgFileStatus_t &status);

    static ClassCommon::Error SetConfig(CaptureSequenceConfig_t& config);

    static ClassCommon::Error SetBehaviorConfig(ConoscopeBehavior_t& config);

    static void GetSomeInfo(SomeInfo_t &info);

    static ConoscopeAppProcess* GetInstance();

signals:
    void OnLog(QString message);

private:
    /*
     * image info
     * information about the capture image
     */
    typedef struct
    {
        QString   timeStampString;

        float     temperature;
    } ImageInfo_t;

    class CaptureInfo_t
    {
    public:
        // capture
        QString   timeStampDate;
        QString   timeStampTime;
        QString   timeStampString;

        float     temperature;

        // settings
        int imageHeight;
        int imageWidth;
        int exposureUs;

        float temperatureCpu;
        float temperatureMainBoard;
        float temperatureSensor;

        QString cameraBoardSerialNumber;

        void Clone(CaptureInfo_t& input)
        {
            timeStampDate           = input.timeStampDate;
            timeStampTime           = input.timeStampTime;
            timeStampString         = input.timeStampString;
            temperature             = input.temperature;
            imageHeight             = input.imageHeight;
            imageWidth              = input.imageWidth;
            exposureUs              = input.exposureUs;
            temperatureCpu          = input.temperatureCpu;
            temperatureMainBoard    = input.temperatureMainBoard;
            temperatureSensor       = input.temperatureSensor;
            cameraBoardSerialNumber = input.cameraBoardSerialNumber;
        }
    } ;

    explicit ConoscopeAppProcess(QObject *parent = nullptr);
    ~ConoscopeAppProcess();

    QString _CmdGetPipelineVersion();

    ClassCommon::Error _CmdOpen();
    ClassCommon::Error _CmdSetup(SetupConfig_t &config);
    ClassCommon::Error _CmdSetupStatus(SetupStatus_t &status);
    ClassCommon::Error _CmdMeasure(MeasureConfigWithCropFactor_t &config);
    ClassCommon::Error _CmdExportRaw();
    ClassCommon::Error _CmdExportRaw(std::vector<uint16_t> &bufferV);

    ClassCommon::Error _CmdExportProcessed(ProcessingConfig_t &config);
    ClassCommon::Error _CmdExportProcessed(ProcessingConfig_t &config, std::vector<int16_t> &bufferV, bool bSaveImage);

    ClassCommon::Error _CmdClose();
    ClassCommon::Error _CmdReset();

    ClassCommon::Error _CmdCfgFileWrite();
    ClassCommon::Error _CmdCfgFileRead();
    ClassCommon::Error _CmdCfgFileStatus(CfgFileStatus_t& status);

    ClassCommon::Error _SetConfig(CaptureSequenceConfig_t& config);
    ClassCommon::Error _SetConfig(ConoscopeBehavior_t& config);

    void _GetSomeInfo(SomeInfo_t &info);

    void _Log(QString message);

    SetupConfig_t _setupConfig;

protected:
    static ConoscopeAppProcess* mInstance;
    static ConoscopeAppProcess* _GetInstance();

public:
    static Conoscope::CmdOpenOutput_t            cmdOpenOutput;
    static Conoscope::CmdExportRawOutput_t       cmdExportRawOutput;
    static Conoscope::CmdExportProcessedOutput_t cmdExportProcessedOutput;
    static Conoscope::CmdExportAdditionalInfo_t  cmdExportAdditionalInfo;
    static CfgOutput                             readCfgCameraPipelineOutput;

private:
    static Conoscope* mConoscope;
};

#endif // CONOSCOPEAPPWORKER_H
