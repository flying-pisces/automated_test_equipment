#include "CfgHelper.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>

#include "ConoscopeResource.h"

#define LOG_HEADER "[cfgHelper]"
#define LogInFile(text) RESOURCE->AppendLog(QString("%1 | %2").arg(LOG_HEADER, -20).arg(text))

#define LOG_APP_HEADER "[cfgHelper]"
#define LogInApp(text) RESOURCE->Log(QString("%1 %2").arg(LOG_APP_HEADER).arg(text));

#define CONVERT_TO_QSTRING(a) QString::fromUtf8(a.c_str())

CfgHelper* CfgHelper::mInstance = nullptr;

#define INSTANCE(instance) CfgHelper* instance = CfgHelper::getInstance()

CfgHelper* CfgHelper::getInstance()
{
    if(CfgHelper::mInstance == nullptr)
    {
        CfgHelper::mInstance = new CfgHelper();
    }

    return CfgHelper::mInstance;
}

void CfgHelper::_Log(QString message)
{
    if(!message.isEmpty())
    {
        emit OnLog(message);
    }
}

CfgHelper::CfgHelper(QObject *parent) : QObject(parent)
{
    mConfigContentCameraSn = "";
    mConfigContentFlatFieldIrisIndex = -1;
    mConfigContentFlatFieldFilterIndex = -1;

    mOpticalColumnConfigValid = false;
}

CfgHelper::~CfgHelper()
{

}

bool CfgHelper::_ReadCfgFile(QString fileName, ConfigContent_t& configContent)
{
    bool res = true;

    _Log("Read config file");

    QFile file(fileName);
    // open the file
    if(!file.open(QIODevice::ReadOnly))
    {
        _Log("Can not read configuration file");
        LogInFile("Can not read configuration file");
        res = false;
    }

    QDomDocument cfgConfig("STORMHOLDCONFIGFILE");

    // read the content of the file
    if(res == true)
    {
        if(!cfgConfig.setContent(&file))
        {
            LogInFile("  CameraCfg: ERROR can not set content file");
            _Log("Can not set contentfile");
            res = false;
        }

        file.close();
    }

    if(res == true)
    {
        QDomElement configElement = cfgConfig.documentElement();
        QDomElement Content= configElement.firstChildElement("Content");

        // calibrationStep
        QDomElement inCalibrationStep = Content.firstChildElement("CalibrationStep");
        _ReadCalibrationStep(inCalibrationStep, configContent.calibrationStep);

        // read equipment infos
        QDomElement inEquipment = Content.firstChildElement("Equipment");
        _ReadEquipmentSection(inEquipment, configContent.equipement);

        // Read OpticalColumnCalibration
        QDomElement inOpticalColumnCalibration = Content.firstChildElement("OpticalColumnCalibration");
        _ReadOpticalColumnCalibrationSection(inOpticalColumnCalibration, configContent.opticalColumnCalibration);

        // Read CameraPipeLine
        QDomElement inCameraPipeline = Content.firstChildElement("CameraPipeline");
        _ReadCameraPipeLineSection(inCameraPipeline, configContent.cameraPipeline);

        // Read CurrentGain
        QDomElement inCurrentGain = Content.firstChildElement("CurrentGain");
        _ReadCurrentGain(inCurrentGain, configContent.currentGain);
    }

    // load prnu data
    if(res == true)
    {
        // indicate that PRNU can be applied
        configContent.cameraPipeline.sensorPrnu.correctionEnabled = true;

        QString prnuFileName = CAMERA_PRNU_PATH(fileName);

        QFile prnuFile(prnuFileName);

        if(prnuFile.open(QFile::ReadOnly))
        {
            QByteArray prnuData;

            prnuData = prnuFile.readAll();
            prnuFile.close();

            int prnuDataLength = prnuData.length();

            configContent.cameraPipeline.sensorPrnu.data.resize(prnuDataLength);

            memcpy(configContent.cameraPipeline.sensorPrnu.data.data(),
                   prnuData.data(),
                   prnuDataLength);
        }
        else
        {
            LogInFile(QString("  CameraCfg: ERROR Can not open").arg(prnuFileName));
            LogInApp(QString("  CameraCfg: ERROR Can not open").arg(prnuFileName));
            res = false;
        }
    }

    return res;
}

bool CfgHelper::_ReadCalibrationStep(QDomElement& inCalibrationStep,
                                    CalibrationStep_t& calibrationStep)
{
    if(!inCalibrationStep.isNull() && inCalibrationStep.isElement())
    {
        calibrationStep.value = inCalibrationStep.toElement().text().toInt();
    }

    return true;
}

bool CfgHelper::_ReadSummarySection(QDomElement& inSummary, Summary_t& summary)
{
    if(!inSummary.isNull() && inSummary.isElement())
    {
        summary.date    = inSummary.firstChildElement("Date").text().toStdString();
        summary.time    = inSummary.firstChildElement("Time").text().toStdString();
        summary.comment = inSummary.firstChildElement("Comment").text().toStdString();
    }
    else
    {
        summary.date    = "Not available";
        summary.time    = "Not available";
        summary.comment = "Not available";
    }

    return true;
}

bool CfgHelper::_ReadEquipmentSection(QDomElement& inEquipment,
                                     Equipement_t& equipement)
{
    if(!inEquipment.isNull() && inEquipment.isElement())
    {
        equipement.type         = inEquipment.firstChildElement("Type").text().toStdString();
        equipement.description  = inEquipment.firstChildElement("Description").text().toStdString();
        equipement.location     = inEquipment.firstChildElement("Location").text().toStdString();
        equipement.revision     = inEquipment.firstChildElement("Revision").text().toStdString();
        equipement.serialNumber = inEquipment.firstChildElement("SerialNumber").text().toStdString();
    }

    return true;
}

