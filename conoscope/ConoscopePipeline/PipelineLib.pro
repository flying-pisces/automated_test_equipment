#-------------------------------------------------
#
# Project created by QtCreator 2019-11-19T15:17:51
#
#-------------------------------------------------

QT       -= gui
QT       += core xml

TARGET = PipelineLib
TEMPLATE = lib

DEFINES += PIPELINELIB_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += AE_MEAS_AREA

SOURCES += \
        PipelineLib.cpp \
    Compute.cpp \
    Pipeline/PipelineCompute.cpp \
    Pipeline/PipelineDefectCorrector.cpp \
    Tools/toolErrorCode.cpp \
    Tools/classcommon.cpp \
    Tools/toolString.cpp

HEADERS += \
        PipelineLib.h \
        PipelineLib_global.h \ 
        configuration.h \
    Compute.h \
    Pipeline/logger.h \
    Pipeline/Types.h \
    Pipeline/imageConfiguration.h \
    Pipeline/defines.h \
    Pipeline/PipelineCompute.h \
    Pipeline/PipelineDefectCorrector.h \
    Pipeline/PipelineComputeTypes.h \
    Pipeline/PipelineDefines.h \
    Tools/toolErrorCode.h \
    Tools/classcommon.h \
    Tools/toolString.h \
    PipelineTypes.h \
    Pipeline/PipelineHistogram.h

INCLUDEPATH += './Pipeline'
INCLUDEPATH += './Tools'

unix {
    target.path = /usr/lib
    INSTALLS += target
}

