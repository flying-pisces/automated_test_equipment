#-------------------------------------------------
#
# Project created by QtCreator 2019-11-19T15:17:51
#
#-------------------------------------------------

QT       -= gui
QT       += core xml widgets

# remove warnings from
QMAKE_CXXFLAGS_WARN_ON = -wd4100

TARGET = ConoscopeLib
TEMPLATE = lib

DEFINES += CONOSCOPELIB_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#DEFINES += ENABLE_DEBUG_INTERFACE

DEFINES += AE_MEAS_AREA
# DEFINES += QUIET_MODE
DEFINES += MULTITHREAD_CAPTURE_SEQUENCE
DEFINES += CAPTURE_SEQUENCE_ORDER
DEFINES += CAPTURE_SETTINGS
# DEFINES += CAPTURE_SETTINGS_DEBUG
DEFINES += APP_HELPER

DEFINES += CHECK_WHEEL_INTEGRITY

DEFINES += DEBUG_WHEEL_ERROR

SOURCES += \
    ConoscopeApp/CaptureSequenceThread.cpp \
    ConoscopeApp/ConoscopeAppHelper.cpp \
        ConoscopeLib.cpp \
    Camera/cameraCmvCxp.cpp \
    Camera/camera.cpp \
    CoaXPress/CoaXpressController.cpp \
    CoaXPress/CoaXpressFrame.cpp \
    CoaXPress/CoaXpressGrabber.cpp \
    CoaXPress/CoaXpressTypes.cpp \
    Shared/imageConfiguration.cpp \
    Tools/classcommon.cpp \
    Tools/toolString.cpp \
    Tools/toolTypes.cpp \
    Conoscope/Conoscope.cpp \
    Conoscope/ConoscopeWorker.cpp \
    Conoscope/ConoscopeProcess.cpp \
    Conoscope/ConoscopeConfig.cpp \
    Cfg/CfgHelper.cpp \
    Pipeline/PipelineLib.cpp \
    EldimDevices/CDevices.cpp \
    EldimDevices/CEXI2Message.cpp \
    EldimDevices/CISGWMotor.cpp \
    EldimDevices/CISGWSensor.cpp \
    Conoscope/TempMonitoringWorker.cpp \
    Conoscope/TempMonitoring.cpp \
    Tools/toolReturnCode.cpp \
    Camera/cameraDummy.cpp \
    Conoscope/ConoscopeResource.cpp \
    ConoscopeApp/ConoscopeApp.cpp \
    ConoscopeApp/ConoscopeAppProcess.cpp \
    ConoscopeApp/ConoscopeAppWorker.cpp

HEADERS += \
        ConoscopeApp/CaptureSequenceThread.h \
        ConoscopeApp/ConoscopeAppHelper.h \
        ConoscopeLib.h \
        ConoscopeLib_global.h \ 
    Camera/camera.h \
    Camera/cameraCmvCxp.h \
    Camera/cameraCmvCxpHw.h \
    Camera/HwTool.h \
    CoaXPress/CoaXpressConfiguration.h \
    CoaXPress/CoaXpressController.h \
    CoaXPress/CoaXpressFrame.h \
    CoaXPress/CoaXpressGrabber.h \
    CoaXPress/CoaXpressTypes.h \
    Shared/imageConfiguration.h \
    Shared/imageConfigurationConst.h \
    Tools/classcommon.h \
    Tools/toolString.h \
    Tools/toolTypes.h \
    Conoscope/Conoscope.h \
    configuration.h \
    Conoscope/ConoscopeWorker.h \
    Conoscope/ConoscopeWorker.h \
    Conoscope/ConoscopeProcess.h \
    Conoscope/conoscopeTypes.h \
    Conoscope/ConoscopeConfig.h \
    Cfg/CfgHelper.h \
    Pipeline/PipelineLib.h \
    Tools/Types.h \
    Pipeline/PipelineTypes.h \
    Pipeline/Types.h \
    EldimDevices/CDevices.h \
    EldimDevices/CEXI2Message.h \
    EldimDevices/CISGWMotor.h \
    EldimDevices/CISGWSensor.h \
    Conoscope/TempMonitoringWorker.h \
    Conoscope/TempMonitoring.h \
    Tools/toolReturnCode.h \
    Camera/cameraDummy.h \
    Conoscope/ConoscopeResource.h \
    Tools/logger.h \
    ConoscopeApp/ConoscopeApp.h \
    ConoscopeApp/ConoscopeAppProcess.h \
    ConoscopeApp/ConoscopeAppWorker.h \
    Conoscope/ConoscopeStaticTypes.h

INCLUDEPATH += './Camera'
INCLUDEPATH += './CoaXPress'
INCLUDEPATH += './Tools'
INCLUDEPATH += './Shared'
INCLUDEPATH += './Conoscope'
INCLUDEPATH += './ConoscopeApp'
INCLUDEPATH += './Cfg'
INCLUDEPATH += './Pipeline'
INCLUDEPATH += './EldimDevices'

unix {
    target.path = /usr/lib
    INSTALLS += target
}

# frame grabber
INCLUDEPATH += 'C:/Program Files/Euresys/Coaxlink/include'
DEPENDPATH += 'C:/Program Files/Euresys/Coaxlink/include'

# pipeline library
win32:CONFIG(release, debug|release): LIBS += -L$$PWD\\..\\build-PipelineLib-Desktop_Qt_5_14_2_MSVC2015_64bit-Release\\release -lPipelineLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD\\..\\build-PipelineLib-Desktop_Qt_5_14_2_MSVC2015_64bit-Debug\\debug -lPipelineLib

INCLUDEPATH += $$PWD\\ConoscopePipeline
DEPENDPATH += $$PWD\\ConoscopePipeline