bool CfgHelper::_ReadOpticalColumnCalibrationSection(QDomElement& inOpticalColumnCalibration,
                                                    OpticalColumnCalibration_t & opticalColumnCalibration )
{
    if(!inOpticalColumnCalibration.isNull())
    {
        // get SensorTemperatureDependance
        QDomElement inSensorTemperatureDependance = inOpticalColumnCalibration.firstChildElement("SensorTemperatureDependance");

        if(!inSensorTemperatureDependance.isNull() && inSensorTemperatureDependance.isElement())
        {
        opticalColumnCalibration.sensorTemperatureDependency.slope               = inSensorTemperatureDependance.firstChildElement("Slope").text().toFloat();
        // opticalColumnCalibration.sensorTemperatureDependency.table
        opticalColumnCalibration.sensorTemperatureDependency.correctionEnable    = (bool)inSensorTemperatureDependance.firstChildElement("CorrectionEnabled").text().toInt();
        opticalColumnCalibration.sensorTemperatureDependency.timeStamp           = inSensorTemperatureDependance.firstChildElement("TimeStamp").text().toLong();
        opticalColumnCalibration.sensorTemperatureDependency.sensorSerialNumber  = inSensorTemperatureDependance.firstChildElement("SensorSerialNumber").text().toStdString();
        opticalColumnCalibration.sensorTemperatureDependency.stationSerialNumber = inSensorTemperatureDependance.firstChildElement("StationSerialNumber").text().toStdString();
        opticalColumnCalibration.sensorTemperatureDependency.calibrationDone     = (bool)inSensorTemperatureDependance.firstChildElement("CalibrationDone").text().toInt();
        }

        // get CaptureArea
        QDomElement inCaptureArea = inOpticalColumnCalibration.firstChildElement("CaptureArea");
        if(!inCaptureArea.isNull() && inCaptureArea.isElement())
        {
            QDomElement OpticalAxis= inCaptureArea.firstChildElement("OpticalAxis");
            if(!OpticalAxis.isNull() && OpticalAxis.isElement())
            {
                opticalColumnCalibration.captureArea.opticalAxis.X = OpticalAxis.firstChildElement("X").text().toShort();
                opticalColumnCalibration.captureArea.opticalAxis.Y = OpticalAxis.firstChildElement("Y").text().toShort();
            }

            opticalColumnCalibration.captureArea.measurementRadius     = inCaptureArea.firstChildElement("MeasurementRadius").text().toShort();
            opticalColumnCalibration.captureArea.timeStamp             = inCaptureArea.firstChildElement("TimeStamp").text().toLong();
            opticalColumnCalibration.captureArea.equipmentSerialNumber = inCaptureArea.firstChildElement("EquipmentSerialNumber").text().toStdString();
            opticalColumnCalibration.captureArea.stationSerialNumber   = inCaptureArea.firstChildElement("StationSerialNumber").text().toStdString();
            opticalColumnCalibration.captureArea.calibrationDone       = (bool)inCaptureArea.firstChildElement("CalibrationDone").text().toInt();
        }

        opticalColumnCalibration.maximumIncidentAngle   = inOpticalColumnCalibration.firstChildElement("MaximumIncidentAngle").text().toFloat();
        opticalColumnCalibration.calibratedDataRadius = inOpticalColumnCalibration.firstChildElement("CalibratedDataRadius").text().toShort();

        // get LinearizationCoefficients
        QDomElement inLinearizationCoefficients = inOpticalColumnCalibration.firstChildElement("LinearizationCoefficients");
        if(!inLinearizationCoefficients.isNull() && inLinearizationCoefficients.isElement())
        {
           opticalColumnCalibration.linearizationCoefficients.A1 = inLinearizationCoefficients.firstChildElement("A1").text().toFloat();
           opticalColumnCalibration.linearizationCoefficients.A3 = inLinearizationCoefficients.firstChildElement("A3").text().toFloat();
           opticalColumnCalibration.linearizationCoefficients.A5 = inLinearizationCoefficients.firstChildElement("A5").text().toFloat();
           opticalColumnCalibration.linearizationCoefficients.A7 = inLinearizationCoefficients.firstChildElement("A7").text().toFloat();
           opticalColumnCalibration.linearizationCoefficients.A9 = inLinearizationCoefficients.firstChildElement("A9").text().toFloat();
           opticalColumnCalibration.linearizationCoefficients.timeStamp = inLinearizationCoefficients.firstChildElement("TimeStamp").text().toLong();
           opticalColumnCalibration.linearizationCoefficients.equipmentSerialNumber = inLinearizationCoefficients.firstChildElement("EquipmentSerialNumber").text().toStdString();
           opticalColumnCalibration.linearizationCoefficients.stationSerialNumber = inLinearizationCoefficients.firstChildElement("StationSerialNumber").text().toStdString();
           opticalColumnCalibration.linearizationCoefficients.calibrationDone = (bool)inLinearizationCoefficients.firstChildElement("CalibrationDone").text().toInt();
        }

        // get flat field
        QDomElement inFlatField = inOpticalColumnCalibration.firstChildElement("FlatField");
        if(!inFlatField.isNull() && inFlatField.isElement())
        {
            opticalColumnCalibration.flatField.isCalibrated = inFlatField.firstChildElement("IsCalibrated").text().toInt();
            opticalColumnCalibration.flatField.conversionFactor = inFlatField.firstChildElement("ConversionFactor").text().toFloat();
            opticalColumnCalibration.flatField.maximumIncidentAngle = inFlatField.firstChildElement("MaximumIncidentAngle").text().toFloat();
            opticalColumnCalibration.flatField.radius = inFlatField.firstChildElement("Radius").text().toShort();
            opticalColumnCalibration.flatField.maxBinaryValue = inFlatField.firstChildElement("MaxBinaryValue").text().toShort();
            opticalColumnCalibration.flatField.saturationOccurs = inFlatField.firstChildElement("SaturationOccurs").text().toInt();
            opticalColumnCalibration.flatField.timeStamp = inFlatField.firstChildElement("TimeStamp").text().toLong();

            // calibration data
            int CalibratedDataMatrixSize = opticalColumnCalibration.calibratedDataRadius * 2 + 1;
            _Log( QString("CalibratedDataMatrixSize %1").arg(CalibratedDataMatrixSize));

            QString data64 = inFlatField.firstChildElement("Data").text();
            QByteArray text = QByteArray::fromBase64(data64.toLocal8Bit()); //QByteArray::fromBase64(data64.toLocal8Bit());

            long flatfieldSize = CalibratedDataMatrixSize * CalibratedDataMatrixSize;

            if((text.size() > flatfieldSize) && (CalibratedDataMatrixSize > 0))
            {
                _Log("FlatField buffer read");

                opticalColumnCalibration.flatField.data.resize(flatfieldSize * sizeof(int16));

                std::memcpy(opticalColumnCalibration.flatField.data.data(),
                            text.data(),
                            flatfieldSize * sizeof(int16));
            }
            else
            {
                // correction desactivated
                _Log("FlatField buffer not present");
            }

            // sensor temperature
            QDomElement temperatureNode= inFlatField.firstChildElement("SensorTemperature");
            _ReadSensorTemperature(temperatureNode, opticalColumnCalibration.flatField.sensorTemperature);

            opticalColumnCalibration.flatField.equipmentSerialNumber = inFlatField.firstChildElement("EquipmentSerialNumber").text().toStdString();
            opticalColumnCalibration.flatField.stationSerialNumber   = inFlatField.firstChildElement("StationSerialNumber").text().toStdString();
            opticalColumnCalibration.flatField.calibrationDone       = (bool)inFlatField.firstChildElement("CalibrationDone").text().toInt();
        }

        // get ConversionFactor
        QDomElement inConversionFactor = inOpticalColumnCalibration.firstChildElement("ConversionFactor");
        if(!inConversionFactor.isNull() && inConversionFactor.isElement())
        {
            opticalColumnCalibration.conversionFactor.value = inConversionFactor.firstChildElement("Value").text().toFloat();

            QDomElement temperatureNode= inConversionFactor.firstChildElement("SensorTemperature");
            _ReadSensorTemperature(temperatureNode, opticalColumnCalibration.conversionFactor.sensorTemperature);

            opticalColumnCalibration.conversionFactor.timeStamp = inConversionFactor.firstChildElement("TimeStamp").text().toLong();
            opticalColumnCalibration.conversionFactor.equipmentSerialNumber = inConversionFactor.firstChildElement("EquipmentSerialNumber").text().toStdString();
            opticalColumnCalibration.conversionFactor.stationSerialNumber = inConversionFactor.firstChildElement("StationSerialNumber").text().toStdString();
            opticalColumnCalibration.conversionFactor.calibrationDone = (bool)inConversionFactor.firstChildElement("CalibrationDone").text().toInt();
        }
    }

    return true;
}

