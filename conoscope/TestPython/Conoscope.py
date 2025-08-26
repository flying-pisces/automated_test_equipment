#!/usr/bin/env python
import threading
import ctypes
import time
from enum import Enum
import json

VERSION_REVISION = 53

class Conoscope:
    class Filter(Enum):
        BK7 = 0
        Mirror = 1
        X = 2
        Xz = 3
        Ya = 4
        Yb = 5
        Z = 6
        IrCut = 7
        Invalid = 8

    class Nd(Enum):
        Nd_0 = 0
        Nd_1 = 1
        Nd_2 = 2
        Nd_3 = 3
        Nd_4 = 4
        Invalid = 5

    class Iris(Enum):
        aperture_2mm = 0
        aperture_3mm = 1
        aperture_4mm = 2
        aperture_5mm = 3
        Invalid = 4

    class WheelState(Enum):
        WheelState_Idle = 0
        WheelState_Success = 1
        WheelState_Operating = 2
        WheelState_Error = 3

    class TemperatureMonitoringState(Enum):
        TemperatureMonitoringState_NotStarted = 0
        TemperatureMonitoringState_Processing = 1
        TemperatureMonitoringState_Locked = 2
        TemperatureMonitoringState_Aborted = 3
        TemperatureMonitoringState_Error = 4

    class CfgFileState(Enum):
        CfgFileState_NotDone = 0
        CfgFileState_Reading = 1
        CfgFileState_Writing = 2
        CfgFileState_ReadDone = 3
        CfgFileState_WriteDone = 4
        CfgFileState_ReadError = 5
        CfgFileState_WriteError = 6

    class CaptureSequenceState(Enum):
        CaptureSequenceState_NotStarted = 0
        CaptureSequenceState_Setup = 1
        CaptureSequenceState_WaitForTemp = 2
        CaptureSequenceState_AutoExpo = 3
        CaptureSequenceState_Measure = 4
        CaptureSequenceState_Process = 5
        CaptureSequenceState_Done = 6
        CaptureSequenceState_Error = 7
        CaptureSequenceState_Cancel = 8

    class MeasureAEState(Enum):
        MeasureAEState_NotStarted = 0
        MeasureAEState_Process = 1
        MeasureAEState_Done = 2
        MeasureAEState_Error = 3
        MeasureAEState_Cancel = 4

    class ExportFormat(Enum):
        Bin = 0
        BinJpg = 1

    class ApplicationThread(threading.Thread):
        def __init__(self, threadID, name, counter):
            threading.Thread.__init__(self)
            self.threadID = threadID
            self.name = name
            self.counter = counter

        def setApi(self, api):
            self.__api = api

        def run(self):
            try:
                self.__api.RunApplication()
            except:
                print("Error executing thread")

    class ConoscopeConfig(ctypes.Structure):
        _fields_ = [
            ("cfgPath", ctypes.c_char_p),
            ("capturePath", ctypes.c_char_p),
            ("fileNamePrepend", ctypes.c_char_p),
            ("fileNameAppend", ctypes.c_char_p),
            ("exportFileNameFormat", ctypes.c_char_p),
            ("exportFormat", ctypes.c_int),
            ("AEMinExpoTimeUs", ctypes.c_int),
            ("AEMaxExpoTimeUs", ctypes.c_int),
            ("AEExpoTimeGranularityUs", ctypes.c_int),
            ("AELevelPercent", ctypes.c_float),
            ("AEMeasAreaHeight", ctypes.c_int),
            ("AEMeasAreaWidth", ctypes.c_int),
            ("AEMeasAreaX", ctypes.c_int),
            ("AEMeasAreaY", ctypes.c_int),
            ("bUseRoi", ctypes.c_bool),
            ("RoiXLeft", ctypes.c_int),
            ("RoiXRight", ctypes.c_int),
            ("RoiYTop", ctypes.c_int),
            ("RoiYBottom", ctypes.c_int)]

    class ConoscopeDebugSettings(ctypes.Structure):
        _fields_ = [
            ("debugMode", ctypes.c_bool),
            ("emulatedCamera", ctypes.c_bool),
            ("dummyRawImagePath", ctypes.c_char_p),
            ("emulatedWheel", ctypes.c_bool),]

    class SetupConfig(ctypes.Structure):
        _fields_ = [
            ("sensorTemperature", ctypes.c_float),
            ("eFilter", ctypes.c_int),
            ("eNd", ctypes.c_int),
            ("eIris", ctypes.c_int)]

    class SetupStatus(ctypes.Structure):
        _fields_ = [
            ("eTemperatureMonitoringState", ctypes.c_int),
            ("sensorTemperature", ctypes.c_float),
            ("eWheelStatus", ctypes.c_int),
            ("eFilter", ctypes.c_int),
            ("eNd", ctypes.c_int),
            ("eIris", ctypes.c_int)]

    class MeasureConfig(ctypes.Structure):
        _fields_ = [
            ("exposureTimeUs", ctypes.c_int),
            ("nbAcquisition", ctypes.c_int),
            ("binningFactor", ctypes.c_int),
            ("bTestPattern", ctypes.c_bool)]

    class ProcessingConfig(ctypes.Structure):
        _fields_ = [
            ("bBiasCompensation", ctypes.c_bool),
            ("bSensorDefectCorrection", ctypes.c_bool),
            ("bSensorPrnuCorrection", ctypes.c_bool),
            ("bLinearisation", ctypes.c_bool),
            ("bFlatField", ctypes.c_bool),
            ("bAbsolute", ctypes.c_bool)]

    class CfgFileStatus(ctypes.Structure):
        _fields_ = [
            ("eState", ctypes.c_int),
            ("progress", ctypes.c_int),
            ("elapsedTime", ctypes.c_int64),
            ("fileName", ctypes.c_char_p)]
        
    class CaptureSequenceConfig(ctypes.Structure):
        _fields_ = [
            ("sensorTemperature", ctypes.c_float),
            ("bWaitForSensorTemperature", ctypes.c_bool),
            ("eNd", ctypes.c_int),
            ("eIris", ctypes.c_int),
            ("exposureTimeUs_FilterX", ctypes.c_int),
            ("exposureTimeUs_FilterXz", ctypes.c_int),
            ("exposureTimeUs_FilterYa", ctypes.c_int),
            ("exposureTimeUs_FilterYb", ctypes.c_int),
            ("exposureTimeUs_FilterZ", ctypes.c_int),
            ("nbAcquisition", ctypes.c_int),
            ("bAutoExposure", ctypes.c_bool),
            ("bUseExpoFile", ctypes.c_bool),
            ("bSaveCapture", ctypes.c_bool),
            ("bUseRoi", ctypes.c_bool),
            ("RoiXLeft", ctypes.c_int),
            ("RoiXRight", ctypes.c_int),
            ("RoiYTop", ctypes.c_int),
            ("RoiYBottom", ctypes.c_int)]
     
    class CaptureSequenceStatus(ctypes.Structure):
        _fields_ = [
            ("nbSteps", ctypes.c_int),
            ("currentSteps", ctypes.c_int),
            ("eFilter", ctypes.c_int),
            ("state", ctypes.c_int)]

    class MeasureAEStatus(ctypes.Structure):
        _fields_ = [
            ("exposureTimeUs", ctypes.c_int),
            ("nbAcquisition", ctypes.c_int),
            ("state", ctypes.c_int)]

    def __init__(self):
        print("create an instance of the conoscope")

        self.conoscopeConfig = Conoscope.ConoscopeConfig()
        self.conoscopeDebugSettings = Conoscope.ConoscopeDebugSettings()
        self.setupConfig = Conoscope.SetupConfig()
        self.measureConfig = Conoscope.MeasureConfig(200000, 1, 1, False)

        self.cfgFileStatus = Conoscope.CfgFileStatus()
        self.captureSequenceConfig = Conoscope.CaptureSequenceConfig()
        self.captureSequenceStatus = Conoscope.CaptureSequenceStatus()

        self.measureAEStatus = Conoscope.MeasureAEStatus()

        conoscopeDll = ctypes.WinDLL("ConoscopeLib.dll")

        # hllApiProto = ctypes.WINFUNCTYPE (
        #  ctypes.c_int,      # Return type.
        #  ctypes.c_void_p,   # Parameters 1 ...
        #  ctypes.c_void_p,
        #  ctypes.c_void_p,
        #  ctypes.c_void_p)   # ... thru 4.

        # hllApiParams = (1, "p1", 0), (1, "p2", 0), (1, "p3",0), (1, "p4",0),

        # Actually map the call ("HLLAPI(...)") to a Python name.
        # hllApi = hllApiProto (("HLLAPI", conoscopeDll), hllApiParams)

        # p1 = ctypes.c_int (1)
        # p2 = ctypes.c_char_p (sessionVar)
        # p3 = ctypes.c_int (1)
        # p4 = ctypes.c_int (0)
        # hllApi (ctypes.byref (p1), p2, ctypes.byref (p3), ctypes.byref (p4))

        #RunApplicationProto = ctypes.WINFUNCTYPE(
        #    ctypes.c_char_p)  # Return type.
        #self.__RunApplication = RunApplicationProto(("RunApplication", conoscopeDll)),

        CmdRunApp_Proto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdRunApp = CmdRunApp_Proto(("CmdRunApp", conoscopeDll))

        CmdQuitApp_Proto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdQuitApp = CmdQuitApp_Proto(("CmdQuitApp", conoscopeDll))

        CmdGetVersionProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdGetVersion = CmdGetVersionProto(("CmdGetVersion", conoscopeDll))

        CmdOpenProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdOpen = CmdOpenProto(("CmdOpen", conoscopeDll))

        CmdSetupProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdSetup = CmdSetupProto(("CmdSetup", conoscopeDll))

        CmdSetupStatusProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdSetupStatus = CmdSetupStatusProto(("CmdSetupStatus", conoscopeDll))

        CmdMeasureProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdMeasure = CmdMeasureProto(("CmdMeasure", conoscopeDll))

        CmdExportRawProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdExportRaw = CmdExportRawProto(("CmdExportRaw", conoscopeDll))

        CmdExportRawBufferProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdExportRawBuffer = CmdExportRawBufferProto(("CmdExportRawBuffer", conoscopeDll))

        CmdExportProcessedProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdExportProcessed = CmdExportProcessedProto(("CmdExportProcessed", conoscopeDll))

        CmdExportProcessedBufferProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p,
            ctypes.c_void_p)
        self.__CmdExportProcessedBuffer = CmdExportProcessedBufferProto(("CmdExportProcessedBuffer", conoscopeDll))

        CmdCloseProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdClose = CmdCloseProto(("CmdClose", conoscopeDll))

        CmdResetProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdReset = CmdResetProto(("CmdReset", conoscopeDll))

        CmdSetConfigProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdSetConfig = CmdSetConfigProto(("CmdSetConfig", conoscopeDll))

        CmdGetConfigProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdGetConfig = CmdGetConfigProto(("CmdGetConfig", conoscopeDll))

        CmdSetDebugConfigProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdSetDebugConfig = CmdSetDebugConfigProto(("CmdSetDebugConfig", conoscopeDll))

        CmdGetDebugConfigProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdGetDebugConfig = CmdGetDebugConfigProto(("CmdGetDebugConfig", conoscopeDll))

        CmdCfgFileWriteProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdCfgFileWrite = CmdCfgFileWriteProto(("CmdCfgFileWrite", conoscopeDll))

        CmdCfgFileReadProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdCfgFileRead = CmdCfgFileReadProto(("CmdCfgFileRead", conoscopeDll))
            
        CmdCfgFileStatusProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdCfgFileStatus = CmdCfgFileStatusProto(("CmdCfgFileStatus", conoscopeDll))

        CmdGetCaptureSequenceProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdGetCaptureSequence = CmdGetCaptureSequenceProto(("CmdGetCaptureSequence", conoscopeDll))
            
        CmdCaptureSequenceProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdCaptureSequence = CmdCaptureSequenceProto(("CmdCaptureSequence", conoscopeDll))
            
        CmdCaptureSequenceCancelProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdCaptureSequenceCancel = CmdCaptureSequenceCancelProto(("CmdCaptureSequenceCancel", conoscopeDll))
            
        CmdCaptureSequenceStatusProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdCaptureSequenceStatus = CmdCaptureSequenceStatusProto(("CmdCaptureSequenceStatus", conoscopeDll))

        CmdMeasureAEProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdMeasureAE = CmdMeasureAEProto(("CmdMeasureAE", conoscopeDll))

        CmdMeasureAECancelProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdMeasureAECancel = CmdMeasureAECancelProto(("CmdMeasureAECancel", conoscopeDll))

        CmdMeasureAEStatusProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p,  # Return type.
            ctypes.c_void_p)
        self.__CmdMeasureAEStatus = CmdMeasureAEStatusProto(("CmdMeasureAEStatus", conoscopeDll))

        CmdTerminateProto = ctypes.WINFUNCTYPE(
            ctypes.c_char_p)  # Return type.
        self.__CmdTerminate = CmdTerminateProto(("CmdTerminate", conoscopeDll))

        # create a thread to execute application
        try:
            self.__applicationThread = Conoscope.ApplicationThread(1, "AppThread", 1)
            self.__applicationThread.setApi(self)
            self.__applicationThread.start()

            # magic sleep
            time.sleep(0.5)
        except:
            errorMessage = "Error: unable to start thread"
            print(errorMessage)
            raise Exception(errorMessage)

        # check version of the dll matches with the version of this file
        ret = self.__CmdGetVersion()
        version = json.loads(ret)

        libNameKey = 'Lib_Name'
        libVersionKey = 'Lib_Version'

        libName = 'CONOSCOPE_LIB'
        #versionMajor = 0
        #versionMinor = 8
        #versionRevision = 27

        if (libNameKey in version) and (version[libNameKey] == libName):
            if libVersionKey in version:
                versionNumber = version[libVersionKey].split('.')

                versionMajorLib = int(versionNumber[0])
                versionMinorLib = int(versionNumber[1])
                versionRevisionLib = int(versionNumber[2])

                if VERSION_REVISION != versionRevisionLib:
                    errorMessage = "Error: invalid revision {0}.{1}.{2} != {3}".format(
                        versionMajorLib, versionMinorLib, versionRevisionLib, VERSION_REVISION)
                    print(errorMessage)
                    raise Exception(errorMessage)
            else:
                errorMessage = "Error: unknown version"
                print(errorMessage)
                raise Exception(errorMessage)
        else:
            errorMessage = "Error: invalid lib"
            print(errorMessage)
            raise Exception(errorMessage)

        # configure the conoscope to default value
        ret = self.CmdGetDebugConfig()

        # configure conoscope in normal mode
        ret = self.CmdSetDebugConfig({"debugMode": False,
                                      "emulatedCamera": False,
                                      "emulatedWheel": False})
        print("instance done")

    def __del__(self):
        print("CONOSCOPE INSTANCE BEING DELETED")

    def __Result(ret):
        return json.loads(ret)

    def __FillStructure(structure, config):
        for field_name, field_type in structure._fields_:
            # print("{0}".format(field_name))

            if field_name in config:
                field_value = config[field_name]

                try:
                    # convert string
                    b_field_value = field_value.encode('utf-8')
                    field_value = b_field_value
                except:
                    # this is not a string
                    pass

                setattr(structure, field_name, field_value)

    def RunApplication(self):
        return self.__CmdRunApp()

    def QuitApplication(self):
        # terminate the dll
        self.__CmdTerminate()

        # stop the thread
        self.__CmdQuitApp()

        # wait for the thread to be finished
        self.__applicationThread.join()

    def CmdGetVersion(self):
        return Conoscope.__Result(self.__CmdGetVersion())

    def CmdGetInfo(self):
        return Conoscope.__Result(self.__CmdGetInfo())

    def CmdOpen(self):
        return Conoscope.__Result(self.__CmdOpen())

    def CmdSetup(self, config):
        Conoscope.__FillStructure(self.setupConfig, config)
        return Conoscope.__Result(self.__CmdSetup(ctypes.byref(self.setupConfig)))

    def CmdSetupStatus(self):
        setupStatus = Conoscope.SetupStatus()

        ret = self.__CmdSetupStatus(ctypes.byref(setupStatus))

        returnVal = Conoscope.__Result(ret)

        # update return value with setup status
        returnVal["eTemperatureMonitoringState"] = Conoscope.TemperatureMonitoringState(setupStatus.eTemperatureMonitoringState)
        returnVal["sensorTemperature"] = setupStatus.sensorTemperature
        returnVal["eWheelState"] = Conoscope.WheelState(setupStatus.eWheelStatus)
        returnVal["eFilter"] = Conoscope.Filter(setupStatus.eFilter)
        returnVal["eNd"] = Conoscope.Nd(setupStatus.eNd)
        returnVal["eIris"] = Conoscope.Iris(setupStatus.eIris)

        return returnVal

    def CmdMeasure(self, config):
        Conoscope.__FillStructure(self.measureConfig, config)
        return Conoscope.__Result(self.__CmdMeasure(ctypes.byref(self.measureConfig)))

    def CmdExportRaw(self):
        return Conoscope.__Result(self.__CmdExportRaw())

    def CmdExportRawBuffer(self):
        print("not implemented correctly")
        return Conoscope.__Result(self.__CmdExportRawBuffer())

    def CmdExportProcessed(self, config={"bBiasCompensation": True,
                                         "bSensorDefectCorrection": True,
                                         "bSensorPrnuCorrection": True,
                                         "bLinearisation": True,
                                         "bFlatField": True,
                                         "bAbsolute": True}):
        processingConfig = Conoscope.ProcessingConfig(False, False, False, False, False, False)
        Conoscope.__FillStructure(processingConfig, config)
        return Conoscope.__Result(self.__CmdExportProcessed(ctypes.byref(processingConfig)))

    def CmdExportProcessedBuffer(self, config={"bBiasCompensation": True,
                                               "bSensorDefectCorrection": True,
                                               "bSensorPrnuCorrection": True,
                                               "bLinearisation": True,
                                               "bFlatField": True,
                                               "bAbsolute": True}):
        processingConfig = Conoscope.ProcessingConfig(False, False, False, False, False, False)
        Conoscope.__FillStructure(processingConfig, config)
        return Conoscope.__Result(self.__CmdExportProcessedBuffer(ctypes.byref(processingConfig)))

    def CmdClose(self):
        return Conoscope.__Result(self.__CmdClose())

    def CmdReset(self):
        return Conoscope.__Result(self.__CmdReset())

    def CmdSetConfig(self, config):
        Conoscope.__FillStructure(self.conoscopeConfig, config)
        ret = Conoscope.__Result(self.__CmdSetConfig(ctypes.byref(self.conoscopeConfig)))
        return ret

    def CmdGetConfig(self):
        return Conoscope.__Result(self.__CmdGetConfig(ctypes.byref(self.conoscopeConfig)))

    def CmdSetDebugConfig(self, config):
        Conoscope.__FillStructure(self.conoscopeDebugSettings, config)
        ret = Conoscope.__Result(self.__CmdSetDebugConfig(ctypes.byref(self.conoscopeDebugSettings)))
        return ret

    def CmdGetDebugConfig(self):
        ret = Conoscope.__Result(self.__CmdGetDebugConfig(ctypes.byref(self.conoscopeDebugSettings)))
        return ret

    #def CmdCfgFileWrite(self):
    #    ret = Conoscope.__Result(self.__CmdCfgFileWrite())

    def CmdCfgFileRead(self):
        ret = Conoscope.__Result(self.__CmdCfgFileRead())
        return ret
            
    def CmdCfgFileStatus(self):
        ret = Conoscope.__Result(self.__CmdCfgFileStatus(ctypes.byref(self.cfgFileStatus)))
        return ret

    def CmdGetCaptureSequence(self):
        ret = Conoscope.__Result(self.__CmdGetCaptureSequence(ctypes.byref(self.captureSequenceConfig)))
        return ret

    def CmdCaptureSequence(self, config):
        Conoscope.__FillStructure(self.captureSequenceConfig, config)
        ret = Conoscope.__Result(self.__CmdCaptureSequence(ctypes.byref(self.captureSequenceConfig)))
        return ret

    def CmdCaptureSequenceCancel(self):
        ret = Conoscope.__Result(self.__CmdCaptureSequenceCancel())
        return ret

    def CmdCaptureSequenceStatus(self):
        status = Conoscope.CaptureSequenceStatus()
        ret = self.__CmdCaptureSequenceStatus(ctypes.byref(status))

        self.captureSequenceStatus = status

        returnVal = Conoscope.__Result(ret)

        # update return value with setup status
        returnVal["nbSteps"] = status.nbSteps
        returnVal["currentSteps"] = status.currentSteps
        returnVal["eFilter"] = Conoscope.Filter(status.eFilter)
        returnVal["state"] = Conoscope.CaptureSequenceState(status.state)

        return returnVal

    def CmdMeasureAE(self, config):
        Conoscope.__FillStructure(self.measureConfig, config)
        return Conoscope.__Result(self.__CmdMeasureAE(ctypes.byref(self.measureConfig)))

    def CmdMeasureAECancel(self):
        return Conoscope.__Result(self.__CmdMeasureAECancel())

    def CmdMeasureAEStatus(self):
        status = Conoscope.MeasureAEStatus()
        ret = self.__CmdMeasureAEStatus(ctypes.byref(status))

        self.measureAEStatus = status

        returnVal = Conoscope.__Result(ret)

        # update return value with setup status
        returnVal["exposureTimeUs"] = status.exposureTimeUs
        returnVal["nbAcquisition"] = status.nbAcquisition
        returnVal["state"] = Conoscope.MeasureAEState(status.state)

        return returnVal
