#ifndef PIPELINELIB_H
#define PIPELINELIB_H

#include "PipelineTypes.h"

#define CMD(a) f_##a Lib##a

#include "classcommon.h"
#include "Types.h"

class PipelineLib : public ClassCommon
{
    Q_OBJECT
public:
    PipelineLib(QObject *parent = nullptr);

    ~PipelineLib();

public:
    QString GetVersion();

    Error CmdComputeRawData(
            int16 *inputData,
            Pipeline_RawDataParam *param,
            Pipeline_ResultRawDataParam &resultParam);

    Error CmdComputeKLibData(int16 *inputData,
            Pipeline_KLibDataParam &param,
            Pipeline_CalibrationParam *calibration,
            int16 *klibData);

    QString GetErrorDescription()
    {
        return mErrorDescription;
    }

    typedef const char* (*f_CmdGetVersion)();
    CMD(CmdGetVersion);

    typedef char* (*f_CmdComputeRawData)(
            int16* inputData,
            Pipeline_RawDataParam* param,
            Pipeline_ResultRawDataParam &resultParam);
    CMD(CmdComputeRawData);

    typedef char* (*f_CmdComputeKLibData)(
            int16* inputData,
            Pipeline_KLibDataParam &param,
            Pipeline_CalibrationParam *calibration,
            int16* klibData);
    CMD(CmdComputeKLibData);

private:
    void _Load();

    QString mErrorDescription;
};

#endif // PIPELINELIB_H