bool CfgHelper::_ReadCameraPipeLineSection(QDomElement& inCameraPipeline, CameraPipeline_t& cameraPipeline)
{
    if(!inCameraPipeline.isNull())
    {
        /* biasMode */
        QDomElement inBiasMode = inCameraPipeline.firstChildElement("BiasMode");
        if(!inBiasMode.isNull() && inBiasMode.isElement())
        {
            cameraPipeline.biasMode.mode = (BiasMode_t) inBiasMode.firstChildElement("Mode").text().toInt();
            cameraPipeline.biasMode.compensationEnabled = (bool)(inBiasMode.firstChildElement("CompensationEnabled").text().toInt() > 0);
        }

        /* sensorSaturation */
        QDomElement inSensorSaturation = inCameraPipeline.firstChildElement("SensorSaturation");
        if(!inSensorSaturation.isNull() && inSensorSaturation.isElement())
        {
            cameraPipeline.sensorSaturation.value = inSensorSaturation.firstChildElement("Value").text().toShort();
            // getTemperature
            QDomElement temperatureNode= inSensorSaturation.firstChildElement("SensorTemperature");
            _ReadSensorTemperature(temperatureNode, cameraPipeline.sensorSaturation.sensorTemperature);

            cameraPipeline.sensorSaturation.timeStamp = inSensorSaturation.firstChildElement("TimeStamp").text().toLong();
            cameraPipeline.sensorSaturation.sensorSerialNumber = inSensorSaturation.firstChildElement("SensorSerialNumber").text().toStdString();
            cameraPipeline.sensorSaturation.stationSerialNumber = inSensorSaturation.firstChildElement("StationSerialNumber").text().toStdString();
            cameraPipeline.sensorSaturation.calibrationDone = (bool)inSensorSaturation.firstChildElement("CalibrationDone").text().toInt();
        }

        /* sensorDefects */
        QDomElement inSensorDefects = inCameraPipeline.firstChildElement("SensorDefects");
        if(!inSensorDefects.isNull() && inSensorDefects.isElement())
        {
            // retrieve all pixels
            QDomNode inPixels = inSensorDefects.firstChildElement("Pixels");
            if(!inPixels.isNull())
            {
                QDomNode pixel = inPixels.firstChild();
                while(!pixel.isNull())
                {
                    if(pixel.isElement())
                    {
                        Defect aDefect;
                        aDefect.coord.x = pixel.toElement().attribute("X").toShort();
                        aDefect.coord.y = pixel.toElement().attribute("Y").toShort();
                        aDefect.type = (DefectType_t)pixel.toElement().attribute("Type").toInt();

                        cameraPipeline.sensorDefects.pixels.push_back(aDefect);
                    }
                    pixel = pixel.nextSibling();
                }
            }

            // ROWS and Columns not used
            cameraPipeline.sensorDefects.correctionEnabled = inSensorDefects.firstChildElement("CorrectionEnabled").text().toInt();

            // sensor temperature
            QDomElement temperatureNode= inSensorDefects.firstChildElement("SensorTemperature");
            _ReadSensorTemperature(temperatureNode, cameraPipeline.sensorDefects.sensorTemperature);

            cameraPipeline.sensorDefects.timeStamp = inSensorDefects.firstChildElement("TimeStamp").text().toLong();
            cameraPipeline.sensorDefects.sensorSerialNumber = inSensorDefects.firstChildElement("SensorSerialNumber").text().toStdString();
            cameraPipeline.sensorDefects.stationSerialNumber = inSensorDefects.firstChildElement("StationSerialNumber").text().toStdString();
            cameraPipeline.sensorDefects.calibrationDone = (bool)inSensorDefects.firstChildElement("CalibrationDone").text().toInt();
        }

        /* sensorPrnu */
        QDomElement inSensorPRNU = inCameraPipeline.firstChildElement("SensorPRNU");
        if(!inSensorPRNU.isNull() && inSensorPRNU.isElement())
        {
            QDomElement CaptureSize = inSensorPRNU.firstChildElement("CaptureSize");

            if(!CaptureSize.isNull())
            {
                cameraPipeline.sensorPrnu.captureSize.width  = CaptureSize.firstChildElement("Width").text().toShort();
                cameraPipeline.sensorPrnu.captureSize.height = CaptureSize.firstChildElement("Height").text().toShort();
            }

            cameraPipeline.sensorPrnu.scaleFactor = inSensorPRNU.firstChildElement("ScaleFactor").text().toFloat();

            // read data PRNU
            QByteArray sensorPRNUData;
            sensorPRNUData.clear();
            sensorPRNUData = QByteArray::fromBase64(inSensorPRNU.firstChildElement("Data").text().toLocal8Bit());
            _Log(QString("Read from CFG mPRNUDataV2 size: %1").arg(sensorPRNUData.size()));

            const int prnuNbPixels = cameraPipeline.sensorPrnu.captureSize.width * cameraPipeline.sensorPrnu.captureSize.height;
            const int size = prnuNbPixels * 2;

            if(size <= sensorPRNUData.size())
            {
                cameraPipeline.sensorPrnu.data.resize(size);

                std::memcpy(cameraPipeline.sensorPrnu.data.data(),
                            sensorPRNUData.data(),
                            size);

                cameraPipeline.sensorPrnu.correctionEnabled = true;
            }
            else
            {
                cameraPipeline.sensorPrnu.correctionEnabled = false;
            }

            // sensor temperature
            QDomElement temperatureNode= inSensorPRNU.firstChildElement("SensorTemperature");
            _ReadSensorTemperature(temperatureNode, cameraPipeline.sensorPrnu.sensorTemperature);

            cameraPipeline.sensorPrnu.timeStamp = inSensorPRNU.firstChildElement("TimeStamp").text().toLong();
            cameraPipeline.sensorPrnu.sensorSerialNumber = inSensorPRNU.firstChildElement("SensorSerialNumber").text().toStdString();
            cameraPipeline.sensorPrnu.stationSerialNumber = inSensorPRNU.firstChildElement("StationSerialNumber").text().toStdString();
            cameraPipeline.sensorPrnu.calibrationDone = (bool)inSensorPRNU.firstChildElement("CalibrationDone").text().toInt();
        }
    }

    return true;
}

