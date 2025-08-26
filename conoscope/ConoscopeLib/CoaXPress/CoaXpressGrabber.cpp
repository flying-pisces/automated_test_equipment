#include "CoaXpressGrabber.h"

QString CoaXpressGrabber::mScriptFile = "";

const AddressDefinition CoaXpressGrabber::mI2CAddressList[] =
        {{0x65F8, 0x6638},
         {0x65FC, 0x67FC}};

CoaXpressGrabber_CameraSettings::CoaXpressGrabber_CameraSettings(const CoaXpressGrabber_CameraSettings& settings)
{
    pixelFormat                  = settings.pixelFormat;
    binningHorizontal            = settings.binningHorizontal;
    binningVertical              = settings.binningVertical;
    DefectPixelCorrectionEnable  = settings.DefectPixelCorrectionEnable;
    DF_ColumnOffsetCorrection    = settings.DF_ColumnOffsetCorrection;
    BF_ColumnGainCorrection      = settings.BF_ColumnGainCorrection;
    exposureMode                 = settings.exposureMode;
    blackLevel                   = settings.blackLevel;
    SensitivityMatching          = settings.SensitivityMatching;
}

CoaXpressGrabber_Config::CoaXpressGrabber_Config()
{
    exposureUs        = 0;
    acquisitionNumber = 0;
    VBin              = 0;
    bExtTrig          = false;
    bTestPattern      = false;
    trigDelayMs       = 0;
}

CoaXpressGrabber_Config::CoaXpressGrabber_Config(const CoaXpressGrabber_Config& value)
{
    exposureUs        = value.exposureUs;
    acquisitionNumber = value.acquisitionNumber;
    VBin              = value.VBin;
    dimensions        = value.dimensions;
    bExtTrig          = value.bExtTrig;
    bTestPattern      = value.bTestPattern;
    trigDelayMs       = value.trigDelayMs;
}

#ifndef USE_POP
CoaXpressGrabber::CoaXpressGrabber(EGenTL &gentl, bool bPowerCycle, QObject *parent) :  QObject(parent), EGrabber<CallbackMultiThread>(gentl)
// CoaXpressGrabber::CoaXpressGrabber(EGenTL &gentl, QObject *parent) :  QObject(parent), EGrabber<CallbackSingleThread>(gentl)
#else
CoaXpressGrabber::CoaXpressGrabber(EGenTL &gentl, bool bPowerCycle, QObject *parent) :  QObject(parent), EGrabber<CallbackOnDemand>(gentl)
#endif
{
    mI2CAddressIndex = 0; // should be updated depending on firmware version

    mFileTransferPacketSize = FILE_TRANSFER_PACKET_SIZE_V10;
    mFileTransferBuffer.resize(mFileTransferPacketSize);

    // reset the camera
    if(bPowerCycle == true)
    {
        PowerCycle();
    }

    enableEvent<NewBufferData>();
    reallocBuffers(1);
}

void CoaXpressGrabber::getInterfaceInfo()
{
        // frame grabber information
        std::string vendorName = getString<Euresys::SystemModule>("TLVendorName");
        std::string modelName  = getString<Euresys::SystemModule>("TLModelName");
        std::string id         = getString<Euresys::SystemModule>("TLID");
        std::string version    = getString<Euresys::SystemModule>("TLVersion");
        std::string path       = getString<Euresys::SystemModule>("TLPath");
        std::string type       = getString<Euresys::SystemModule>("TLType");
        std::string versionMajor = getString<Euresys::SystemModule>("GenTLVersionMajor");
        std::string versionMinor = getString<Euresys::SystemModule>("GenTLVersionMinor");

        QString versionInfo = QString::fromStdString(version);
        QStringList versionInfoList = versionInfo.split(".");

        QList<int> interfaceVersion;

        for(int index = 0; index < versionInfoList.length(); index++)
        {
            int value = versionInfoList[index].toInt();
            interfaceVersion.append(value);
        }

        mInterfaceInfo.version = interfaceVersion;
}

void CoaXpressGrabber::DetectCamera(std::string connection)
{
    mCameraManufacturer = CameraManufacturer_Unknown;
    mAcquisitionCount = 0;

    // runScript("config.js");

    //enableEvent<NewBufferData>();
    //reallocBuffers(1);

    // read camera type
    _ReadCameraInfo();

    // configure the number of connection prior to anything
    SetConnectionConfig(connection);

    SetCameraDefaultConfiguration();

    SetCameraConstants();

    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:
        break;

    case CameraManufacturer_CriticalLink:
        // be sure no acquisition is on going
        execute<Euresys::RemoteModule>("AcquisitionStop");

        if(connection == "CXP6_X4")
        {
            // 20 fps
            mAcquisitionFramePeriodMin = 50000;
        }
        else if(connection == "CXP6_X2")
        {
            // 10 fps
            mAcquisitionFramePeriodMin = 100000;

            // wa for JRTI
            if(this->mCameraInfo.cameraModelName == "CMV8000_CXP")
            {
                mAcquisitionFramePeriodMin = 200000;
            }
        }
        else
        {
            // 1 fps
            mAcquisitionFramePeriodMin = 1000000;
        }

        break;

    default:
        break;
    }

    // DeviceAccessStatus

    UpdateFrameSize();

    PrintConfig();
}

