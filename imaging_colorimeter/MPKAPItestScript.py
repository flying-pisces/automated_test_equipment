import clr
import os
import json
import pprint
import datetime
import random

def printFormatted(s):
    pprint.pprint(json.loads(s))

clr.AddReference("System")
import System

apiPath = os.path.join(os.getcwd(), "MPK_API.dll")
#apiPath = os.path.join(r"Z:\MLO\bin\Debug\MPK_API.dll")

clr.AddReference(apiPath)
from MPK_API_CS import *

class MPKtest():
    def __init__(self, verbose = True):
        self._verbose = verbose
        self._device = MPK_API()

    ########################
    # INITIALIZATION
    ########################
    def initializeCamera(self, sn):
        result = self._device.InitializeCamera(sn, False, False)
        if self._verbose:
            printFormatted(result)
        return result

    def setDatabase(self, databasePath):
        result = self._device.SetMeasurementDatabase(databasePath)
        if self._verbose:
            printFormatted(result)
        return result

    def createDatabase(self, databasePath):
        result = self._device.CreateMeasurementDatabase(databasePath)
        if self._verbose:
            printFormatted(result)
        return result

    def setSequence(self, sequencePath):
        result = self._device.SetSequence(sequencePath)
        if self._verbose:
            printFormatted(result)
        return result

    ########################
    # MEASUREMENT OPERATIONS
    ########################
    def getPatternList(self):
        result = json.loads(self._device.GetListOfPatternSetups())
        if self._verbose:
            pprint.pprint(result)
        return result

    def getMeasurementList(self):
        result = self._device.GetMeasurementList()
        if self._verbose:
            printFormatted(result)
        return result

    def getNameOfFirstMeasurement(self):
        result = self._device.GetMeasurementList()
        measurementDictList = json.loads(result)
        return measurementDictList[0]['Description']

    def getIDofFirstMeasurement(self):
        result = self._device.GetMeasurementList()
        measurementTuple = json.loads(result)
        return measurementTuple[0]['Measurement ID']

    def getMeasurementInfo(self, imageName):
        result = self._device.GetMeasurementInfo(imageName)
        if self._verbose:
            printFormatted(result)
        return result

    def editMeasurementInfo(self, imageName, jsonMeasInfo):
        result = self._device.EditMeasurementInfo(imageName, jsonMeasInfo)
        if self._verbose:
            printFormatted(result)
        return result

    def exportMeasurement(self, imageName, exportPath, exportFileName, resX = 0, resY = 0):
        result = self._device.ExportMeasurement(imageName, exportPath, exportFileName, resX, resY)
        if self._verbose:
            print(result)
            # printFormatted(result)
        return result

    ########################
    # SEQUENCING
    ########################
    def sequenceRunAll(self, patternName, useCamera = True, saveImages = True):
        result = self._device.RunAllSequenceSteps(patternName, useCamera, saveImages)
        if self._verbose:
            printFormatted(result)
        return result

    # def sequenceRunAllWithCamera(self, useCamera, saveImages):
    #     result = self._device.RunAllSequenceSteps(useCamera, saveImages)
    #     if self._verbose:
    #         printFormatted(result)

    def sequenceRunStep(self, step, patternName = "", useCamera = True, saveImages = True):
        result = self._device.RunSequenceStepByName(step, patternName, useCamera, saveImages)
        if self._verbose:
            printFormatted(result)
            #printFormatted(result)
        return result

    def sequenceRunStepList(self, stepList, patternName, useCamera = True, saveImages = True):
        result = self._device.RunSequenceStepListByName(stepList, patternName, useCamera, saveImages)
        if self._verbose:
            printFormatted(result)
            # printFormatted(result)
        return result

if __name__ == "__main__":
    verbose = True

    testSequencing = False
    testMeasurementOperations = False
    testMeasurementInfoEdit = False
    testExportPNG = True
    testExportConoscope = True

    device = MPKtest(verbose)
    databasePath = r"C:\Radiant Vision Systems Data\TrueTest\UserData\Demo.ttxm"
    # databasePath = r"C:\Radiant Vision Systems Data\TrueTest\UserData\\" + str(random.randint(0, 1000)) + ".ttxm"


    sequencePath = r"C:\Radiant Vision Systems Data\TrueTest\UserData\Demo Sequence.seqx"

    device.initializeCamera("Demo")
    device.setDatabase(databasePath)
    # device.createDatabase()
    device.setSequence(sequencePath)

    if testSequencing:
        useCamera = True
        saveMeasurements = True
        # device.sequenceRunAll()
        # device.sequenceRunAllWithCamera(useCamera, saveMeasurements)
        pattern = device.getPatternList()[0]
        device.sequenceRunStep("Particle Defects", pattern)

        # device.sequenceRunStepList(["Gradient", "ANSI Brightness"])

    if testMeasurementOperations:
        device.getMeasurementList()
        ID = device.getIDofFirstMeasurement()
        device.getMeasurementInfo(ID)

    if testMeasurementInfoEdit:
        measurementInfoDict = {
            "Description" : "New Description",
            "Model Number" : "New Model Number",
            "Technician" : "New Technician",
            "Pattern" : "New Pattern",
            "Measurement Setup" : "New Measurement Setup"
        }

        device.getMeasurementList()
        ID = device.getIDOfFirstMeasurement()
        device.getMeasurementInfo(ID)

        device.editMeasurementInfo(ID, json.dumps(measurementInfoDict))
        device.getMeasurementInfo(ID)

    if testExportPNG:
        measList = json.loads(device.getMeasurementList())
        exportPath = r"C:\Radiant Vision Systems Data\TrueTest\UserData\ExportTest\\"
        for meas in measList:
            exportName = meas["Description"] + "_" + meas["Measurement Setup"] + ".png"
            measID = meas["Measurement ID"]
            device.exportMeasurement(measID, exportPath, exportName)
        # name = device.getNameOfFirstMeasurement()
        # device._exportPath = r"C:\Radiant Vision Systems Data\TrueTest\UserData"
        # device._exportFileName = r"ExportTest.csv"
        # device.exportMeasurementAndResize(name, 100, 100)
        # device._exportFileName = r"ExportTest.png"
        # device.exportMeasurement(name)
        # device._exportFileName = r"ExportTest.jpg"
        # device.exportMeasurement(name)
        # device._exportFileName = r"ExportTest.bmp"
        # device.exportMeasurement(name)
        # device._exportFileName = r"ExportTest1"
        # device.exportMeasurement(name)
        # device._exportFileName = ""
        # device.exportMeasurement(name)

    if testExportConoscope:
        databasePath = r"C:\Radiant Vision Systems Data\TrueTest\UserData\ConoscopeTest.ttxm"
        device.setDatabase(databasePath)
        measList = json.loads(device.getMeasurementList())
        exportPath = r"C:\Radiant Vision Systems Data\TrueTest\UserData\ExportTest\\"
        resX = 100
        resY = 100
        for meas in measList:
            exportName = meas["Description"] + ".csv"
            measID = meas["Measurement ID"]
            device.exportMeasurement(measID, exportPath, exportName, resX, resY)