bool CfgHelper::_ReadCurrentGain(QDomElement &inCurrentGain, CurrentGain_t& currentGain)
{
    if(!inCurrentGain.isNull() && inCurrentGain.isElement())
    {
        currentGain.value = inCurrentGain.toElement().text().toFloat();
    }

    return true;
}

bool CfgHelper::_CreateCfgFile(QString airshipFileName, QString fileName, ConfigContent_t& configContent)
{
    bool res = true;
    QDomDocument cfgConfig("STORMHOLDCONFIGFILE");

    // read airship file
    res = QFileInfo::exists(airshipFileName);
    if(res == true)
    {
        // read the content of the file
        QFile airshipFile(airshipFileName);

        if(!cfgConfig.setContent(&airshipFile))
        {
            LogInFile("  CameraCfg: ERROR can not set content file");
            _Log("Can not set contentfile");
            res = false;
        }

        airshipFile.close();
    }

    // parse airship file
    if(res == true)
    {
        QDomElement configElement = cfgConfig.documentElement();
        QDomElement Content= configElement.firstChildElement("Content");

        // calibrationStep
        QDomElement inCalibrationStep = Content.firstChildElement("CalibrationStep");
        _ReadCalibrationStep(inCalibrationStep, configContent.calibrationStep);

        // read equipment infos
        QDomElement inEquipment = Content.firstChildElement("Equipment");
        _ReadEquipmentSection(inEquipment, configContent.equipement);

        // Read OpticalColumnCalibration
        QDomElement inOpticalColumnCalibration = Content.firstChildElement("OpticalColumnCalibration");
        _ReadOpticalColumnCalibrationSection(inOpticalColumnCalibration, configContent.opticalColumnCalibration);

        // Read CameraPipeLine
        QDomElement inCameraPipeline = Content.firstChildElement("CameraPipeline");
        _ReadCameraPipeLineSection(inCameraPipeline, configContent.cameraPipeline);

        // Read CurrentGain
        QDomElement inCurrentGain = Content.firstChildElement("CurrentGain");
        _ReadCurrentGain(inCurrentGain, configContent.currentGain);

        // create the document
        QDomDocument doc = QDomDocument("xml");
        QDomElement root = doc.createElement("StormholdConfigFile");
        doc.appendChild(root);

        QDomElement contentElement = doc.createElement("Content");
        root.appendChild(contentElement);

        // calibrationStep
        contentElement.appendChild(inCalibrationStep);

        // equipment infos
        contentElement.appendChild(inEquipment);

        // OpticalColumnCalibration
        contentElement.appendChild(inOpticalColumnCalibration);

        // CameraPipeLine
        // remove PRNU data
        QDomElement inSensorPRNU = inCameraPipeline.firstChildElement("SensorPRNU");
        QDomElement SensorPrnuData = inSensorPRNU.firstChildElement("Data");
        inSensorPRNU.removeChild(SensorPrnuData);

        contentElement.appendChild(inCameraPipeline);

        // CurrentGain
        contentElement.appendChild(inCurrentGain);

        // save the new cfg file
        LogInApp(QString("Write Cfg file %1").arg(fileName));
        QFile file(fileName);

        // open the file
        if(file.open(QIODevice::WriteOnly))
        {
            QTextStream outputStream(&file);
            outputStream << doc.toString();;
            file.close();
        }
        else
        {
            LogInFile(QString("  CameraCfg: ERROR Can not open").arg(fileName));
            LogInApp(QString("  CameraCfg: ERROR Can not open").arg(fileName));
            res = false;
        }

        // save PRNU in a separate file
        if(res == true)
        {
            QString prnuFileName = CAMERA_PRNU_PATH(fileName);

            QFile prnuFile(prnuFileName);

            if(prnuFile.open(QFile::WriteOnly))
            {
                int prnuDataSize = configContent.cameraPipeline.sensorPrnu.data.size();
                char* prnuData = configContent.cameraPipeline.sensorPrnu.data.data();

                // save the full image
                quint64 writtenBytesCount = prnuFile.write(prnuData, prnuDataSize);
                prnuFile.close();

                if(writtenBytesCount != prnuDataSize)
                {
                    // there is an error
                    LogInFile(QString("  CameraCfg: PRNU data size does not match (%1 != %2)")
                              .arg(writtenBytesCount)
                              .arg(prnuDataSize));
                    LogInApp(QString("  CameraCfg: PRNU data size does not match (%1 != %2)")
                             .arg(writtenBytesCount)
                             .arg(prnuDataSize));
                }
            }
            else
            {
                LogInFile(QString("  CameraCfg: ERROR Can not open").arg(fileName));
                LogInApp(QString("  CameraCfg: ERROR Can not open").arg(fileName));
                res = false;
            }
        }
    }

    return res;
}

bool CfgHelper::_ReadSensorTemperature(QDomElement temperatureNode, SensorTemperature_t& sensorTemperature)
{
    if(!temperatureNode.isNull())
    {
        QDomElement Die= temperatureNode.firstChildElement("Die");
        if(!Die.isNull() && Die.hasChildNodes())
        {
            if(!Die.firstChildElement("Averaged").isNull())
            {
                sensorTemperature.die.averaged = Die.firstChildElement("Averaged").text().toFloat();
            }
            else
            {
                sensorTemperature.die.averaged = Die.text().toFloat();
            }

            if(!Die.firstChildElement("Current").isNull())
            {
                sensorTemperature.die.current = Die.firstChildElement("Current").text().toFloat();
            }
            else
            {
                sensorTemperature.die.current = Die.text().toFloat();
            }
        }
        sensorTemperature.heatsink = temperatureNode.firstChildElement("Heatsink").text().toFloat();
    }

    return true;
}