void CoaXpressGrabber::SetCameraDefaultConfiguration()
{
    int index = 0;

    QString niosSwVersion = "";
    QStringList niosSwVersionList;

    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:
        // configure the camera
#ifdef DISPLAY_FRAME_FORMAT
        setString<Euresys::RemoteModule>("PixelFormat", "Mono8");
#else
        setString<Euresys::RemoteModule>("PixelFormat", "Mono12");
#endif

        // default setting of the camera
        setInteger<Euresys::RemoteModule>("BinningHorizontal", 1);
        setInteger<Euresys::RemoteModule>("BinningVertical", 1);

        // DefectPixelCorrection disable
        setInteger<Euresys::RemoteModule>("DefectPixelCorrectionEnable", 0);

        // DarkFieldCorrection disable
        setInteger<Euresys::RemoteModule>("DF_ColumnOffsetCorrection", 0);

        // BrightFieldCorrection disable
        setInteger<Euresys::RemoteModule>("BF_ColumnGainCorrection", 0);

        // set exposure mode
        setString<Euresys::RemoteModule>("ExposureMode", "Timed");

        // set black level
        setInteger<Euresys::RemoteModule>("BlackLevel", 128);

        // force sensitivity matching
        // note: that the factory settings of the camera may disable all the amplifiers
        setInteger<Euresys::RemoteModule>("SensitivityMatching", 1);

        // (please refer to D203EN-GenICam_Reference-CoaxLink.pdf)

/*
        std::string connectionConfigDefault = "CXP6_X1";
        setString<Euresys::RemoteModule>("ConnectionConfig", connectionConfigDefault);

        connectionConfigDefault = "CXP6_X4";
        setString<Euresys::RemoteModule>("ConnectionConfig", connectionConfigDefault);
*/

        // run script configuration (static variable)
        runScript(_LoadScriptStatement(mScriptFile));

        // DeviceUpdateList
        // DeviceSelector

        // retrieve camera settings
        mCameraSettings.pixelFormat                   = getString<Euresys::RemoteModule>("PixelFormat");
        mCameraSettings.binningHorizontal             = getInteger<Euresys::RemoteModule>("BinningHorizontal");
        mCameraSettings.binningVertical               = getInteger<Euresys::RemoteModule>("BinningVertical");
        mCameraSettings.DefectPixelCorrectionEnable   = (bool) getInteger<Euresys::RemoteModule>("DefectPixelCorrectionEnable");
        mCameraSettings.DF_ColumnOffsetCorrection     = (bool)getInteger<Euresys::RemoteModule>("DF_ColumnOffsetCorrection");
        mCameraSettings.BF_ColumnGainCorrection       = (bool)getInteger<Euresys::RemoteModule>("BF_ColumnGainCorrection");
        mCameraSettings.exposureMode                  = getString<Euresys::RemoteModule>("ExposureMode");
        mCameraSettings.blackLevel                    = getInteger<Euresys::RemoteModule>("BlackLevel");
        mCameraSettings.SensitivityMatching           = (bool)getInteger<Euresys::RemoteModule>("SensitivityMatching");

        // 20 fps
        mAcquisitionFramePeriodMin = 50000;

        break;

    case CameraManufacturer_CriticalLink:

        mCameraSettings.pixelFormat                   = "Mono12";
        mCameraSettings.binningHorizontal             = 0;
        mCameraSettings.binningVertical               = 0;
        mCameraSettings.DefectPixelCorrectionEnable   = "NA";
        mCameraSettings.DF_ColumnOffsetCorrection     = "NA";
        mCameraSettings.BF_ColumnGainCorrection       = "NA";
        mCameraSettings.exposureMode                  = "NA";
        mCameraSettings.blackLevel                    = 0;
        mCameraSettings.SensitivityMatching           = "NA";

        // first register to fill
        setInteger<Euresys::RemoteModule>("DeviceStreamChannelPacketSize", 4096);

        setString<Euresys::RemoteModule>("DeviceIndicatorMode", "Active");

        // fill settings required for file transfer
        if(mInterfaceInfo.version[0] >= 11)
        {
            setString<Euresys::InterfaceModule>("ShowCoaXPressAdvancedFeatures", "True");
            setString<Euresys::InterfaceModule>("CxpControlParameterSelector", "ControlPacketSizeMax");
            setInteger<Euresys::InterfaceModule>("CxpControlParameter", 1024);

            mFileTransferPacketSize = FILE_TRANSFER_PACKET_SIZE_V11;
        }
        else
        {
            mFileTransferPacketSize = FILE_TRANSFER_PACKET_SIZE_V10;
        }
        mFileTransferBuffer.resize(mFileTransferPacketSize);

        // retrieve nios fw version
        index = 0;
        while(index < mCameraInfo.cameraSpecific.length())
        {
            if(mCameraInfo.cameraSpecific[index].name == "NiosSwVersion")
            {
                niosSwVersion = QString::fromStdString(mCameraInfo.cameraSpecific[index].value);
                break;
            }
            index ++;
        }
        niosSwVersionList = niosSwVersion.split(".");

        if(niosSwVersionList.count() == 2)
        {
            mMajorNumber = niosSwVersionList[0].toInt();
            mMinorNumber = niosSwVersionList[1].toInt();
        }

        // set pixel format
        if(mCameraInfo.cameraModelName == "CMV8000_CXP")
        {
            setString<Euresys::RemoteModule>("PixelFormat", "Mono12");
        }
        else
        {
            // from fw 20190522 Mono12p is replaced by Mono12
            // keep backward compatibility
            if((mMajorNumber < 1) ||
               ((mMajorNumber == 1) && (mMinorNumber < 14193)))
            {
                setString<Euresys::RemoteModule>("PixelFormat", "Mono12p");
            }
            else
            {
                setString<Euresys::RemoteModule>("PixelFormat", "Mono12");
            }
        }

        setString<Euresys::RemoteModule>("TestPattern", "Off");

        setString<Euresys::RemoteModule>("AcquisitionMode", "Continuous");

        setFloat<Euresys::RemoteModule>("AcquisitionFrameRate", 1.0);
        //setFloat<Euresys::RemoteModule>("MSRM_Int_AcquisitionFrameInterval", 1000000);

        if(mCameraInfo.cameraModelName == "CMV8000_CXP")
        {
            setFloat<Euresys::RemoteModule>("ExposureTime", 5000.0);
        }

        try
        {
            if(mCameraInfo.cameraModelName != "CMV8000_CXP")
            {
                // configure PID
                setInteger<Euresys::RemoteModule>("PIDEnabled", 1);
                setFloat<Euresys::RemoteModule>("PIDP", 0.25);
                setFloat<Euresys::RemoteModule>("PIDI", 0.013);
                setFloat<Euresys::RemoteModule>("PIDD", 0.0);
                setFloat<Euresys::RemoteModule>("PIDTarget", 25);

                // configure fan
                setFloat<Euresys::RemoteModule>("FanSelect", 1);
                setFloat<Euresys::RemoteModule>("FanPeriod", 100);
#ifndef QUIET_MODE
                setFloat<Euresys::RemoteModule>("FanWidth", 80);
#else
                // for debug purpose (QUIET)
                setFloat<Euresys::RemoteModule>("FanWidth", 40);
#endif

                // configure gain
                setString<Euresys::RemoteModule>("AnalogGain", "Gain1x");
            }
            else
            {
                // configure PID
                setInteger<Euresys::RemoteModule>("PIDEnabled", 0);

                // configure fan
                setFloat<Euresys::RemoteModule>("FanSelect", 1);
                setFloat<Euresys::RemoteModule>("FanPeriod", 100);
#ifndef QUIET_MODE
                setFloat<Euresys::RemoteModule>("FanWidth", 80);
#else
                // for debug purpose (QUIET)
                setFloat<Euresys::RemoteModule>("FanWidth", 40);
#endif

                // configure gain
                setString<Euresys::RemoteModule>("AnalogGain", "Gain1x0");
            }

#define FORCE_IMAGE_SIZE_TO_SENSOR_SIZE
#ifdef FORCE_IMAGE_SIZE_TO_SENSOR_SIZE
            // force image size to sensor size
            // ifever it is not correctly configured
            int width  = getInteger<Euresys::RemoteModule>("SensorWidth");
            int height = getInteger<Euresys::RemoteModule>("SensorHeight");

            setInteger<Euresys::RemoteModule>("OffsetX", 0);
            setInteger<Euresys::RemoteModule>("OffsetY", 0);
            setInteger<Euresys::RemoteModule>("Width",   width);
            setInteger<Euresys::RemoteModule>("Height",  height);
#endif

        }
        catch(...)
        {
        }
        break;

    default:
        break;
    }
}

