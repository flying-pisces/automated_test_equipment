#!/usr/bin/env python
from ProcessEngine import ProcessEngine

engine = ProcessEngine('.\\Cfg')

folderList = ['.\\CaptureImage']

engine.processFolderList(folderList, outputFolder="pyproc")

del(engine)

print("Done")