void CfgHelper::_ConvertNewPRNUArray(int16 *newArray, float *oldArray, float scaleFactor, int32 size)
{
    for (int i=0;i<size;i++)
    {
        oldArray[i] = (1 + ((float)newArray[i]*scaleFactor));
    }
}

bool CfgHelper::ReadCfgCameraPipeline(QString sn, QString path, CfgOutput &output)
{
    INSTANCE(instance);
    return instance->_ReadCfgCameraPipeline(sn, path, output);
}

bool CfgHelper::GetCfgFile(QString path,
                           IrisIndex_t irisIndex,
                           Filter_t filterIndex,
                           Nd_t ndIndex,
                           ConfigContent_t &configContent,
                           CfgOutput &output,
                           bool bLoadOpticalColumn,
                           bool bLoadFlatField)
{
    INSTANCE(instance);
    return instance->_GetCfgFile(path, irisIndex, filterIndex, ndIndex, configContent, output, bLoadOpticalColumn, bLoadFlatField);
}

bool CfgHelper::PackCameraFile(QString sn, QString path, QString zipFilePath)
{
    INSTANCE(instance);

    return instance->_PackCameraFile(sn, path, zipFilePath);
}

bool CfgHelper::_GenerateCfgCamera(QString sn, QString path)
{
    bool res = true;

    QString airshipCfgFileName = QString(AIRSHIP_CFG_PATH).arg(sn);
    airshipCfgFileName = QString("%1/%2").arg(path).arg(airshipCfgFileName);

    QString cameraCfgFileName = QString(CAMERA_CFG_PATH).arg(sn);
    cameraCfgFileName = QString("%1/%2").arg(path).arg(cameraCfgFileName);

    if(sn.isEmpty() == true)
    {
        LogInFile("  CameraCfg: sn is not defined");
        LogInApp("  CameraCfg: sn is not defined");
        res = false;
    }

    if(res == true)
    {
#ifdef REMOVE_CFG_FILE
        LogInApp("  DEBUG delete CAMERA_CFG");
        if(QFileInfo::exists(cameraCfgFileName))
        {
            QFile cfgFile(cameraCfgFileName);
            cfgFile.remove();
        }
#endif

        if(!QFileInfo::exists(cameraCfgFileName))
        {
            LogInFile("  CameraCfg: generate cfg file from airship");
            LogInApp("  CameraCfg: generate cfg file from airship");

            // read airship file and generate camera cfg
            res = _CreateCfgFile(airshipCfgFileName, cameraCfgFileName, mConfigContent);

            if(res == false)
            {
                LogInFile(QString("  CameraCfg: Error airship file not present (%1)").arg(airshipCfgFileName));
                LogInApp(QString("  CameraCfg: Error airship file not present (%1)").arg(airshipCfgFileName));
                res = false;
            }
        }
    }

    return res;
}

bool CfgHelper::_ReadCfgCameraPipeline(QString sn, QString path, CfgOutput &output)
{
    bool res = true;

    // generate camera cfg from airship cfg if it does not exist
    res = _GenerateCfgCamera(sn, path);

    if(res == true)
    {
        QString airshipCfgFileName = QString(AIRSHIP_CFG_PATH).arg(sn);
        airshipCfgFileName = QString("%1/%2").arg(path).arg(airshipCfgFileName);
        output.cameraCfgFileName.SetValue(airshipCfgFileName, QFile::exists(airshipCfgFileName));

        // check whether the config file has already been read
        if(sn.compare(mConfigContentCameraSn) != 0)
        {
            QString cameraCfgFileName = QString(CAMERA_CFG_PATH).arg(sn);
            cameraCfgFileName = QString("%1/%2").arg(path).arg(cameraCfgFileName);
            output.cameraCfgFileName.SetValue(cameraCfgFileName, QFile::exists(cameraCfgFileName));

            LogInFile(QString("  CameraCfg: read %1").arg(cameraCfgFileName));

            // update mConfigContent with camera pipeline
            res = _ReadCfgFile(cameraCfgFileName, mConfigContent);

            if(res == true)
            {
                mConfigContentCameraSn = sn;
            }
            else
            {
                mConfigContentCameraSn = "";
            }

            // in any case, the flat field part is not valid anymore
            mConfigContentFlatFieldIrisIndex = -1;
            mConfigContentFlatFieldFilterIndex = -1;

            // indicate the optical column cfg must be loaded
            mOpticalColumnConfigValid = false;

#ifdef CHECK_PRNU_DATA
            // for debug compare both cfg loaded
            int sizeRef = mConfigContentAirship.cameraPipeline.sensorPrnu.data.size();
            int sizeNew = mConfigContent.cameraPipeline.sensorPrnu.data.size();

            char* ptrRef = mConfigContentAirship.cameraPipeline.sensorPrnu.data.data();
            char* ptrNew = mConfigContent.cameraPipeline.sensorPrnu.data.data();

            for(int index = 0; index < sizeNew; index ++)
            {
                if(ptrRef[index] != ptrNew[index])
                {
                    LogInApp(QString(" ** ERROR @ %1").arg(index));
                }
            }

            LogInApp(QString(" ** Done"));
#endif
        }
    }

    return res;
}