void CoaXpressGrabber::SetCameraConstants()
{
    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:
        break;

    case CameraManufacturer_CriticalLink:

        // change I2C address depending on firmware version
        if((mMajorNumber < 1) ||
           ((mMajorNumber == 1) && (mMinorNumber < 15043)))
        {
            mI2CAddressIndex = 0;
        }
        else
        {
            mI2CAddressIndex = 1;
        }

        break;
    }
}

#define EXPOSURE_TIME_MIN 100

void CoaXpressGrabber::Configure(CoaXpressGrabber_Config& config)
{
    // store the configuration
    mConfig = config;

    // exposure time
    int exposureTimeUs = EXPOSURE_TIME_MIN;

    if(config.exposureUs >= exposureTimeUs)
    {
        exposureTimeUs = config.exposureUs;
    }
    else
    {
        config.exposureUs = exposureTimeUs;
    }

#ifndef FIXED_FPS
    // adjust acquisition period
    // it should be bigger than the processing time
    mCurrentAcquisitionFramePeriod = (exposureTimeUs > PROCESSING_TIME_US) ? exposureTimeUs : PROCESSING_TIME_US;

    mCurrentAcquisitionFramePeriod = (mCurrentAcquisitionFramePeriod < mAcquisitionFramePeriodMin) ?
                mAcquisitionFramePeriodMin : mCurrentAcquisitionFramePeriod;

    // there are some lag in the sensor capture
    mCurrentAcquisitionFramePeriod += PROCESSING_SENSOR_LAG_US;

#ifdef STD_DEV_FILE
    if(config.bStoreStdDev == true)
    {
        mCurrentAcquisitionFramePeriod += PROCESSING_STD_DEV_PROC_US;
    }
#endif
#else
    // for debug purpose
    mCurrentAcquisitionFramePeriod = 1000000;
#endif

    int imageWidth  = config.dimensions.width();
    int imageHeight = config.dimensions.height();

    int xOffset = 0;
    int yOffset = 0;

    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:

        if(mCurrentAcquisitionFramePeriod > MAX_ACQUISITION_FRAME_PERIOD)
        {
            mCurrentAcquisitionFramePeriod = MAX_ACQUISITION_FRAME_PERIOD;
        }

        setInteger<Euresys::RemoteModule>("AcquisitionFramePeriod", mCurrentAcquisitionFramePeriod);
        PRINT("  grabber", QString("AcquisitionFramePeriod = %1").arg(mCurrentAcquisitionFramePeriod));

        // dimension
        xOffset = config.dimensions.x();
        yOffset = config.dimensions.y();

        setInteger<Euresys::RemoteModule>("Width", imageWidth);
        setInteger<Euresys::RemoteModule>("Height", imageHeight);

        setInteger<Euresys::RemoteModule>("OffsetX", xOffset);
        setInteger<Euresys::RemoteModule>("OffsetY", yOffset);

        break;

    case CameraManufacturer_CriticalLink:
        // hard coded
        // mCurrentAcquisitionFramePeriod = 1000000;
        // mCurrentAcquisitionFramePeriod = 1000000;

        setFloat<Euresys::RemoteModule>("MSRM_Int_AcquisitionFrameInterval", mCurrentAcquisitionFramePeriod);

#ifdef LOG_ACQUISITION_FRAME_PERIOD
        AcqFR = getFloat<Euresys::RemoteModule>("AcquisitionFrameRate");
        AcqFI = getFloat<Euresys::RemoteModule>("MSRM_Int_AcquisitionFrameInterval");

        PRINT("  grabber", QString("AcquisitionFrameRate = %1").arg(AcqFR));
        PRINT("  grabber", QString("MSRM_Int_AcquisitionFrameInterval = %1").arg(AcqFI));
#endif

        if(config.bTestPattern == true)
        {
            // setString<Euresys::RemoteModule>("AcquisitionMode", "Continuous");
            setString<Euresys::RemoteModule>("TestPattern", "SensorTestPattern");
        }
        else
        {
            setString<Euresys::RemoteModule>("TestPattern", "Off");
        }

        if((config.dimensions.height() >= 6004) || (config.dimensions.width() >= 7920))
        {
            mImageFeature.width  = 8112;
            mImageFeature.height = 6048;
            mImageFeature.offsetX = 0;
            mImageFeature.offsetY = 0;
            // keep the size of the sensor, image is cropped when retrieved
            setInteger<Euresys::RemoteModule>("OffsetX", mImageFeature.offsetX);
            setInteger<Euresys::RemoteModule>("OffsetY", mImageFeature.offsetY);
            setInteger<Euresys::RemoteModule>("Width",   mImageFeature.width);
            setInteger<Euresys::RemoteModule>("Height",  mImageFeature.height);

        }
        else
        {
            setInteger<Euresys::RemoteModule>("Width",   config.dimensions.width());
            setInteger<Euresys::RemoteModule>("Height",  config.dimensions.height());
            setInteger<Euresys::RemoteModule>("OffsetX", config.dimensions.x());
            setInteger<Euresys::RemoteModule>("OffsetY", config.dimensions.y());

            mImageFeature.width   = config.dimensions.width();
            mImageFeature.height  = config.dimensions.height();
            mImageFeature.offsetX = config.dimensions.x();
            mImageFeature.offsetY = config.dimensions.y();
        }

        break;
    }

    // set exposure time
    setFloat<Euresys::RemoteModule>("ExposureTime", exposureTimeUs);
    PRINT("  grabber", QString("ExposureTime           = %1").arg(exposureTimeUs));

    // binning
#ifdef REMOVED
    config.VBin;
/*
    setInteger<Euresys::RemoteModule>("BinningHorizontal", 1);
    setInteger<Euresys::RemoteModule>("BinningVertical", 1);
    setString<Euresys::RemoteModule>("BinningMode", "Average");
    // setString<Euresys::RemoteModule>("BinningMode", "Sum");
*/
#endif

#ifdef REMOVED

    config.dimensions; // QRect
/*
    int defWidth  = 7902;
    int defHeight = 6004;

    //int imageWidth  = defWidth / 2;
    //int imageHeight = defHeight / 2;
    int imageWidth  = defWidth;
    int imageHeight = defHeight;

    int xOffset = (defWidth - imageWidth) / 2;
    int yOffset = (defHeight - imageHeight) / 2;

    setInteger<Euresys::RemoteModule>("Width", imageWidth);
    setInteger<Euresys::RemoteModule>("Height", imageHeight);

    setInteger<Euresys::RemoteModule>("OffsetX", xOffset);
    setInteger<Euresys::RemoteModule>("OffsetY", yOffset);
*/

    config.bExtTrig;
    config.bTestPattern;
    config.trigDelayMs;
