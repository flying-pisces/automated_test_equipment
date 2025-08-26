#-------------------------------------------------
#
# Project created by QtCreator 2019-11-15T15:07:17
#
#-------------------------------------------------

#QT       += core gui
QT       += core gui xml widgets axcontainer network serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_FILE = resource\resources.rc

TARGET = ConoscopeTestApp
TEMPLATE = app

CONFIG += conoscopedll

DEFINES += TEST_SHIFT

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

conoscopedll {
    DEFINES += CONOSCOPE_DLL
} 

# DEFINES += ENABLE_DEBUG_INTERFACE

SOURCES += \
    main.cpp \
    Forms/mainwindow.cpp \
    Tools/classcommon.cpp \
    Shared/imageConfiguration.cpp \
    Tools/toolString.cpp \
    Tools/toolTypes.cpp \
    AppController/appController.cpp \
    AppController/appControllerWorker.cpp \
    AppController/appResource.cpp \
    Conoscope/conoscopeApi.cpp \
    Forms/formCmdParam.cpp \
    Forms/formCmd.cpp \
    Tools/toolReturnCode.cpp \
    Forms/DialogStream.cpp \
    Stream/CLineItem.cpp \
    Stream/qEldViewer.cpp \
    Stream/FrameBuffer.cpp

#conoscopedll
#{
#    SOURCES += \
#        Conoscope/conoscopeLib.cpp
#}

#!conoscopedll
#{
#    SOURCES += \
#        Camera/camera.cpp \
#        Camera/cameraCmvCxp.cpp \
#        CoaXPress/CoaXpressController.cpp \
#        CoaXPress/CoaXpressFrame.cpp \
#        CoaXPress/CoaXpressGrabber.cpp \
#        CoaXPress/CoaXpressTypes.cpp \
#        Conoscope/conoscope.cpp
#}

HEADERS += \
    Forms/mainwindow.h \
    Tools/classcommon.h \
    Shared/imageConfiguration.h \
    Shared/imageConfigurationConst.h \
    Tools/toolString.h \
    Tools/toolTypes.h \
    AppController/appController.h \
    AppController/appControllerWorker.h \
    AppController/appResource.h \
    Conoscope/conoscopeApi.h \
    configuration.h \
    Conoscope/conoscopeTypes.h \
    Forms/formCmdParam.h \
    Forms/formCmd.h \
    Forms/formTypes.h \
    Tools/toolReturnCode.h \
    Forms/DialogStream.h \
    Stream/qEldViewer.h \
    Stream/CLineItem.h \
    Stream/FrameBuffer.h

#conoscopedll
#{
#    HEADERS += \
#        Conoscope/conoscopeLib.h
#}

#!conoscopedll
#{
#    HEADERS += \
#        Camera/camera.h \
#        Camera/cameraCmvCxp.h \
#        Camera/cameraCmvCxpHw.h \
#        Camera/HwTool.h \
#        CoaXPress/CoaXpressConfiguration.h \
#        CoaXPress/CoaXpressController.h \
#        CoaXPress/CoaXpressFrame.h \
#        CoaXPress/CoaXpressGrabber.h \
#        CoaXPress/CoaXpressTypes.h \
#        Conoscope/conoscope.h
#}

CONFIG(conoscopedll) {
    SOURCES += \
        Conoscope/conoscopeLib.cpp

    HEADERS += \
        Conoscope/conoscopeLib.h
} else {
#    message(Building without conoscopedll.)
    
#     SOURCES += \
#        Camera/camera.cpp \
#        Camera/cameraCmvCxp.cpp \
#        CoaXPress/CoaXpressController.cpp \
#        CoaXPress/CoaXpressFrame.cpp \
#        CoaXPress/CoaXpressGrabber.cpp \
#        CoaXPress/CoaXpressTypes.cpp \
#        Conoscope/conoscope.cpp
        
#    HEADERS += \
#        Camera/camera.h \
#        Camera/cameraCmvCxp.h \
#        Camera/cameraCmvCxpHw.h \
#        Camera/HwTool.h \
#        CoaXPress/CoaXpressConfiguration.h \
#        CoaXPress/CoaXpressController.h \
#        CoaXPress/CoaXpressFrame.h \
#        CoaXPress/CoaXpressGrabber.h \
#        CoaXPress/CoaXpressTypes.h \
#        Conoscope/conoscope.h
}

FORMS += \
    Forms/mainwindow.ui \
    Forms/formCmdParam.ui \
    Forms/formCmd.ui \
    Forms/DialogStream.ui

#!conoscopedll
#{
#    INCLUDEPATH += './Camera'
#    INCLUDEPATH += './CoaXPress'
#}

INCLUDEPATH += './Tools'
INCLUDEPATH += './Forms'
INCLUDEPATH += './Shared'
INCLUDEPATH += './AppController'
INCLUDEPATH += './Conoscope'
INCLUDEPATH += './Stream'

# INCLUDEPATH += 'C:/Program Files (x86)/Euresys/Coaxlink/include'
# DEPENDPATH += 'C:/Program Files (x86)/Euresys/Coaxlink/include'

#!conoscopedll
#{
#    INCLUDEPATH += 'C:/Program Files/Euresys/Coaxlink/include'
#    DEPENDPATH += 'C:/Program Files/Euresys/Coaxlink/include'
#}

conoscopedll
{
    win32:CONFIG(release, debug|release): LIBS += -L$$PWD\..\build-ConoscopeLib-Desktop_Qt_5_14_2_MSVC2015_64bit-Release\release -lConoscopeLib
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD\..\build-ConoscopeLib-Desktop_Qt_5_14_2_MSVC2015_64bit-Debug\debug -lConoscopeLib

    INCLUDEPATH += $$PWD\..\ConoscopeLib
    DEPENDPATH += $$PWD\..\ConoscopeLib
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD\..\build-PipelineLib-Desktop_Qt_5_14_2_MSVC2015_64bit-Release\release -lPipelineLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD\..\build-PipelineLib-Desktop_Qt_5_14_2_MSVC2015_64bit-Debug\debug -lPipelineLib

INCLUDEPATH += $$PWD\ConoscopePipeline
DEPENDPATH += $$PWD\ConoscopePipeline