bool CfgHelper::_GetCfgFile(QString path,
                            int irisIndex,
                            int filterIndex,
                            int ndIndex,
                            ConfigContent_t &configContent,
                            CfgOutput &output,
                            bool bLoadOpticalColumn,
                            bool bLoadFlatField)
{
    bool res = false;

    // optical column cfg file
    QString opticalColumnCfgFileName = QString("%1/%2").arg(path).arg(OPTICAL_COLUMN_FILE_NAME);
    output.opticalColumnCfgFileName.SetValue(opticalColumnCfgFileName, QFile::exists(opticalColumnCfgFileName));

    // flat field data
    QString flatFieldFileName = QString(FLAT_FIELD_FILE_NAME)
            .arg(RESOURCE->ToString((IrisIndex_t)irisIndex, true))
            .arg(RESOURCE->ToString((Filter_t)filterIndex));
    flatFieldFileName = QString("%1/%2").arg(path).arg(flatFieldFileName);
    output.flatFieldFileName.SetValue(flatFieldFileName, QFile::exists(flatFieldFileName));

    LogInFile(QString("  Cfg: %1").arg(opticalColumnCfgFileName));
    LogInFile(QString("  Cfg: %1").arg(flatFieldFileName));

    // read config file if necessary
    if(bLoadOpticalColumn == true)
    {
        res = _ReadConfigFile(opticalColumnCfgFileName, output);
        output.opticalColumnCfgFileName.SetValid(res);
    }
    else
    {
        res = true;
        output.opticalColumnCfgFileName.SetValid(res);

        mOpticalColumnConfig.flatField.MaximumIncidentAngle = 60;
        mOpticalColumnConfig.flatField.radius = 3000;
    }

    if(bLoadFlatField == false)
    {
        mOpticalColumnConfig.flatField.MaximumIncidentAngle = 60;
        mOpticalColumnConfig.flatField.radius = 3000;
    }

    // update config file
    if(res == true)
    {
        mConfigContent.opticalColumnCalibration.sensorTemperatureDependency.correctionEnable = false;
        mConfigContent.opticalColumnCalibration.sensorTemperatureDependency.slope = 0;

        // center
        OpticalAxis_t opticalAxis;
        opticalAxis = mOpticalColumnConfig.center.opticalAxis[filterIndex][ndIndex];

        mConfigContent.opticalColumnCalibration.captureArea.opticalAxis.X = opticalAxis.X;
        mConfigContent.opticalColumnCalibration.captureArea.opticalAxis.Y = opticalAxis.Y;

        // linearisation
        LinearisationCoefficient_t linCoef;
        linCoef = mOpticalColumnConfig.linearisation.coefficients[filterIndex];

        mConfigContent.opticalColumnCalibration.linearizationCoefficients.A1 = linCoef.A1;
        mConfigContent.opticalColumnCalibration.linearizationCoefficients.A3 = linCoef.A3;
        mConfigContent.opticalColumnCalibration.linearizationCoefficients.A5 = linCoef.A5;
        mConfigContent.opticalColumnCalibration.linearizationCoefficients.A7 = linCoef.A7;
        mConfigContent.opticalColumnCalibration.linearizationCoefficients.A9 = linCoef.A9;

        mConfigContent.opticalColumnCalibration.flatField.isCalibrated = true;

        // flat field
        float maximumIncidenceAngle = mOpticalColumnConfig.flatField.MaximumIncidentAngle;
        int radius = mOpticalColumnConfig.flatField.radius;

        // flatfield configuration
        mConfigContent.opticalColumnCalibration.maximumIncidentAngle = maximumIncidenceAngle;
        mConfigContent.opticalColumnCalibration.calibratedDataRadius = radius;

        mConfigContent.opticalColumnCalibration.flatField.maximumIncidentAngle = maximumIncidenceAngle;
        mConfigContent.opticalColumnCalibration.flatField.radius = radius;

        int calibratedDataMatrixSize = radius * 2 + 1;
        long flatFieldSize = calibratedDataMatrixSize * calibratedDataMatrixSize;

        mConfigContent.opticalColumnCalibration.flatField.data.resize(flatFieldSize * sizeof(int16));

        mConfigContent.calibrationSummary.date    = mOpticalColumnConfig.summary.date;
        mConfigContent.calibrationSummary.time    = mOpticalColumnConfig.summary.time;
        mConfigContent.calibrationSummary.comment = mOpticalColumnConfig.summary.comment;

        // default flat field buffer
        // read flat field matching the configuration
        if(bLoadFlatField == true)
        {
            if((irisIndex != mConfigContentFlatFieldIrisIndex) ||
               (filterIndex != mConfigContentFlatFieldFilterIndex))
            {
                std::memset(mConfigContent.opticalColumnCalibration.flatField.data.data(),
                            1,
                            flatFieldSize * sizeof(int16));

                res = _ReadFlatFieldFile(flatFieldFileName,
                                         mConfigContent.opticalColumnCalibration.flatField.data,
                                         flatFieldSize);

                if(res == true)
                {
                    mConfigContentFlatFieldIrisIndex = irisIndex;
                    mConfigContentFlatFieldFilterIndex = filterIndex;
                }
            }
            else
            {

            }
        }
    }

    if((bLoadOpticalColumn == false) ||
       (bLoadFlatField == false))
    {
        mConfigContentFlatFieldIrisIndex = -1;
        mConfigContentFlatFieldFilterIndex = -1;
    }

    if(res == true)
    {
        // output
        configContent = mConfigContent;

        float colorCoefCorr = mOpticalColumnConfig.colorCoefCorr.coefficients[filterIndex][ndIndex];

        // std::map<int, std::map<int, std::map<int, float>>> coefficients; // index is irisIndex, index is color comp, index is filterIndex
        float colorCoefCompX = mOpticalColumnConfig.colorCoefComp.coefficients[irisIndex][(int)ComposantType_X][filterIndex];
        float colorCoefCompY = mOpticalColumnConfig.colorCoefComp.coefficients[irisIndex][(int)ComposantType_Y][filterIndex];
        float colorCoefCompZ = mOpticalColumnConfig.colorCoefComp.coefficients[irisIndex][(int)ComposantType_Z][filterIndex];

        colorCoefCompX = colorCoefCompX * colorCoefCorr;
        colorCoefCompY = colorCoefCompY * colorCoefCorr;
        colorCoefCompZ = colorCoefCompZ * colorCoefCorr;

        output.colorCoefCompX.SetValue(colorCoefCompX);
        output.colorCoefCompY.SetValue(colorCoefCompY);
        output.colorCoefCompZ.SetValue(colorCoefCompZ);
    }
    else
    {
        output.colorCoefCompX.SetValue(0, false);
        output.colorCoefCompY.SetValue(0, false);
        output.colorCoefCompZ.SetValue(0, false);
    }

    return res;
}