#endif

    // UpdateFrameSize();
}

void CoaXpressGrabber::UpdateFrameSize()
{
#ifdef NOT_VALID_SIZE
    int width        = getInteger<Euresys::RemoteModule>("Width");
    int height       = getInteger<Euresys::RemoteModule>("Height");
    int offsetX      = getInteger<Euresys::RemoteModule>("OffsetX");
    int offsetY      = getInteger<Euresys::RemoteModule>("OffsetY");
#endif
    std::string pixelFormat = getString<Euresys::RemoteModule>("PixelFormat");

#ifdef NOT_VALID_SIZE
    mImageFeature.width  = width;
    mImageFeature.height = height;
    mImageFeature.offsetX = offsetX;
    mImageFeature.offsetY = offsetY;
#else
    mImageFeature.width  = 0;
    mImageFeature.height = 0;
    mImageFeature.offsetX = 0;
    mImageFeature.offsetY = 0;
#endif

    if(pixelFormat == "Mono8")
    {
        mImageFeature.eFormat = PixelFormat_Mono8;
    }
    else if(pixelFormat == "Mono10")
    {
        mImageFeature.eFormat = PixelFormat_Mono10;
    }
    else if((pixelFormat == "Mono12") || (pixelFormat == "Mono12p"))
    {
        mImageFeature.eFormat = PixelFormat_Mono12;
    }
    else
    {
        mImageFeature.eFormat = PixelFormat_Unknown;
    }

    mImageFeature.eCameraManufacturer = mCameraManufacturer;

#ifdef NOT_VALID_SIZE
    CoaXpressFrame::SetFrameSize(mImageFeature);
#else
    // set frame size to sensor size
    ImageFeature imageFeature;

    imageFeature.width        = getInteger<Euresys::RemoteModule>("SensorWidth");
    imageFeature.height       = getInteger<Euresys::RemoteModule>("SensorHeight");
    imageFeature.offsetX      = 0;
    imageFeature.offsetY      = 0;

    imageFeature.eFormat             = mImageFeature.eFormat;
    imageFeature.eCameraManufacturer = mImageFeature.eCameraManufacturer;

    CoaXpressFrame::SetFrameSize(imageFeature);
#endif
}

void CoaXpressGrabber::PowerCycle()
{
    execute<Euresys::InterfaceModule>("CxpPoCxpTurnOff");
    //QThread::msleep(1000);
    QThread::msleep(5000);

    execute<Euresys::InterfaceModule>("CxpPoCxpAuto");
    //QThread::msleep(1000);

    bool bCameraStarted = false;
    int timeout = 0;
    std::string vendorName;
    std::string connectionId;

#define MAX_TIMEOUT 100
    do
    {
        QThread::msleep(1000);

        try
        {
            // retrieve state of the camera to check whether is restarted
            // it may happen that this function returns an exception
            connectionId = getString<Euresys::InterfaceModule>("CxpDeviceConnectionID");
        }
        catch(...)
        {
            connectionId = "NotReady";
        }

        if(connectionId.compare("NotReady") != 0)
        {
            bCameraStarted = true;
        }
        else
        {
            timeout ++;
        }
    } while((bCameraStarted == false) && (timeout < MAX_TIMEOUT));
}

void CoaXpressGrabber::SetConnectionConfig(std::string connection)
{
    std::string registerName;
    std::string currentConfig;

    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:
        registerName = "ConnectionConfig";
        break;

    case CameraManufacturer_CriticalLink:
        registerName = "CxpLinkConfiguration";
        break;

    default:
        break;
    }

    if(registerName.empty() != true)
    {
        // read current connection configuration and update it if required
        currentConfig = getString<Euresys::RemoteModule>(registerName);

        if(currentConfig != connection)
        {
            setString<Euresys::RemoteModule>(registerName, connection);
        }
    }
}

void CoaXpressGrabber::onNewBufferEvent(const NewBufferData &data)
{
#ifndef USE_POP
    ScopedBuffer buf(*this, data);

    ImageBuffer imageBuffer;

    imageBuffer.width   = mImageFeature.width;
    imageBuffer.height  = mImageFeature.height;
    imageBuffer.bufSize = buf.getInfo<size_t>(GenTL::BUFFER_INFO_SIZE);
    imageBuffer.ptr     = buf.getInfo<void*>(GenTL::BUFFER_INFO_BASE);

#ifndef COAXPRESS_FRAME_AVERAGE
    int frameIndex = CoaXpressFrame::StoreImage(imageBuffer);
#else
    CoaXpressFrame::AppendImage(mCurrentFrameIndex, imageBuffer);
#endif

    mAcquisitionCount --;

    if(mAcquisitionCount == 0)
    {
        stop();

#ifdef COAXPRESS_FRAME_AVERAGE
        emit FrameCaptured(mCurrentFrameIndex);
#endif
    }

    PRINT("  grabber", QString("acquisition done"));

#ifndef COAXPRESS_FRAME_AVERAGE
    emit FrameCaptured(frameIndex);
#endif

#endif
}

bool CoaXpressGrabber::Start(int acquisitionNumber)
{
    bool res = false;

#ifdef CAMERA_LOG_IN_FILE
    _LogInFile("Start");
#endif

    // use configuration
    if(acquisitionNumber == 0)
    {
        acquisitionNumber = mConfig.acquisitionNumber;
    }

    if(mAcquisitionCount == 0)
    {
        mAcquisitionCount = acquisitionNumber;

#ifdef COAXPRESS_FRAME_AVERAGE
        mCurrentFrameIndex = CoaXpressFrame::GetImageIndex();
#endif

#ifdef STD_DEV_FILE
        CoaXpressFrame::StoreStdDev(mCurrentFrameIndex, mConfig.bStoreStdDev);
#endif

#ifdef USE_POP
        reallocBuffers(acquisitionNumber);
        flushBuffers();
#endif

        start(acquisitionNumber);

#ifdef USE_POP
        uint64_t timeout = 100000;

        mAcquisitionCount = acquisitionNumber;

        do
        {
#ifdef CAMERA_LOG_IN_FILE
            _LogInFile("loop start");
#endif
            NewBufferData buffer(pop(timeout));

#ifdef CAMERA_LOG_IN_FILE
            _LogInFile("loop pop done");
#endif

            ScopedBuffer buf(*this, buffer);
#ifdef CAMERA_LOG_IN_FILE
            _LogInFile("loop got buffer");
#endif

            ImageBuffer imageBuffer;

            imageBuffer.width   = mImageFeature.width;
            imageBuffer.height  = mImageFeature.height;
            imageBuffer.bufSize = buf.getInfo<size_t>(GenTL::BUFFER_INFO_SIZE);
            imageBuffer.ptr     = buf.getInfo<void*>(GenTL::BUFFER_INFO_BASE);

#ifdef CAMERA_LOG_IN_FILE
            _LogInFile("loop AppendImage");
#endif

#ifndef COAXPRESS_FRAME_AVERAGE
            int frameIndex = CoaXpressFrame::StoreImage(imageBuffer);
#else
            CoaXpressFrame::AppendImage(mCurrentFrameIndex, imageBuffer);
#endif

            mAcquisitionCount --;
        } while (mAcquisitionCount != 0);

#ifdef CAMERA_LOG_IN_FILE
        _LogInFile("loop done");
        _LogInFile("stop");
#endif
        stop();

#ifdef CAMERA_LOG_IN_FILE
        _LogInFile("FrameCaptured");
#endif
        emit FrameCaptured(mCurrentFrameIndex);

        res = true;

        PRINT("  grabber", QString("acquisition done"));
#endif
    }
    else
    {
        PRINT("  grabber", QString("      not started"));
    }

    return res;
}

