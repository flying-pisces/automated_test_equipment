#!/usr/bin/env python
import json
from Conoscope import Conoscope
import time
import os

def LogFunction(ret, functionName):
    print("  {0}".format(functionName))

    try:
        # case of return value is a string
        # print("  -> {0}".format(ret))
        returnData = json.loads(ret)
    except:
        # case of return is already a dict
        returnData = ret

    try:
        # display return data
        for key, value in returnData.items():
            print("      {0:<20}: {1}".format(key, value))
    except:
        print("  invalid return value")

class ProcessEngine:

    def __init__(self, cfgPath):
        # create the conoscope instance
        self.conoscope = Conoscope()

        ret = self.conoscope.CmdGetDebugConfig()
        LogFunction(ret, "CmdSetDebugConfig")

        # configure conoscope in emulated mode
        ret = self.conoscope.CmdSetDebugConfig({"debugMode": False,
                                                "emulatedCamera": True})
        LogFunction(ret, "CmdSetDebugConfig")

        # retrieve current configuration
        ret = self.conoscope.CmdGetConfig()
        LogFunction(ret, "CmdSetConfig")

        ret = self.conoscope.CmdSetConfig({"cfgPath": cfgPath})
        LogFunction(ret, "CmdSetConfig")

        ret = self.conoscope.CmdOpen()
        LogFunction(ret, "CmdOpen")

        # this is no use in this configuration
        ret = self.conoscope.CmdSetup({"sensorTemperature": 25.0,
                                  "eFilter": Conoscope.Filter.X.value,
                                  "eNd": Conoscope.Nd.Nd_0.value,
                                  "eIris": Conoscope.Iris.aperture_2mm.value})
        LogFunction(ret, "CmdSetup")

    def __del__(self):
        print("del the instance")
        self.close()

    def close(self):
        ret = self.conoscope.CmdClose()
        LogFunction(ret, "CmdClose")

        # end the application
        self.conoscope.QuitApplication()

        del(self.conoscope)

    def processFolderList(self, folderList, inputFolder=None, outputFolder="proc", processConfig={"bBiasCompensation": True,
                                                                                "bSensorDefectCorrection": True,
                                                                                "bSensorPrnuCorrection": True,
                                                                                "bLinearisation": True,
                                                                                "bFlatField": True,
                                                                                "bAbsolute": True}):
        for folderName in folderList:
            self.processFolder(folderName, inputFolder, outputFolder, processConfig)

    def processFolder(self, folderName, inputFolder, outputFolder, processConfig):
        if inputFolder is not None:
            inputFolderName = "{0}\\{1}".format(inputFolder, folderName)
        else:
            inputFolderName = folderName

        fileList = os.listdir(inputFolderName)

        if inputFolder is not None:
            processFolderName = "{0}\\{1}".format(outputFolder, folderName)
        else:
            processFolderName = "{0}\\{1}".format(folderName, outputFolder)

        print("input  folder: {0}".format(inputFolderName))
        print("output folder: {0}".format(processFolderName))

        ret = self.conoscope.CmdSetConfig({"capturePath": processFolderName})
        LogFunction(ret, "CmdSetConfig")

        for fileName in fileList:
            if fileName.endswith('.bin'):
                self.processCapture(fileName, inputFolderName, processConfig)

    def processCapture(self, fileName, folderName, processConfig):
        imagePath = "{0}\\{1}".format(folderName, fileName)

        print("   {0} [{1}]".format(imagePath, fileName))

        # set the image to process
        ret = self.conoscope.CmdSetDebugConfig({"dummyRawImagePath": imagePath})
        LogFunction(ret, "CmdSetDebugConfig")

        # retrieve raw data
        ret = self.conoscope.CmdMeasure({"exposureTimeUs": 100000,
                                         "nbAcquisition": 1})
        LogFunction(ret, "CmdMeasure")

        #ret = self.conoscope.CmdExportRaw()
        #LogFunction(ret, "CmdExportRaw")

        ret = self.conoscope.CmdExportProcessed(processConfig)
        LogFunction(ret, "CmdExportProcessed")