bool CfgHelper::_ReadConfigFile(QString fileName, CfgOutput&)
{
    bool res = true;

    _Log(QString("Read config file (%1)").arg(fileName));

    // read the cfg if it has not already been read
    if(mOpticalColumnConfigValid == false)
    {
        QFile file(fileName);

        // open the file
        if(!file.open(QIODevice::ReadOnly))
        {
            _Log("Can not read configuration file");
            LogInFile(QString("  ERROR can not read %1").arg(fileName));
            res = false;
        }

        QDomDocument cfgConfig("OpticalColumnConfigFile");

        // read the content of the file
        if(res == true)
        {
            if(!cfgConfig.setContent(&file))
            {
                _Log("Can not set contentfile");
                LogInFile(QString("  ERROR can set contentfile %1").arg(fileName));
                res = false;
            }

            file.close();
        }

        if(res == true)
        {
            QDomElement configElement = cfgConfig.documentElement();
            QDomElement Content= configElement.firstChildElement("Content");

            // read equipment infos
            QDomElement inSummary = Content.firstChildElement("Summary");
            _ReadSummarySection(inSummary, mOpticalColumnConfig.summary);

            QDomElement inEquipment = Content.firstChildElement("Equipment");
            _ReadEquipmentSection(inEquipment, mOpticalColumnConfig.equipement);

            QDomElement inLinearisation = Content.firstChildElement("Linearisation");
            _ReadLinearisationSection(inLinearisation, mOpticalColumnConfig.linearisation);

            QDomElement inCenter = Content.firstChildElement("Center");
            _ReadCenterSection(inCenter, mOpticalColumnConfig.center);

            QDomElement inFlatField = Content.firstChildElement("FlatField");
            _ReadFlatFieldSection(inFlatField, mOpticalColumnConfig.flatField);

            QDomElement inColorCoef = Content.firstChildElement("ColorCoef");
            _ReadColorCoefSection(inColorCoef, mOpticalColumnConfig.colorCoefComp);

            QDomElement inColorCoefComp = Content.firstChildElement("ColorCoefComp");
            _ReadColorCoefCompSection(inColorCoefComp, mOpticalColumnConfig.colorCoefComp);

            QDomElement inColorCoefCorr = Content.firstChildElement("ColorCoefCorr");
            _ReadColorCoefCorrSection(inColorCoefCorr, mOpticalColumnConfig.colorCoefCorr);
        }

        if(res == true)
        {
            // indicate the cfg file has been loaded
            // no need to load it again
            mOpticalColumnConfigValid = true;
        }
    }

    return res;
}

bool CfgHelper::_ReadLinearisationSection(QDomElement& inLinearisation,
                                          Linearisation_t& data)
{
    int filterIndex;
    LinearisationCoefficient_t coefficient;

    // retrieve all entries
    QDomNode inCoefficient = inLinearisation.firstChild();
    while(!inCoefficient.isNull())
    {
        if(inCoefficient.isElement())
        {
            filterIndex = inCoefficient.toElement().attribute("FilterIndex").toShort();
            coefficient.A1 = inCoefficient.toElement().attribute("A1").toFloat();
            coefficient.A3 = inCoefficient.toElement().attribute("A3").toFloat();
            coefficient.A5 = inCoefficient.toElement().attribute("A5").toFloat();
            coefficient.A7 = inCoefficient.toElement().attribute("A7").toFloat();
            coefficient.A9 = inCoefficient.toElement().attribute("A9").toFloat();

            data.coefficients[filterIndex] = coefficient;
        }
        inCoefficient = inCoefficient.nextSibling();
    }

    return true;
}

bool CfgHelper::_ReadCenterSection(QDomElement& inCenter,
                                   Center_t& data)
{
    int filterIndex;
    int ndIndex;
    OpticalAxis_t axis;

    // retrieve all entries
    QDomNode inOpticalAxis = inCenter.firstChild();
    while(!inOpticalAxis.isNull())
    {
        if(inOpticalAxis.isElement())
        {
            filterIndex = inOpticalAxis.toElement().attribute("FilterIndex").toShort();
            ndIndex     = inOpticalAxis.toElement().attribute("NdIndex").toShort();

            axis.X = inOpticalAxis.toElement().attribute("X").toShort();
            axis.Y = inOpticalAxis.toElement().attribute("Y").toShort();

            data.opticalAxis[filterIndex][ndIndex] = axis;
        }
        inOpticalAxis = inOpticalAxis.nextSibling();
    }

    return true;
}

bool CfgHelper::_ReadColorCoefSection(QDomElement& inColorCoefs,
                                      ColorCoefComp_t& data)
{
    int irisIndex;
    int filterIndex;
    float coef;

    // retrieve all entries
    QDomNode inColorCoef = inColorCoefs.firstChild();
    while(!inColorCoef.isNull())
    {
        if(inColorCoef.isElement())
        {
            irisIndex   = inColorCoef.toElement().attribute("IrisIndex").toShort();
            filterIndex = inColorCoef.toElement().attribute("FilterIndex").toShort();
            coef        = inColorCoef.toElement().attribute("Coef").toFloat();

            // initialise
            data.coefficients[irisIndex][(int)ComposantType_X][filterIndex] = 0;
            data.coefficients[irisIndex][(int)ComposantType_Y][filterIndex] = 0;
            data.coefficients[irisIndex][(int)ComposantType_Z][filterIndex] = 0;

            Filter_t eFilter = (Filter_t)filterIndex;

            switch(eFilter)
            {
            case Filter_X:
            case Filter_Xz:
                data.coefficients[irisIndex][(int)ComposantType_X][filterIndex] = coef;
                break;

            case Filter_Ya:
            case Filter_Yb:
                data.coefficients[irisIndex][(int)ComposantType_Y][filterIndex] = coef;
                break;

            case Filter_Z:
                data.coefficients[irisIndex][(int)ComposantType_Z][filterIndex] = coef;
                break;
            }
        }
        inColorCoef = inColorCoef.nextSibling();
    }

    return true;
}

bool CfgHelper::_ReadColorCoefCompSection(QDomElement& inColorCoefComps, ColorCoefComp_t& data)
{
    int irisIndex;
    QString comp;
    ComposantType_t composantType;

    int filterIndex;
    float coef;

    QMap<QString, ComposantType_t> compMap;

    compMap["X"] = ComposantType_X;
    compMap["Y"] = ComposantType_Y;
    compMap["Z"] = ComposantType_Z;

    QDomNode inColorCoefComp = inColorCoefComps.firstChild();
    while(!inColorCoefComp.isNull())
    {
        if(inColorCoefComp.isElement())
        {
            irisIndex = inColorCoefComp.firstChildElement("IrisIndex").text().toShort();
            comp = CONVERT_TO_QSTRING(inColorCoefComp.firstChildElement("Comp").text().toStdString()).simplified();

            if(compMap.contains(comp))
            {
                composantType = compMap[comp];
            }

            QDomNode inCoefs = inColorCoefComp.firstChildElement("Coef");
            QDomNode inCoef = inCoefs.firstChild();
            while(!inCoef.isNull())
            {
                if(inCoef.isElement())
                {
                    filterIndex = inCoef.toElement().attribute("FilterIndex").toShort();
                    coef        = inCoef.toElement().attribute("Coef").toFloat();

                    data.coefficients[irisIndex][(int)composantType][filterIndex] = coef;
                }
                inCoef = inCoef.nextSibling();
            }
        }
        inColorCoefComp = inColorCoefComp.nextSibling();
    }

    return true;
}