void CoaXpressGrabber::Stop()
{
    // use configuration
    stop();
}

#define CAMERA_SETTING_TEMP(a, b) CameraSettingItem("Temperature", a, b, "deg")
#define CAMERA_SETTING_SENSE(a, b) CameraSettingItem("Sense", a, b, "")

void CoaXpressGrabber::GetTemperature(CameraSettings &settings)
{
    settings.Clear();

    try
    {
        switch(mCameraManufacturer)
        {
        case CameraManufacturer_Adimec:
            settings.Append(CAMERA_SETTING_TEMP("Device", getFloat<Euresys::RemoteModule>("DeviceTemperature")));
            settings.Append(CAMERA_SETTING_TEMP("Sensor", getFloat<Euresys::RemoteModule>("SensorTemperature")));
            settings.Append(CAMERA_SETTING_TEMP("SensorChip", getFloat<Euresys::RemoteModule>("SensorChipTemperature")));

            break;

        case CameraManufacturer_CriticalLink:
            setString<Euresys::RemoteModule>("DeviceTemperatureSelector", "Sensor");
            QThread::msleep(10);
            settings.Append(CAMERA_SETTING_TEMP("Sensor", getFloat<Euresys::RemoteModule>("DeviceTemperature")));

            setString<Euresys::RemoteModule>("DeviceTemperatureSelector", "Mainboard");
            QThread::msleep(10);
            settings.Append(CAMERA_SETTING_TEMP("MainBoard", getFloat<Euresys::RemoteModule>("DeviceTemperature")));

            setString<Euresys::RemoteModule>("DeviceTemperatureSelector", "CPU");
            QThread::msleep(10);
            settings.Append(CAMERA_SETTING_TEMP("Cpu", getFloat<Euresys::RemoteModule>("DeviceTemperature")));

            // for debug add other measurements
            settings.Append(CameraSettingItem("Humidity", "", getFloat<Euresys::RemoteModule>("Humidity"), "%"));

            settings.Append(CAMERA_SETTING_SENSE("3_3V", getFloat<Euresys::RemoteModule>("Sensor_3_3V_Sense")));
            settings.Append(CAMERA_SETTING_SENSE("1_8V", getFloat<Euresys::RemoteModule>("Sensor_1_8V")));
            settings.Append(CAMERA_SETTING_SENSE("2_7V", getFloat<Euresys::RemoteModule>("Sensor_2_7V_Sense")));
            settings.Append(CAMERA_SETTING_SENSE("Array", getFloat<Euresys::RemoteModule>("Sensor_Array_Sense")));
            settings.Append(CAMERA_SETTING_SENSE("1_2V", getFloat<Euresys::RemoteModule>("Sensor_1_2V")));
            settings.Append(CAMERA_SETTING_SENSE("1_2C", getFloat<Euresys::RemoteModule>("Sensor_1_2C")));

            setString<Euresys::InterfaceModule>("CxpPoCxpHostConnectionSelector", "A");
            QThread::msleep(10);

            settings.Append(CameraSettingItem("CxpPo_A", "current", getFloat<Euresys::InterfaceModule>("CxpPoCxpCurrent"), "A"));
            settings.Append(CameraSettingItem("CxpPo_A", "voltage", getFloat<Euresys::InterfaceModule>("CxpPoCxpVoltage"), "V"));

            setString<Euresys::InterfaceModule>("CxpPoCxpHostConnectionSelector", "B");
            QThread::msleep(10);

            settings.Append(CameraSettingItem("CxpPo_B", "current", getFloat<Euresys::InterfaceModule>("CxpPoCxpCurrent"), "A"));
            settings.Append(CameraSettingItem("CxpPo_B", "voltage", getFloat<Euresys::InterfaceModule>("CxpPoCxpVoltage"), "V"));

            setString<Euresys::InterfaceModule>("CxpPoCxpHostConnectionSelector", "C");
            QThread::msleep(10);

            settings.Append(CameraSettingItem("CxpPo_C", "current", getFloat<Euresys::InterfaceModule>("CxpPoCxpCurrent"), "A"));
            settings.Append(CameraSettingItem("CxpPo_C", "voltage", getFloat<Euresys::InterfaceModule>("CxpPoCxpVoltage"), "V"));

            setString<Euresys::InterfaceModule>("CxpPoCxpHostConnectionSelector", "D");
            QThread::msleep(10);

            settings.Append(CameraSettingItem("CxpPo_D", "current", getFloat<Euresys::InterfaceModule>("CxpPoCxpCurrent"), "A"));
            settings.Append(CameraSettingItem("CxpPo_D", "voltage", getFloat<Euresys::InterfaceModule>("CxpPoCxpVoltage"), "V"));
            break;

        default:
            break;
        }
    }
    catch(...)
    {

    }
}

void CoaXpressGrabber::GetExposureTime(int& exposureTime)
{
    exposureTime = (int)getFloat<Euresys::RemoteModule>("ExposureTime");
}