bool CfgHelper::_ReadColorCoefCorrSection(QDomElement& inColorCoefs,
                                          ColorCoefCorr_t& data)
{
    int filterIndex;
    int ndIndex;

    float corr;

    // retrieve all entries
    QDomNode inColorCoef = inColorCoefs.firstChild();
    while(!inColorCoef.isNull())
    {
        if(inColorCoef.isElement())
        {
            filterIndex = inColorCoef.toElement().attribute("FilterIndex").toShort();
            ndIndex     = inColorCoef.toElement().attribute("NdIndex").toShort();

            corr        = inColorCoef.toElement().attribute("Corr").toFloat();

            data.coefficients[filterIndex][ndIndex] = corr;
        }
        inColorCoef = inColorCoef.nextSibling();
    }

    return true;
}

bool CfgHelper::_ReadFlatFieldSection(QDomElement& inFlatField,
                                      FlatField_t& flatField)
{
    bool res = true;

    if(!inFlatField.isNull() && inFlatField.isElement())
    {
        float maximumIncidenceAngle = inFlatField.firstChildElement("MaximumIncidentAngle").text().toFloat();
        int radius = inFlatField.firstChildElement("Radius").text().toShort();

        flatField.MaximumIncidentAngle = maximumIncidenceAngle;
        flatField.radius = radius;
    }

    return res;
}

bool CfgHelper::_ReadFlatFieldFile(
        QString fileName,
        std::vector<char>& data,
        long expectedSize)
{
    bool res = true;
    QByteArray buffer;

    QFile ff(QString("%1").arg(fileName));

    if(ff.exists() && ff.open(QFile::ReadOnly))
    {
        int fileSize = ff.size();

        if(fileSize == (expectedSize * sizeof(int16)))
        {
            buffer.resize(fileSize);
            buffer = ff.readAll();

            data.resize(fileSize);
            std::memcpy(data.data(),
                        buffer.data(),
                        fileSize);
        }
        else
        {
            // the size is not good
            LogInFile(QString("  ERROR size is not good %1").arg(fileName));

            res = false;
        }

        ff.close();
    }
    else
    {
        LogInFile(QString("  ERROR file not found %1").arg(fileName));

        res = false;
    }

    return res;
}

bool CfgHelper::_PackCameraFile(QString sn, QString path, QString zipFilePath)
{
    bool res = false;
    QString pathTmp;
    QStringList fileNames;

    // generate camera files if required
    res = _GenerateCfgCamera(sn, path);

    // zip the files
    if(res == true)
    {
        // read json configuration files
        QStringList flatFieldFileNames;

        if( _PackCameraFileListLoad(flatFieldFileNames) == false)
        {
            // no json files, set default values
            for(int irisIndex = 2; irisIndex < 6; irisIndex ++)
            {
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("BK7"));
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("IrCut"));
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("X"));
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("Xz"));
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("Ya"));
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("Yb"));
                flatFieldFileNames.append(QString(FLAT_FIELD_FILE_NAME).arg(irisIndex).arg("Z"));
            }

            // create the json file
            _PackCameraFileListSave(flatFieldFileNames);
        }

        // add camera files
        QString cameraFileName = QString(CAMERA_CFG_PATH).arg(sn);
        fileNames.append(cameraFileName);
        fileNames.append(CAMERA_PRNU_PATH(cameraFileName));
        fileNames.append("OpticalColumn.xml");

        // add flat fields files
        fileNames.append(flatFieldFileNames);

        // copy files into a directory
        pathTmp = QDir::cleanPath(path + QDir::separator() + "tmp");

        QDir().mkpath(pathTmp);

        QString src;
        QString dest;

        LogInApp(QString("pack files:"));

        foreach(QString fileName, fileNames)
        {
            src = QDir::cleanPath(path + QDir::separator() + fileName);
            dest = QDir::cleanPath(pathTmp + QDir::separator() + fileName);

            if(QFile(src).exists())
            {
                QFile(dest).remove();

                if(QFile::copy(src, dest))
                {
                    LogInApp(QString("    %1").arg(fileName));
                }
                else
                {
                     LogInApp(QString("    %1 <- ERROR").arg(fileName, -40));
                     res = false;
                }
            }
            else
            {
                LogInApp(QString("    %1 <- ERROR").arg(fileName, -40));
                res = false;
            }
        }

        if(res == false)
        {
            LogInApp(QString("Error: please check if files are missing"));
        }
    }

    if(res == true)
    {
        // zip files
        QString _cfgPath = pathTmp;
        QString _fileName = zipFilePath;

        QString cmdLine =
                QString("powershell.exe -nologo -noprofile -command \"& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('%1', '%2'); }\"")
                .arg(pathTmp)
                .arg(zipFilePath);

        QProcess::execute(cmdLine);

        LogInApp(QString("zip file created"));
    }

    // clean temp directory
    QDir dir(pathTmp);
    dir.removeRecursively();

    return res;
}

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextStream>
#include <QJsonArray>

#define CAMERA_FILE_LIST_NAME    "CalibrationFileList.json"
#define FLAT_FIELD_FILES_LABEL   "FlatFieldFiles"

bool CfgHelper::_PackCameraFileListLoad(QStringList &fileList)
{
    bool res = true;

    // read json file list
    QFile jsonFile(CAMERA_FILE_LIST_NAME);

    if(jsonFile.open(QIODevice::ReadOnly))
    {
        QByteArray fileArray = jsonFile.readAll();

        QJsonDocument loadDoc(QJsonDocument::fromJson(fileArray));
        QJsonObject object = loadDoc.object();

        if(object.contains(FLAT_FIELD_FILES_LABEL))
        {
            QJsonArray input = object[FLAT_FIELD_FILES_LABEL].toArray();

            fileList.clear();

            for(int index = 0; index < input.count(); index ++)
            {
                QString key = input[index].toString();

                fileList.append(key);
            }
        }
        else
        {
            res = false;
        }

        jsonFile.close();
    }
    else
    {
        res = false;
    }

    return res;
}

bool CfgHelper::_PackCameraFileListSave(QStringList &fileList)
{
    bool res = true;
    // QJsonObject objectAppConfig;

    QJsonArray array;

    foreach(QString fileName, fileList)
    {
        array.append(fileName);
    }

    //#define JSON_INSERT(a, b) object##a.insert(TOSTRING(b), m##a.b)
    // objectAppConfig.insert(FLAT_FIELD_FILES_LABEL, array);

    // record
    QJsonObject recordObject;
    recordObject.insert(FLAT_FIELD_FILES_LABEL,   array);

    QJsonDocument doc(recordObject);

    QFile jsonFile(CAMERA_FILE_LIST_NAME);
    if(jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&jsonFile);
        out.setCodec("UTF-8");
        out << doc.toJson();
        jsonFile.close();
    }

    return res;
}