void CoaXpressGrabber::GetInfo(GetInfo_t& infoData)
{
    std::string temp;

    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:
        infoData.serialNumber = mCameraInfo.cameraSerialNumber.c_str();
        temp  = getString<Euresys::RemoteModule>("PixelFormat");
        infoData.pixelFormat = temp.c_str();
        infoData.DefectPixelCorrectionEnable = getInteger<Euresys::RemoteModule>("DefectPixelCorrectionEnable");
        infoData.DF_ColumnOffsetCorrection = getInteger<Euresys::RemoteModule>("DF_ColumnOffsetCorrection");
        infoData.BF_ColumnGainCorrection = getInteger<Euresys::RemoteModule>("BF_ColumnGainCorrection");
        infoData.SensitivityMatching = getInteger<Euresys::RemoteModule>("SensitivityMatching");
        temp = getString<Euresys::RemoteModule>("ExposureMode");
        infoData.ExposureMode = temp.c_str();
        infoData.BlackLevel = getInteger<Euresys::RemoteModule>("BlackLevel");
        infoData.ReverseX = getInteger<Euresys::RemoteModule>("ReverseX");
        infoData.BinningHorizontal = getInteger<Euresys::RemoteModule>("BinningHorizontal");
        infoData.BinningVertical = getInteger<Euresys::RemoteModule>("BinningVertical");
        infoData.Width   = getInteger<Euresys::RemoteModule>("Width");
        infoData.Height  = getInteger<Euresys::RemoteModule>("Height");
        infoData.OffsetX = getInteger<Euresys::RemoteModule>("OffsetX");
        infoData.OffsetY = getInteger<Euresys::RemoteModule>("OffsetY");
        infoData.AcquisitionFramePeriod    = getInteger<Euresys::RemoteModule>("AcquisitionFramePeriod");
        infoData.AcquisitionFramePeriodRaw = getInteger<Euresys::RemoteModule>("AcquisitionFramePeriodRaw");
        infoData.ExposureTime    = getInteger<Euresys::RemoteModule>("ExposureTime");
        infoData.ExposureTimeRaw = getInteger<Euresys::RemoteModule>("ExposureTimeRaw");
        break;

    case CameraManufacturer_CriticalLink:
        infoData.serialNumber = mCameraInfo.cameraSerialNumber.c_str();
        temp  = getString<Euresys::RemoteModule>("PixelFormat");
        infoData.pixelFormat = temp.c_str();
        infoData.DefectPixelCorrectionEnable = 0;
        infoData.DF_ColumnOffsetCorrection = 0;
        infoData.BF_ColumnGainCorrection = 0;
        infoData.SensitivityMatching = 0;
        //temp = getString<Euresys::RemoteModule>("ExposureMode");
        temp = "NA";
        infoData.ExposureMode = temp.c_str();
        infoData.BlackLevel = 128;
        infoData.ReverseX = 0;
        infoData.BinningHorizontal = 1;
        infoData.BinningVertical = 1;

        infoData.Width   = getInteger<Euresys::RemoteModule>("Width");
        infoData.Height  = getInteger<Euresys::RemoteModule>("Height");
        infoData.OffsetX = getInteger<Euresys::RemoteModule>("OffsetX");
        infoData.OffsetY = getInteger<Euresys::RemoteModule>("OffsetY");

        infoData.AcquisitionFramePeriod    = getInteger<Euresys::RemoteModule>("MSRM_Int_AcquisitionFrameInterval");
        infoData.AcquisitionFramePeriodRaw = getInteger<Euresys::RemoteModule>("MSRM_Int_AcquisitionFrameInterval");
        infoData.ExposureTime    = getInteger<Euresys::RemoteModule>("ExposureTime");
        infoData.ExposureTimeRaw = getInteger<Euresys::RemoteModule>("ExposureTime");
        break;
    }
}

void CoaXpressGrabber::PrintConfig()
{
    std::string pixelFormat;

    CameraSettings settings;

    GetTemperature(settings);

    switch(mCameraManufacturer)
    {
    case CameraManufacturer_Adimec:
        pixelFormat = getString<Euresys::RemoteModule>("PixelFormat");
        break;

    case CameraManufacturer_CriticalLink:
        pixelFormat = "UNKNONW_pixel_format";
        break;
    }

    PRINT("  grabber", QString("Resolution  : %1 x %2").arg(mImageFeature.width).arg(mImageFeature.height));
    PRINT("  grabber", QString("PixelFormat : %1").arg(QString::fromStdString(pixelFormat)));

    for(int index = 0; index < settings.Length(); index ++)
    {
        CameraSettingItem *ptr = settings.Get(index);

        PRINT("  grabber", QString("%1 %2: %3")
              .arg(ptr->type)
              .arg(ptr->name)
              .arg(ptr->value));
    }
}

int CoaXpressGrabber::GetCurrentCaptureTimeoutSteps(int stepMs)
{
    // frame period is in us
    int timeout = (mCurrentAcquisitionFramePeriod * CAPTURE_TIMEOUT_FACTOR) / 1000;

    if(timeout > stepMs)
    {
        timeout = ((timeout + stepMs) / stepMs);
    }
    else
    {
        timeout = 1;
    }

    return timeout;
}

void CoaXpressGrabber::GetCameraInfo(CoaXpressGrabber_CameraInfo& info)
{
    info = mCameraInfo;
}

void CoaXpressGrabber::GetCameraSettings(CoaXpressGrabber_CameraSettings& settings)
{
    settings = mCameraSettings;
}

void CoaXpressGrabber::GetInterfaceInfo(CoaXpressGrabber_InterfaceInfo& info)
{
    info = mInterfaceInfo;
}

void CoaXpressGrabber::ErrorRecovery()
{
    std::string connectionConfig = getString<Euresys::RemoteModule>("ConnectionConfig");

    std::string connectionConfigReset = "CXP6_X1";
    setString<Euresys::RemoteModule>("ConnectionConfig", connectionConfigReset);

    setString<Euresys::RemoteModule>("ConnectionConfig", connectionConfig);
}

bool CoaXpressGrabber::IsFileTransferSupported()
{
    bool res = false;
    // this may depends on camera manufacturer

    if(mCameraManufacturer == CameraManufacturer_CriticalLink)
    {
        res = true;
    }

    return res;
}

void CoaXpressGrabber::FileTransferOpen()
{
    setString<Euresys::RemoteModule>("FileSelector", "CalibrationData");

    // perform a close if it is not the last executed command
    std::string currentOperating = getString<Euresys::RemoteModule>("FileOperationSelector");
    if(currentOperating != std::string("Close"))
    {
        setString<Euresys::RemoteModule>("FileOperationSelector", "Close");
        execute<Euresys::RemoteModule>("FileOperationExecute");
    }

    setString<Euresys::RemoteModule>("FileOperationSelector", "Open");
    setString<Euresys::RemoteModule>("FileOpenMode", "ReadWrite");

    execute<Euresys::RemoteModule>("FileOperationExecute");
}

void CoaXpressGrabber::FileTransferOpen(eFirmwareType eType)
{
    if(eType < eFirmwareType::unknown)
    {
        if(eType == eFirmwareType::FPGA)
        {
            setString<Euresys::RemoteModule>("FileSelector", "FPGAAppImg");
        }
        else if(eType == eFirmwareType::NIOS)
        {
            setString<Euresys::RemoteModule>("FileSelector", "NIOSAppImg");
        }

        setString<Euresys::RemoteModule>("FileOperationSelector", "Open");
        setString<Euresys::RemoteModule>("FileOpenMode", "ReadWrite");

        execute<Euresys::RemoteModule>("FileOperationExecute");
    }
}

void CoaXpressGrabber::FileTransferClose()
{
    setString<Euresys::RemoteModule>("FileOperationSelector", "Close");
    execute<Euresys::RemoteModule>("FileOperationExecute");
}

void CoaXpressGrabber::FileTransferWrite()
{
    setString<Euresys::RemoteModule>("FileOperationSelector", "Write");
    execute<Euresys::RemoteModule>("FileOperationExecute");
}

void CoaXpressGrabber::FileTransferRead()
{
    setString<Euresys::RemoteModule>("FileOperationSelector", "Read");
    execute<Euresys::RemoteModule>("FileOperationExecute");
}

//#define MSRM_FILE_ACCESS_BUFFER_ADDR 0x6068

void CoaXpressGrabber::FileTransferWrite(const void *data, size_t size)
{
    uint64_t address = 0;
    address = 0x6068;

    FileTransferWrite(address, data, size);
}

#define OFFSET_RETRY_MAX 10

void CoaXpressGrabber::FileTransferWrite(uint64_t address, const void *data, size_t size)
{
    //uint64_t address = MSRM_FILE_ACCESS_BUFFER_ADDR;
    char* ptr = (char*) data;

    bool bLastPart = false;

    size_t remaining = size;

    int loop = 0;

    int offset = 0;
    int offsetDelta = 0;
    int offsetRetry = 0;

    while(remaining > 0)
    {
        if(remaining > mFileTransferPacketSize)
        {
            size = mFileTransferPacketSize;
        }
        else
        {
            size = remaining;

            if((size % FILE_TRANSFER_PACKET_ALIGNMENT) != 0)
            {
                size += (FILE_TRANSFER_PACKET_ALIGNMENT - size % FILE_TRANSFER_PACKET_ALIGNMENT);
            }

            bLastPart = true;
        }

        offset = getInteger<Euresys::RemoteModule>("FileAccessOffset");
        offsetRetry = 0;

        do
        {
            gcWritePortData<Euresys::RemoteModule>(address, ptr, size);

            // check that the offset has changed as expected, else retry
            offsetDelta = getInteger<Euresys::RemoteModule>("FileAccessOffset");

            if(offsetDelta != (offset + size))
            {
                setInteger<Euresys::RemoteModule>("FileAccessOffset", offset);
                offsetRetry ++;
            }
            else
            {
                offsetRetry = 0;
            }

        } while((offsetRetry > 0) && (offsetRetry < OFFSET_RETRY_MAX));

        // there is an error
        if(offsetRetry != 0)
        {
            throw std::exception("Offset Error while writing");
        }

        // emit the progression update event according to granularity (to avoid signal drowning)
        loop ++;
        if(loop % mFileTransferProgressStepSize == 0)
        {
            emit ProgressUpdate(0);
        }

        if(bLastPart == true)
        {
            remaining = 0;
        }
        else
        {
            remaining -= size;
        }
        ptr += size;
    }
}

void CoaXpressGrabber::FileTransferRead(void *data, size_t size)
{
    uint64_t address = 0;
    address = 0x6068;

    FileTransferRead(address, data, size);
}

void CoaXpressGrabber::FileTransferRead(uint64_t address, void *data, size_t size)
{
    //uint64_t address = MSRM_FILE_ACCESS_BUFFER_ADDR;
    char* ptr = (char*) data;

    char* ptrCopy = nullptr;
    bool bLastPart = false;

    size_t remaining = size;

    int loop = 0;

    int offset = 0;
    int offsetDelta = 0;
    int offsetRetry = 0;

    while(remaining > 0)
    {
        ptrCopy = ptr;

        if(remaining > mFileTransferPacketSize)
        {
            size = mFileTransferPacketSize;
        }
        else
        {
            size = remaining;

            if((size % FILE_TRANSFER_PACKET_ALIGNMENT) != 0)
            {
                size += (FILE_TRANSFER_PACKET_ALIGNMENT - size % FILE_TRANSFER_PACKET_ALIGNMENT);

                // copy in a temp buffer
                bLastPart = true;
                ptrCopy = &mFileTransferBuffer[0];
            }
        }

        offset = getInteger<Euresys::RemoteModule>("FileAccessOffset");
        offsetRetry = 0;

        do
        {
            gcReadPortData<Euresys::RemoteModule>(address, ptrCopy, size);

            // check that the offset has changed as expected, else retry
            offsetDelta = getInteger<Euresys::RemoteModule>("FileAccessOffset");

            if(offsetDelta != (offset + size))
            {
                setInteger<Euresys::RemoteModule>("FileAccessOffset", offset);
                offsetRetry ++;
            }
            else
            {
                offsetRetry = 0;
            }

        } while((offsetRetry > 0) && (offsetRetry < OFFSET_RETRY_MAX));

        // there is an error
        if(offsetRetry != 0)
        {
            throw std::exception("Offset Error while reading");
        }

        // emit the progression update event according to granularity (to avoid signal drowning)
        loop ++;
        if(loop % mFileTransferProgressStepSize == 0)
        {
            emit ProgressUpdate(0);
        }

        // if data have been copied into temp buffer
        if(bLastPart == true)
        {
            memcpy(ptr, ptrCopy, remaining);

            remaining = 0;
        }
        else
        {
            remaining -= size;
            ptr += size;
        }
    }
}

int CoaXpressGrabber::FwUpdateConfigure(size_t size)
{
    // calculate the number of steps and the size of the step
    // in order to have less than 100 steps
    // so the application does not spend its time updating the interface
    mFileTransferProgressStepSize = 1;
    int nbSteps = ((int)size / mFileTransferPacketSize) + 1;

    if(nbSteps > 100)
    {
        mFileTransferProgressStepSize = nbSteps / 100;
        nbSteps = (nbSteps / mFileTransferProgressStepSize) + 1;
    }

    return nbSteps;
}

ClassCommon::Error CoaXpressGrabber::FwUpdate(eFirmwareType eType, const void *data, size_t size)
{
    ClassCommon::Error eError = ClassCommon::Error::Ok;

    if(eType < eFirmwareType::unknown)
    {
        emit ProgressUpdate(0);

        // disable the PID else fw update may fail
        setInteger<Euresys::RemoteModule>("PIDEnabled", 0);

        FileTransferOpen(eType);

        FileTransferWrite();

        uint64_t address = 0;
        address = 0x6068;

        FileTransferWrite(address, data, size);
        // FileTransferWrite(data, size);

        FileTransferRead();

        // allocate a buffer as big as the firmware
        QByteArray readData((int)size, 0);

        // FileTransferRead(readData.data(), size);
        FileTransferRead(address, readData.data(), size);

        if(memcmp(data, readData.data(), size) != 0)
        {
            eError = ClassCommon::Error::Failed;
        }

        FileTransferClose();
    }
    else
    {
        eError = ClassCommon::Error::InvalidParameter;
    }

    return eError;
}

void CoaXpressGrabber::I2CSelectDevice(unsigned char ucBUS, unsigned char ucSlaveAddress)
//----------------------------------------------------------------------------------
{
    mucBus = ucBUS;
    mucSlaveAddress = ucSlaveAddress ;
}

bool CoaXpressGrabber::I2CSendAndReceiveData(const unsigned char *TXBuffer, unsigned char TXCount, unsigned char *RXBuffer, unsigned char RXCount)
//------------------------------------------------------------------------------------------------------------------------------------------------
{
     // uint64_t   tx_address = 0x65F8;
     //uint64_t   rx_address = 0x6638;
     uint64_t tx_address = CoaXpressGrabber::mI2CAddressList[mI2CAddressIndex].tx;
     uint64_t rx_address = CoaXpressGrabber::mI2CAddressList[mI2CAddressIndex].rx;

     int        intResult ;
     char       rx_count ;

     try
     {
        setString<Euresys::RemoteModule>("I2CPasslock", "i2cexecute") ;
        setInteger<Euresys::RemoteModule>("I2CBusNum", mucBus) ;
        setInteger<Euresys::RemoteModule>("I2CSlaveAddr", mucSlaveAddress) ;
        gcWritePortData<Euresys::RemoteModule>(tx_address, (char *)TXBuffer, TXCount);

        setString<Euresys::RemoteModule>("I2CPasslock", "i2cexecute") ;

        setInteger<Euresys::RemoteModule>("I2CTxCount", TXCount) ;
        setInteger<Euresys::RemoteModule>("I2CRxCount", RXCount) ;

        execute<Euresys::RemoteModule>("I2CExecute") ;
        intResult =  getInteger<Euresys::RemoteModule>("I2COperationResult") ;

        if (intResult != 0)
            return (false) ;
        rx_count = getInteger<Euresys::RemoteModule>("I2CRxCount") ;
        gcReadPortData<Euresys::RemoteModule>(rx_address, (char *)RXBuffer, RXCount);

        return (true)     ;
     }
     catch(...)
     {
         return (false)     ;
     }
}

void CoaXpressGrabber::ChangeEepromModelName(QString modelName, QString serialNumber)
{
    std::string mDeviceVendorName       = getString<Euresys::RemoteModule>("DeviceVendorName");
    std::string mDeviceModelName        = getString<Euresys::RemoteModule>("DeviceModelName");
    std::string mDeviceManufacturerInfo = getString<Euresys::RemoteModule>("DeviceManufacturerInfo");
    std::string mDeviceVersion          = getString<Euresys::RemoteModule>("DeviceVersion");
    std::string mDeviceSerialNumber     = getString<Euresys::RemoteModule>("DeviceSerialNumber");

    mDeviceModelName = "";
    mDeviceModelName = modelName.toStdString();

    if(serialNumber.isEmpty() == false)
    {
        mDeviceSerialNumber = serialNumber.toStdString();
    }

    setString<Euresys::RemoteModule>("EEPROMVendorName", mDeviceVendorName);
    setString<Euresys::RemoteModule>("EEPROMModelName", mDeviceModelName);
    setString<Euresys::RemoteModule>("EEPROMManufacturerInfo", mDeviceManufacturerInfo);
    setString<Euresys::RemoteModule>("EEPROMVersion", mDeviceVersion);
    setString<Euresys::RemoteModule>("EEPROMSerialNumber", mDeviceSerialNumber);

    setString<Euresys::RemoteModule>("EEPROMWrite", "writeeeprom");
}

bool CoaXpressGrabber::HasCameraChanged()
{
    std::string cameraSerialNumber     = getString<Euresys::RemoteModule>("DeviceSerialNumber");
    return (cameraSerialNumber == mCameraInfo.cameraSerialNumber);
}

std::string CoaXpressGrabber::_LoadScriptStatement(QString fileName)
{
    std::string scriptStatement;

    QFile cameraConfigFile(fileName);
    if(cameraConfigFile.open(QFile::ReadOnly) == true)
    {
        QTextStream in(&cameraConfigFile);
        while (!in.atEnd())
        {
            QString line = in.readLine();

            scriptStatement = scriptStatement + line.toStdString() + "\n";
        }

        cameraConfigFile.close();
    }

    return scriptStatement;
}

void CoaXpressGrabber::_ReadCameraInfo()
{
    std::string deviceId               = getString<Euresys::DeviceModule>("DeviceID");
    std::string deviceVendorName       = getString<Euresys::DeviceModule>("DeviceVendorName");
    std::string deviceModelName        = getString<Euresys::DeviceModule>("DeviceModelName");

    std::string cameraVendorName       = getString<Euresys::RemoteModule>("DeviceVendorName");
    std::string cameraModelName        = getString<Euresys::RemoteModule>("DeviceModelName");
    std::string cameraManufacturerInfo = getString<Euresys::RemoteModule>("DeviceManufacturerInfo");
    std::string cameraVersion          = getString<Euresys::RemoteModule>("DeviceVersion");
    std::string cameraFirmwareVersion  = getString<Euresys::RemoteModule>("DeviceFirmwareVersion");
    std::string cameraSerialNumber     = getString<Euresys::RemoteModule>("DeviceSerialNumber");

    mCameraInfo.deviceId               = deviceId;
    mCameraInfo.deviceVendorName       = deviceVendorName;
    mCameraInfo.deviceModelName        = deviceModelName;

    mCameraInfo.cameraVendorName       = cameraVendorName;
    mCameraInfo.cameraModelName        = cameraModelName;
    mCameraInfo.cameraManufacturerInfo = cameraManufacturerInfo;
    mCameraInfo.cameraVersion          = cameraVersion;
    mCameraInfo.cameraFirmwareVersion  = cameraFirmwareVersion;
    mCameraInfo.cameraSerialNumber     = cameraSerialNumber;

    if(cameraVendorName.find(VENDOR_CRITICAL_LINK) != std::string::npos)
    {
        mCameraManufacturer = CameraManufacturer_CriticalLink;

        try
        {
            // camera version
            mCameraInfo.cameraSpecific.append(
                CoaXpressGrabber_CameraInfoItem ("NiosSwVersion", getString<Euresys::RemoteModule>("NiosSwVersion")));

            std::string niosBuildDate = getString<Euresys::RemoteModule>("NiosBuildDate");
            // remove hour from build date
            QStringList niosBuildDateList = QString::fromStdString(niosBuildDate).split(" ");
            niosBuildDate = QString("%1 %2 %3").arg(niosBuildDateList[0]).arg(niosBuildDateList[1]).arg(niosBuildDateList[2]).toStdString();

            mCameraInfo.cameraSpecific.append(
                CoaXpressGrabber_CameraInfoItem ("NiosBuildDate", niosBuildDate));
        }
        catch(...)
        {

        }
    }
    else if(cameraVendorName.find(VENDOR_ADIMEC) != std::string::npos)
    {
        mCameraManufacturer = CameraManufacturer_Adimec;
    }
    else
    {
        mCameraManufacturer = CameraManufacturer_Unknown;
    }
}

void CoaXpressGrabber::_LogInFile(QString message)
{
    emit LogInFile("[Grabber]", message);
}
